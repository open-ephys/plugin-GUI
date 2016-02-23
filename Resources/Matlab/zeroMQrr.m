% zeroMQrr provides a simple wrapper around the zeromq library with a
% request->response scenario in mind
%
% Usage:
% 
%      zeroMQrr('Command',...);
%
%      zeroMQrr('Send',url, request, [blocking=0]);
%           When blocking is true, zeroMQrr waits for and returns the response
%             [response, dialogue]=zeroMQrr('Send',url, request, 1);
%             When blocking is 0 (default) zeroMQrr adds the request to a queue and returns the time the request was added to that queue. See 'GetResponses'
%             [timeRequestAdded]=zeroMQrr('Send',url, request, 0);
%             
%      dialogue = zeroMQrr('GetResponses' [, url] [,wairForEmptyQueue=1])
%             Retirieves responses collected for requests send in non blocking mode. If an url is provided
%             only responses for that url will be returend. If wairForEmptyQueue>0, the function waits for
%             the last queued request to get a response.
%             Dialogue is a struct with the fields: request, response, timeRequestAdded, timeRequestSent, timeResponeRecieved
%             At the moment this cannot get filtered by the url of the request yet.
%             
%      zeroMQrr('CloseAll')   
%             closes all open sockets and the queue thread if open.
%        
% 
%      zeroMQrr('CloseThread')
%             closes the queue thread if open
%             
%      url = zeroMQrr('StartConnectThread', url)
%             This function is not neccessary and for backward compatability only. It simply returns the url of the input as previous versions required a handle instead of the url if the 'Send' command
%            
%   currently only tested on OSX. To compile you need the zeromq library
%   installed (e.g. using brew on OSX) and the libraries set correctly in
%   compile_matlab_wrapper
% 
% Copyright (c) 2008 Shay Ohayon, California Institute of Technology.
% This file is a part of a free software. you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation (see GPL.txt)
%
% Changes to allow receiving responses by Jonas Knoell, 2016