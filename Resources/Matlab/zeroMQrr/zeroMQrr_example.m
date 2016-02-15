function zeroMQrr_example(url)

if nargin<1
    url = 'tcp://localhost:5556'; % or, e.g., //'tcp://10.71.212.19:5556 if GUI runs on another machine...
end

%blocking request to get the current recording status
[isacquiring, details]=zeroMQrr('Send',url, 'IsAcquiring', 1); 
%also get addiional info. this has a struct as returned by getResponses for
%that one response
isrecording=zeroMQrr('Send',url, 'IsRecording', 1); 

createNewDir=true;
RecDir=[];
PrependText=[];
AppendText=[];
if (strcmp(isacquiring, '1'))
    message='StartAcquisition';
%     zeroMQrr('Send',url, 'StartAcquisition', 1); 
    %options:
    if createNewDir
        message=[message ' CreateNewDir=1'];
    end
    if ~ismepty(RecDir)
        message=[message ' RecDir=' RecDir];
    end
    if ~ismepty(PrependText)
        message=[message ' PrependText=' PrependText];
    end
    if ~ismepty(AppendText)
        message=[message ' AppendText=' AppendText];
    end

%     message=sprintf('StartAcquisition CreateNewDir=%i RecDir=%s PrependText=%s AppendText=%s');
    zeroMQrr('Send',url, message); 
end
if (strcmp(isrecording,'1'))
    zeroMQrr('Send',url, 'StartRecord'); 
end

%wait for responsess
responses=zeroMQrr('GetResponses', url); 

recordingNumber=zeroMQrr('Send',url, 'getRecordingNumber',1); 
experimentNumber=zeroMQrr('Send',url, 'getExperimentNumber',1); 

zeroMQrr('Send',url ,'ClearDesign');
zeroMQrr('Send',url ,'NewDesign nGo_Left_Right');
zeroMQrr('Send',url ,'AddCondition Name GoRight TrialTypes 1 2 3');
zeroMQrr('Send',url ,'AddCondition Name GoLeft TrialTypes 4 5 6');

%wait for responsess
responses=zeroMQrr('GetResponses', url); 

for k = 1:10
    % indicate trial type number #2 has started
    zeroMQrr('Send',url ,'TrialStart 2');  
    tic; while toc < 0.2; end;
    disp('Trial start')
    % indicate that trial has ended with outcome "1"
    zeroMQrr('Send',url ,'TrialEnd 1');
    tic; while toc < 1; end;
end

zeroMQrr('Send',url, 'StopRecord', 0); 

zeroMQrr('Send',url, 'StopAcquisition', 0); 

%get all responses to the 'Send' requests since the last call to 'GetResponses'
%default: block untill all responses are in
blockTillQueueEmpty=1;%default
responses=zeroMQrr('GetResponses'); 
responses=zeroMQrr('GetResponses',blockTillQueueEmpty); 
%caviat: will also block if only responses in queue are for another url
responses=zeroMQrr('GetResponses',url); 
responses=zeroMQrr('GetResponses',url,blockTillQueueEmpty); 

%%or choose only to get responses already obtained
blockTillQueueEmpty=0;
responses=zeroMQrr('GetResponses',blockTillQueueEmpty); 
%caviat: will also block if only responses in queue are for another url
responses=zeroMQrr('GetResponses',url,blockTillQueueEmpty); 

zeroMQrr('CloseThread', url);

zeroMQrr('CloseAll');


end 

