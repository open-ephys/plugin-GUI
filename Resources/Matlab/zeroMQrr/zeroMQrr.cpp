/*
% Copyright (c) 2008 Shay Ohayon, California Institute of Technology.
% This file is a part of a free software. you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation (see GPL.txt)
*/
#include <stdio.h>
#include "mex.h"
#include "zmq.h"
#include "zmq_utils.h"
#include <string>
#include <cstring>
#include <queue>
#include <list>
#include <map>

#include <mutex> 
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h>
#else
    #include <unistd.h>
    #include <pthread.h>
#endif



typedef struct {
    bool thread_running;
    int threadIdleTime;
#ifdef _WIN32
    DWORD dwThread;
#endif
} ThreadData;

typedef struct {
    const char *url;
    void * zmqSocket;
    double timeOut;
} socketData;

typedef struct{
    std::string url;
    std::string request;
    std::string response;
    double timeRequestAdded; //e.g. from PsychGetAdjustedPrecisionTimerSeconds(returnValue);
    double timeRequestSent;
    double timeResponseReceived;
} dialogue;
const char* dialogueFieldnames[5] = { "request", "response","timeRequestAdded", "timeRequestSent", "timeResponseReceived"};

#ifdef _WIN32
    #define ms1 1
    #define ms10 10
    #define ms100 100
    #define ms1000 1000
    DWORD WINAPI MyThreadFunction( LPVOID lpParam );
   
    void do_sleep(DWORD s){
        Sleep(s);
    }
#else
    #define ms1 1000
    #define ms10 10000
    #define ms100 100000
    #define ms1000 1000000
    void* MyThreadFunction( void* lpParam );
    
    void do_sleep(useconds_t s){
        usleep(s);
    }
#endif

dialogue SendAndReceive( std::string url, std::string request );

#define MAX(x,y)(x>y)?(x):(y)
#define MIN(x,y)(x<y)?(x):(y)


typedef std::map<std::string,socketData> mp;
mp socketMap;
std::mutex socketMap_mutex;

std::queue<dialogue> InDialogues;
std::mutex InDialogues_mutex;

typedef std::list<dialogue> dlist;
dlist OutDialogues;
std::mutex OutDialogues_mutex;

typedef std::list<ThreadData*> lst;
lst threadList;
std::mutex threadList_mutex;

double lastDialogueAdded;
double lastDialogueFetched;

std::mutex sendReceiveBusy;

bool initialized = false;
void* context;

std::chrono::high_resolution_clock timer;

void CloseContext(void)
{
	if (initialized) {
        threadList_mutex.lock();
        for (lst::iterator i= threadList.begin(); i != threadList.end(); i++) {
            (*i)->thread_running=false;
        }
        threadList_mutex.unlock();

        bool threadListEmpty=false;
        while(!threadListEmpty){
            threadList_mutex.lock();
            if(threadList.empty()){
                threadListEmpty=true;
            }
            threadList_mutex.unlock();
            
            if(!threadListEmpty){
                do_sleep(ms10);
            }
        }

		for (mp::iterator i= socketMap.begin(); i != socketMap.end(); i++) {
            int toms=0;
            zmq_setsockopt(i->second.zmqSocket,ZMQ_LINGER,&toms, sizeof(toms));
			zmq_close (i->second.zmqSocket);
		}
		zmq_ctx_destroy (context);
		initialized = false;
        socketMap.clear();
	}
}

bool close_socket(std::string url){

    bool success=false;
    socketMap_mutex.lock();
    mp::iterator it=socketMap.find(url);
    socketData sock;
	int toms=0;
    if (it != socketMap.end()){
        sock = it->second;
        zmq_setsockopt(sock.zmqSocket,ZMQ_LINGER,&toms, sizeof(toms));
		zmq_close (sock.zmqSocket);
        socketMap.erase(it);
        success = true;
    }
    socketMap_mutex.unlock();

    return success;
}

void initialize_zmq()
{
	mexAtExit(CloseContext);
	context = zmq_ctx_new();
    lastDialogueAdded=0;
    lastDialogueFetched=0;
	initialized = true;
}

void SendAndReceive( dialogue* d )
{
    //check if we have an open socket for that url:
    std::string url = d->url;
    std::string request = d->request;
    
    socketMap_mutex.lock();
    mp::iterator it=socketMap.find(url);
    socketData sock;
    if (it == socketMap.end()){
        sock.url = url.c_str();
        sock.zmqSocket = zmq_socket (context, ZMQ_REQ);
        sock.timeOut=0.5;
        int toms=sock.timeOut*1000;
        zmq_setsockopt(sock.zmqSocket,ZMQ_LINGER,&toms, sizeof(toms));
        zmq_connect (sock.zmqSocket, sock.url); 
        socketMap.insert(std::pair<std::string,socketData>(url, sock));
    }else{
        sock = it->second;
    }
    socketMap_mutex.unlock();
			
    sendReceiveBusy.lock();
    
	zmq_msg_t request_z;
	zmq_msg_init_size (&request_z, d->request.length());
	memcpy (zmq_msg_data (&request_z), d->request.c_str() , d->request.length());

    zmq_msg_send (&request_z, sock.zmqSocket, 0);

    auto time = std::chrono::high_resolution_clock::now().time_since_epoch();
    d->timeRequestSent = (std::chrono::duration_cast<std::chrono::microseconds>(time)).count();
	zmq_msg_close (&request_z);

    zmq_msg_t reply;
	zmq_msg_init (&reply);
    bool failed=false;
    auto timeR=std::chrono::high_resolution_clock::now().time_since_epoch();
    while (zmq_msg_recv(&reply, sock.zmqSocket, ZMQ_DONTWAIT)==-1)
    {
        timeR = std::chrono::high_resolution_clock::now().time_since_epoch();
        bool isTimeout=(std::chrono::duration_cast<std::chrono::microseconds>(timeR-time)).count() > sock.timeOut*1000000;

        if(isTimeout || errno!=EAGAIN){

            failed=true;
            break;
        }
    }
    
    sendReceiveBusy.unlock();
    
    if(failed){
        d->timeResponseReceived=-1;
        d->response="failed waiting for a reply";
    }else{
        timeR = std::chrono::high_resolution_clock::now().time_since_epoch();
        d->timeResponseReceived = (std::chrono::duration_cast<std::chrono::microseconds>(timeR)).count();

        size_t size = zmq_msg_size (&reply);
        char *s = (char *) malloc (size + 1);
        memcpy (s, zmq_msg_data (&reply), size);

        d->response=std::string(s,size);
    }

    zmq_msg_close (&reply);
	return;
}

#ifdef _WIN32
    DWORD WINAPI MyThreadFunction( LPVOID lpParam )
#else
    void* MyThreadFunction( void* lpParam )
#endif    
{
    //work on all queued dialogues
    ThreadData *pData = (ThreadData*) lpParam ;
    pData->thread_running = true;
    
    while(pData->thread_running){
        InDialogues_mutex.lock();
        if(!InDialogues.empty()){
            dialogue d = InDialogues.front();
            InDialogues.pop();
            InDialogues_mutex.unlock(); 

            SendAndReceive( &d );

            OutDialogues_mutex.lock(); 
            OutDialogues.push_back(d);
            OutDialogues_mutex.unlock(); 
        }else{
            InDialogues_mutex.unlock(); 
            do_sleep(pData->threadIdleTime);
        } 
    }
    
    pData->thread_running = false;
    threadList_mutex.lock();
    threadList.remove(pData);
    threadList_mutex.unlock();
    
    return 0;
}

ThreadData* create_client_thread()
{
	ThreadData* pData = new ThreadData;
	pData->threadIdleTime=ms10;
    pData->thread_running = false;

    #ifdef _WIN32
    HANDLE retValue = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            MyThreadFunction,       // thread function name
            pData,          // argument to thread function 
            0,                      // use default creation flags 
            &pData->dwThread);   // returns the thread identifier 
    #else
	pthread_t thread;
	int retValue = pthread_create(&thread,
							      NULL,
							      MyThreadFunction,
							      (void*) pData);
    #endif
	return pData;
}

void mexFunction( int nlhs, mxArray* plhs[], 
				 int nrhs, const mxArray* prhs[] ) 
{

    if (!initialized) initialize_zmq();
    if (nrhs < 1) {
        return;
    }

	char* Command = mxArrayToString(prhs[0]);

	if   (strcmp(Command, "StartConnectThread") == 0)  {
        //return the url for backward compatability
        if(nlhs>0 && nrhs>1){
            int url_length = int(mxGetNumberOfElements(prhs[1])) + 1;
			char* url = mxArrayToString(prhs[1]);
            plhs[0] = mxCreateString(url);
        }
    }

    if (strcmp(Command, "Send") == 0) {
        if (nrhs < 3) {
            return;
        }
        bool blocking=false;
        if (nrhs >3) {
            double Tmp = mxGetScalar(prhs[3]);
            blocking = Tmp>0;
        }
        
        dialogue d;
        d.url = std::string(mxArrayToString(prhs[1]));
        d.request = std::string(mxArrayToString(prhs[2]));
        auto time = std::chrono::high_resolution_clock::now().time_since_epoch();
        d.timeRequestAdded = (std::chrono::duration_cast<std::chrono::microseconds>(time)).count();

        if(blocking){
            SendAndReceive(&d);
            if (d.timeResponseReceived==-1) {
                // Thread was killed and now we try to send things again?
                mexPrintf("ZMQ: Failed to get a response.\n");
            }

            // in blocking mode first argument is the response, second is the full dialogue
            if (nlhs>0){//just return the response
                mxArray * mxOutResponse;
                mxOutResponse = mxCreateString( d.response.c_str() );
                plhs[0]=mxOutResponse;
            }
            if (nlhs>1){//also return the dialogue struct
                mxArray * mxOutStruct = mxCreateStructMatrix(1,1,5,dialogueFieldnames);
                mxSetField(mxOutStruct, 0, "request",  mxCreateString( d.request.c_str() ) );
                mxSetField(mxOutStruct, 0, "response",  mxCreateString( d.response.c_str() ) );

                mxSetField(mxOutStruct,0, "timeRequestAdded", mxCreateDoubleMatrix(1,1, mxREAL));
                memcpy(mxGetData(mxGetField(mxOutStruct,0, "timeRequestAdded")) ,&(d.timeRequestSent),sizeof(double));

                mxSetField(mxOutStruct,0, "timeRequestSent", mxCreateDoubleMatrix(1,1, mxREAL));
                memcpy(mxGetData(mxGetField(mxOutStruct,0, "timeRequestSent")) ,&(d.timeRequestSent),sizeof(double));

                mxSetField(mxOutStruct,0, "timeResponseReceived", mxCreateDoubleMatrix(1,1, mxREAL));
                memcpy(mxGetData(mxGetField(mxOutStruct,0, "timeResponseReceived")) ,&(d.timeResponseReceived),sizeof(double));
                plhs[1]=mxOutStruct;
            }
        }else{ //not blocking
            if( lastDialogueAdded >= d.timeRequestAdded){ //make sure we get a new timestamp
                d.timeRequestAdded = lastDialogueAdded+1;
            }
            lastDialogueAdded = d.timeRequestAdded;
            InDialogues_mutex.lock(); 
            InDialogues.push(d);
            InDialogues_mutex.unlock(); 

            //start a thread if non is open
            threadList_mutex.lock();
            if(threadList.empty()){
                ThreadData* sd = create_client_thread();
                threadList.push_back(sd);
            }
            threadList_mutex.unlock();
            
            //if a n output is requested return the time the request was added to the queue, for later matching in the 'getResponses' output
            if (nlhs>0)
            {
                mxArray * mxOutStruct;
                mxOutStruct = mxCreateDoubleMatrix(1,1, mxREAL);
                memcpy(mxGetData(mxOutStruct) ,&(d.timeRequestAdded),sizeof(double));
                plhs[0]=mxOutStruct;
            }
        
        }//if blocking
    }
			
    if (strcmp(Command, "CloseAll") == 0)
    {
        CloseContext();
    }
        
	if (strcmp(Command, "CloseThread") == 0){

        if(nrhs<2){
            return;
        }
        close_socket(std::string(mxArrayToString(prhs[1])));

        //close thread only if socketMap empty
        socketMap_mutex.lock();
        if(!socketMap.empty()){
            socketMap_mutex.unlock();
            threadList_mutex.lock();
            for (lst::iterator i= threadList.begin(); i != threadList.end(); i++) {
                (*i)->thread_running=false;
            }
            threadList_mutex.unlock();
        }else{
            socketMap_mutex.unlock();
        }
	}
    
   if (strcmp(Command, "GetResponses") == 0){

       bool wairForEmptyQueue;
       std::string url;
       //if no second input or second input is no string
       if ( nrhs<2 || !mxIsChar(prhs[1]) ){
           url = "*";
       }else{
            url = (mxArrayToString(prhs[1]));
       }

       //do we want to waint untill the current Sending Queue is emptied and all replies are in?
       if(nrhs<2 || (nrhs<3 && mxIsChar(prhs[1])) ){//if only one input or 2 and that is an url
           wairForEmptyQueue = true;
       }else{
           double Tmp;
           if (mxIsChar(prhs[1])){ //if second is the url, take the third
            Tmp = mxGetScalar(prhs[2]);
           }else{
            Tmp = mxGetScalar(prhs[1]);
           }
           wairForEmptyQueue = Tmp>0;
       }

       double lastAddedTime = lastDialogueAdded;
       double lastFetchedTime = lastDialogueFetched;
       if(lastFetchedTime ==lastAddedTime){ //nothing to fetch
          wairForEmptyQueue = false;
       }
       //if so, do it now
       while(wairForEmptyQueue)
       {
           OutDialogues_mutex.lock();
         if(!OutDialogues.empty() && OutDialogues.back().timeRequestAdded>=lastAddedTime)
          {
             wairForEmptyQueue=false;
          }
          OutDialogues_mutex.unlock();     
          do_sleep(ms10);
       }

       //find the ones we want to return
       std::queue<dialogue> returnDialogues;
       OutDialogues_mutex.lock();
       if(!OutDialogues.empty()){
        lastDialogueFetched=OutDialogues.back().timeRequestAdded;

        dlist::iterator it=OutDialogues.begin();
        while(it!=OutDialogues.end()){
         if(!url.compare("*") || !it->url.compare(url)){
          dialogue d= (*it);
          returnDialogues.push(d);
          it=OutDialogues.erase(it);
         }else{
             it++;
         }
        }
       }
       OutDialogues_mutex.unlock();

       //now we build a struct with all replies
       mxArray * mxOutStruct = mxCreateStructMatrix(1,returnDialogues.size(),5,dialogueFieldnames);

       int j=0;
       while(!returnDialogues.empty())
       {
           dialogue d = returnDialogues.front();
           returnDialogues.pop();

           mxSetField(mxOutStruct, j, "request",  mxCreateString( d.request.c_str() ) );
           mxSetField(mxOutStruct, j, "response",  mxCreateString( d.response.c_str() ) );

           mxSetField(mxOutStruct,j, "timeRequestAdded", mxCreateDoubleMatrix(1,1, mxREAL));
       	memcpy(mxGetData(mxGetField(mxOutStruct,j, "timeRequestAdded")) ,&(d.timeRequestAdded),sizeof(double));

           mxSetField(mxOutStruct,j, "timeRequestSent", mxCreateDoubleMatrix(1,1, mxREAL));
       	memcpy(mxGetData(mxGetField(mxOutStruct,j, "timeRequestSent")) ,&(d.timeRequestSent),sizeof(double));

           mxSetField(mxOutStruct,j, "timeResponseReceived", mxCreateDoubleMatrix(1,1, mxREAL));
       	memcpy(mxGetData(mxGetField(mxOutStruct,j, "timeResponseReceived")) ,&(d.timeResponseReceived),sizeof(double));
           j++;
       }

       plhs[0] = mxOutStruct;
   }
}