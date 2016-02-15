url = 'localhost:5556'; % or, e.g., //'tcp://10.71.212.19:5556 if GUI runs on another machine...

handle = zeroMQwrapper('StartConnectThread',url);

zeroMQwrapper('Send',handle ,'ClearDesign');
zeroMQwrapper('Send',handle ,'NewDesign nGo_Left_Right');
zeroMQwrapper('Send',handle ,'AddCondition Name GoRight TrialTypes 1 2 3');
zeroMQwrapper('Send',handle ,'AddCondition Name GoLeft TrialTypes 4 5 6');

tic; while toc < 2; end;

for k = 1:10
    % indicate trial type number #2 has started
    zeroMQwrapper('Send',handle ,'TrialStart 2');  
    tic; while toc < 0.2; end;
    disp('Trial start')
    % indicate that trial has ended with outcome "1"
    zeroMQwrapper('Send',handle ,'TrialEnd 1');
    tic; while toc < 1; end;
end

zeroMQwrapper('CloseThread',handle);
