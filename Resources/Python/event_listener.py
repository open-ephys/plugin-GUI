from __future__ import print_function, unicode_literals
from collections import OrderedDict
from itertools import chain, repeat
try:
    from itertools import izip
except ImportError:
    izip = zip  # Python 3
import struct

import zmq


# Event types
TTL = 3
SPIKE = 4
MESSAGE = 5
BINARY_MSG = 6


def unpacker(format, *fields):
    s = struct.Struct(format)

    def unpack(data):
        values = s.unpack(data[:s.size])
        if (len(values) == 1) and (not fields):
            assert len(data) == s.size
            return values[0], ''
        assert len(values) <= len(fields)
        return (OrderedDict(izip(fields, chain(values, repeat(None)))),
                data[s.size:])

    return unpack


unpack_standard = unpacker('3BxB',
                           'node_id',
                           'event_id',
                           'event_channel',
                           'source_node_id',
                           )


unpack_ttl = unpacker('<Q')


unpack_spike = unpacker('<2q2x5H3B2fH',
                        'timestamp',
                        'timestamp_software',
                        'n_channels',
                        'n_samples',
                        'sorted_id',
                        'electrode_id',
                        'channel',
                        'color_r', 'color_g', 'color_b',
                        'pc_proj_x', 'pc_proj_y',
                        'sampling_frequency_hz',
                        'data',
                        'gain',
                        'threshold',
                        )


def run(hostname='localhost', port=5557):
    with zmq.Context() as ctx:
        with ctx.socket(zmq.SUB) as sock:
            sock.connect('tcp://%s:%d' % (hostname, port))

            for etype in (TTL, SPIKE, MESSAGE):
                sock.setsockopt(zmq.SUBSCRIBE, chr(etype).encode('utf-8'))

            while True:
                try:
                    parts = sock.recv_multipart()
                    assert len(parts) == 3

                    etype = ord(parts[0])
                    timestamp_seconds = struct.unpack('d', parts[1])[0]
                    body = parts[2]

                    if etype == SPIKE:
                        spike, body = unpack_spike(body)
                        print('%g: Spike: %s' % (timestamp_seconds, spike))
                        body = ''  # TODO: unpack other data

                    else:
                        header, body = unpack_standard(body)
    
                        if etype == TTL:
                            word, body = unpack_ttl(body)
                            print('%g: TTL: Channel %d: %s' %
                                  (timestamp_seconds,
                                   header['event_channel'] + 1,
                                   'ON' if header['event_id'] else 'OFF'))
    
                        elif etype == MESSAGE:
                            msg, body = body.decode('utf-8'), ''
                            print('%g: Message: %s' % (timestamp_seconds, msg))


                    # Check that all data was consumed
                    assert len(body) == 0

                except KeyboardInterrupt:
                    print()  # Add final newline
                    break


if __name__ == '__main__':
    run()
