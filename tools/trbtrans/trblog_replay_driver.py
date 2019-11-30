# ***********************************************************************
# * Copyright 2016 Qualcomm Technologies International, Ltd.
# ***********************************************************************

from trblog_replay import TrbLogReplayer, TrbLogReplay
import trbtrans
import sys


def replay(driver, log_path):
    print "Parsing..."
    recording = TrbLogReplay.parse(log_path, verbose=False)
    print "Replaying..."
    trb = trbtrans.Trb()
    trb.open(driver)
    replayer = TrbLogReplayer(trb)
    replayer.replay(recording, verify_payloads=False)


if __name__ == '__main__':
    try:
        driver = sys.argv[1]
        source_log = sys.argv[2]
    except IndexError:
        print "Usage: python {0} <driver> <path_to_log>\ne.g. python {0} usb2trb mylog.log".format(sys.argv[0])
        sys.exit(1)

    for i in range(0, 10):
        replay(driver, source_log)