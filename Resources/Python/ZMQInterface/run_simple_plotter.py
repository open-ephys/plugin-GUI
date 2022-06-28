import sys
import simple_plotter_zmq

if __name__ == '__main__':
    pl = simple_plotter_zmq.SimplePlotter(40000.)
    pl.startup()
