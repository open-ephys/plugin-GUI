url = 'localhost:5556'; % or, e.g., //'tcp://10.71.212.19:5556 if GUI runs on another machine...

% handle = zeroMQrr('StartConnectThread',url); not necessary anymore

zeroMQrr('Send',url ,'ClearDesign');
zeroMQrr('Send',url ,'NewDesign nGo_Left_Right');
zeroMQrr('Send',url ,'AddCondition Name GoRight TrialTypes 1 2 3');
zeroMQrr('Send',url ,'AddCondition Name GoLeft TrialTypes 4 5 6');

%blocking request to get the current recording status
isrecording=zeroMQrr('Send',url, 'IsRecording', 1); 

tic; while toc < 2; end;

for k = 1:10
    % indicate trial type number #2 has started
    zeroMQrr('Send',url ,'TrialStart 2');  
    tic; while toc < 0.2; end;
    disp('Trial start')
    % indicate that trial has ended with outcome "1"
    zeroMQrr('Send',url ,'TrialEnd 1');
    tic; while toc < 1; end;
end

%get all responses to the 'Send' requests since the last call to 'GetResponses'
blockTillQueueEmpty=1;%default
responses=zeroMQrr('GetResponses',blockTillQueueEmpty); 

zeroMQrr('CloseThread');
