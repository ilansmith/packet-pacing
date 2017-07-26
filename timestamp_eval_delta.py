#!/usr/bin/python3
"""
Use to verify timestamping's output.
Result is a csv file with the hardware and software RX timestamps, and the
differences between a HW timestamp with its previous, the same difference
diregarding the interval between message, and the difference between the HW
and SW timestamp.

Inputs:
    --messages_per_second - interval between messages
    --in - file name of timestamping test output
    --out - file name to output csv to
"""

import csv
import argparse
import sys

HW_TS_EXP = "HW raw "

def ts_from_line(line, exp):
    """
    convert from line printed by timestamping to numpy.longdouble, for specific value
    """
    start = line.find(exp)+len(exp)
    end = line.find(' ', start)
    try:
        if (end < 0):
            sec, nanosec = line[start:].replace('\n','').split('.')#.replace('.','')
        else:
            sec, nanosec = line[start:end].replace('\n','').split('.')#.replace('.','')
    except:
        return 0, 0
    
    return sec, nanosec

def main(in_name, out_name):
    """
    args:
        messages_per_second - interval between messages
        in_name - file name of timestamping test output
        out_name - file name to output csv to
    """
    packet_num = 0

    with open(in_name, 'rb') as fin, open(out_name, 'wb') as fout:
        csv_writer = csv.writer(fout, delimiter=',')
        csv_writer.writerow(['Packet Num', 'HW Timestamp Seconds', 'HW Timestamp Nanoseconds'])
        for i, line in enumerate(fin):
            if 'SOL_SOCKET' not in line:
                continue

            packet_num += 1
            if packet_num < 4000:
                continue

            hw_sec, hw_nsec = ts_from_line(line, HW_TS_EXP)

            csv_writer.writerow([packet_num, hw_sec, hw_nsec])

            


if __name__ == "__main__":
    EPILOG = """
Use to verify timestamping's output.

Inputs:
    --in - file name of timestamping test output
    --out - file name to output csv to
"""

    parser = argparse.ArgumentParser(epilog=EPILOG)
    parser.add_argument('--in', dest="in_name", type=str,
                        default="timestamping.out",
                        help="name of input file")
    parser.add_argument('--out', dest="out_name", type=str,
                        default="timestamp_eval_delta.csv",
                        help="name of output file")

    parsed_args = parser.parse_args()

    sys.exit(main(parsed_args.in_name, parsed_args.out_name))
