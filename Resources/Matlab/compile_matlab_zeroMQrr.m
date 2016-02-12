% compile 0MQ matlab request->response (zeroMQrr)

% 1. find the correct locations of zermoMQ librearies and heaers
if strfind(computer,'PCWIN')
    Codefolder=mfilename('fullpath');
    Codefolder=Codefolder(1:(end-length(mfilename()-1)-length('Matlab')-1-1));
    libraryName = 'libzmq-v120-mt-4_0_4';
    if strcmp(computer,'PCWIN64')
        libFolder = [Codefolder, '\windows-libs\ZeroMQ\lib_x64'];
    else
        libFolder = [Codefolder, '\windows-libs\ZeroMQ\lib_x86'];
    end
     headerFolder = [Codefolder, '\windows-libs\ZeroMQ\include'];
elseif strcmp(computer,'GLNX86') || strcmp(computer,'GLNXA64')
    libraryName = 'zmq';
    if ~isempty(dir(['/usr/local/lib/lib' libraryName '*']))
        libFolder = '/usr/local/lib';
        headerFolder = '??';
    else
        error('compile:matlab_zeroMQrr','please add the paths to your zeroMQ libraries and headers');
    end
elseif strcmp(computer,'MACI64')
    libraryName = 'zmq';
    %iterate trough some options
    if ~isempty(dir(['/usr/local/Cellar/zeromq/4.1.4/lib/lib' libraryName '*'])) %should prob. iterate to find the newest version
        libFolder = '/usr/local/Cellar/zeromq/4.1.4/lib';
        headerFolder = '/usr/local/Cellar/zeromq/4.1.4/include';
%     elseif ~isempty(dir(['/usr/local/lib/lib' libraryName '*']))
%         libFolder = '/usr/local/lib';
%         headerFolder = '??';
    else
        error('compile:matlab_zeroMQrr','please add the paths to your zeroMQ libraries and headers');
    end

 end

cppFile = 'zeroMQrr/zeroMQrr.cpp';
% 2. compile
eval(['mex ' cppFile ' -I',headerFolder,' -L',libFolder,' -l',libraryName] )
