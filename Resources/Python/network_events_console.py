from __future__ import print_function, unicode_literals

import zmq

try:
    raw_input
except NameError:
    # Python 3
    raw_input = input


def run(hostname='localhost', port=5556):
    with zmq.Context() as ctx:
        with ctx.socket(zmq.REQ) as sock:
            sock.connect('tcp://%s:%d' % (hostname, port))
            while True:
                try:
                    req = raw_input('> ')
                    sock.send_string(req)
                    rep = sock.recv_string()
                    print(rep)
                except EOFError:
                    print()  # Add final newline
                    break


if __name__ == '__main__':
    run()
