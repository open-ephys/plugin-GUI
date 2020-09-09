"""
    A zmq client to test remote control of open-ephys GUI
"""

import zmq
import os
import time


def run_client():

    # Basic start/stop commands
    start_cmd = 'StartRecord'
    stop_cmd = 'StopRecord'

    # Example settings
    rec_dir = os.path.join(os.getcwd(), 'Output_RecordControl')
    print "Saving data to:", rec_dir

    # Some commands
    commands = [start_cmd + ' RecDir=%s' % rec_dir,
                start_cmd + ' PrependText=Session01 AppendText=Condition01',
                start_cmd + ' PrependText=Session01 AppendText=Condition02',
                start_cmd + ' PrependText=Session02 AppendText=Condition01',
                start_cmd,
                start_cmd + ' CreateNewDir=1']

    # Connect network handler
    ip = '127.0.0.1'
    port = 5556
    timeout = 1.

    url = "tcp://%s:%d" % (ip, port)

    with zmq.Context() as context:
        with context.socket(zmq.REQ) as socket:
            socket.RCVTIMEO = int(timeout * 1000)  # timeout in milliseconds
            socket.connect(url)

            # Start data acquisition
            socket.send('StartAcquisition')
            print socket.recv()
            time.sleep(5)

            socket.send('IsAcquiring')
            print "IsAcquiring:", socket.recv()
            print ""

            for start_cmd in commands:

                for cmd in [start_cmd, stop_cmd]:
                    socket.send(cmd)
                    print socket.recv()

                    if 'StartRecord' in cmd:
                        # Record data for 5 seconds
                        socket.send('IsRecording')
                        print "IsRecording:", socket.recv()

                        socket.send('GetRecordingPath')
                        print "Recording path:", socket.recv()

                        time.sleep(5)
                    else:
                        # Stop for 1 second
                        socket.send('IsRecording')
                        print "IsRecording:", socket.recv()
                        time.sleep(1)

                print ""

            # Finally, stop data acquisition; it might be a good idea to
            # wait a little bit until all data have been written to hard drive
            time.sleep(0.5)
            socket.send('StopAcquisition')
            print socket.recv()


if __name__ == '__main__':
    run_client()