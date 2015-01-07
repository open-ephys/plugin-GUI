% compile 0MQ matlab wrapper 
%
% NOTE: On OS X, you may need to implement this patch
%       for compilation to work:
%       http://www.mathworks.com/matlabcentral/answers/94092
%


% 1. make sure mex is setup properly and a compiler is available
mex -setup

%GUIfolder = 'C:\Users\Shay\Documents\GitHub\GUI';
%GUIfolder = '/home/jsiegle/Programming/GUI/';
GUIfolder = '/Users/Josh/Programming/open-ephys/GUI';

headerFolder = [GUIfolder, '/Resources/ZeroMQ/include'];

if strcmp(computer,'PCWIN')
    libFolder = [GUIfolder, '/Resources/ZeroMQ/lib_x86'];
    libraryName = 'libzmq-v110-mt-3_2_2';
    cppFile = 'windows/zeroMQwrapper.cpp';
elseif strcmp(computer,'PCWIN64')
    libFolder = [GUIfolder, '/Resources/ZeroMQ/lib_x64'];
    libraryName = 'libzmq-v110-mt-3_2_2';
    cppFile = 'windows/zeroMQwrapper.cpp';
elseif strcmp(computer,'GLNX86') || strcmp(computer,'GLNXA64')
    libFolder = '/usr/local/lib';
    libraryName = 'zmq';
    cppFile = 'unix/zeroMQwrapper.cpp';
elseif strcmp(computer,'MACI64')
    libFolder = '/opt/local/lib';
    libraryName = 'zmq';
    cppFile = 'unix/zeroMQwrapper.cpp';
end

% 2. compile
eval(['mex ' cppFile ' -I',headerFolder,' -L',libFolder,' -l',libraryName] )
