function zeroMQrr_tests(url)

if nargin<1
    url = 'tcp://localhost:5556'; % or, e.g., //'tcp://10.71.212.19:5556 if GUI runs on another machine...
end
url2 = 'tcp://localhost:5557'; 
% handle = zeroMQrr('StartConnectThread',url); not necessary anymore

%make sure output is cleared;
responses=zeroMQrr('GetResponses'); 
%send some blocking messages
response=zeroMQrr('Send',url ,'Message', 1);%response is string
[response, responses]=zeroMQrr('Send',url ,'Message', 1);%response is a struct

%% some tests that everything is working
times = sendSomeMessages(url,url2);

%get all responses to the 'Send' requests since the last call to 'GetResponses'
blockTillQueueEmpty=1;%default
responses=zeroMQrr('GetResponses') 
if length(responses)~=3
   warning('Missing return Messages') 
end
if any(times-[responses.timeRequestAdded])
    warning('Times dont match');
end

%alternatives:
responses=zeroMQrr('GetResponses',blockTillQueueEmpty); 

%caviat: will also block if only responses in queue are for another url
times = sendSomeMessages(url,url2);
responses=zeroMQrr('GetResponses',url,blockTillQueueEmpty); 
if length(responses)~=2
   warning('Missing return Messages') 
end
if any(times([1 3])-[responses.timeRequestAdded])
    warning('Times dont match');
end
%alternative:
responses=zeroMQrr('GetResponses',url); 

times = sendSomeMessages(url,url2);
blockTillQueueEmpty=0;%default
responses=zeroMQrr('GetResponses',blockTillQueueEmpty); 
%caviat: will also block if only responses in queue are for another url
responses=zeroMQrr('GetResponses',url,blockTillQueueEmpty); 

zeroMQrr('CloseThread', url);

zeroMQrr('CloseAll');

%make sure output is cleared;
responses=zeroMQrr('GetResponses'); 
end 

function timeRequestsAdded = sendSomeMessages(url, url2)
    timeRequestsAdded=zeros(1,3);
    timeRequestsAdded(1)=zeroMQrr('Send',url ,'1');
    timeRequestsAdded(2)=zeroMQrr('Send',url2 ,'2');
    timeRequestsAdded(3)=zeroMQrr('Send',url ,'3');
end