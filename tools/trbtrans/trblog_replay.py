# ***********************************************************************
# * Copyright 2016 Qualcomm Technologies International, Ltd.
# ***********************************************************************

import re
import trbtrans


# Set to True to use the pre-2.2 log format.
PRE_22_LOG_FORMAT = False


class InvalidTrbLogEntry(Exception):
    pass


class ReplayOp:
    def __init__(self, op_type, num_transactions, transactions):
        self.op_type = op_type
        self.num_transactions = num_transactions
        self.transactions = transactions


class TrbLogReplay(object):
    timestamp_matcher = re.compile('\d\d\d\d\.\d\d\d\d\d')
    read_pend_matcher = re.compile('Pending read (\d+)')
    write_matcher = re.compile('Pending write (\d+)\s+')
    write_done_matcher = re.compile('Done waiting for outstanding writes')
    read_done_matcher = re.compile('Done waiting for outstanding reads')

    def __init__(self, verbose=False):
        self.replay = []
        self.partial_read_op = None
        self.verbose = verbose

    @staticmethod
    def _print_write_address(transaction):
        address = 0
        pos = 24
        for b in range(1, 5):
            address |= (transaction.payload[b] << pos)
            pos -= 8
        pm = (address & 0x10000000 == 0x10000000)
        print "W: raw {0:#x} real {1:#x} {2}".format(address, address & 0x0fffffff, "PM" if pm else "DM")

    @staticmethod
    def _get_txn_elements(is_read_and_so_has_timestamp_to_skip, line):
        txn_elements = line.split()

        if is_read_and_so_has_timestamp_to_skip:
            if PRE_22_LOG_FORMAT:
                if len(txn_elements) % 16 != 0:
                    raise InvalidTrbLogEntry("Unexpected number of octets in read transaction sequence: {0}"
                                             .format(len(txn_elements)))
                entries_to_skip_per_txn = 4
            else:
                # Timestamp coming back is formatted as a 32-bit number, i.e. one element. So there's 12 + 1 elements.
                if len(txn_elements) % 13 != 0:
                    raise InvalidTrbLogEntry("Unexpected number of elements in read transaction sequence: {0}"
                                             .format(len(txn_elements)))
                entries_to_skip_per_txn = 1
        else:
            if len(txn_elements) % 12 != 0:
                raise InvalidTrbLogEntry("Unexpected number of octets in write transaction sequence: {0}"
                                         .format(len(txn_elements)))
            entries_to_skip_per_txn = 0

        return txn_elements, entries_to_skip_per_txn

    def _make_transactions(self, is_read_and_so_has_timestamp_to_skip, line, num_transactions):
        transactions = (trbtrans.Transaction * num_transactions)()

        txn_elements, entries_to_skip_per_txn = self._get_txn_elements(is_read_and_so_has_timestamp_to_skip, line)

        ele_index = 0
        octets_per_txn = 12

        for i in range(0, num_transactions):
            ele_index += entries_to_skip_per_txn

            raw_bytes = map(lambda hex_byte: int(hex_byte, base=16),
                            txn_elements[ele_index:ele_index + octets_per_txn])
            ele_index += octets_per_txn

            t = transactions[i]
            t.timestamp = 0
            t.opcode_and_src_subsys_id = raw_bytes[0]
            t.src_block_id_and_dest_subsys_id = raw_bytes[1]
            t.dest_block_id_and_tag = raw_bytes[2]
            for p in range(0, 9):
                t.payload[p] = raw_bytes[3 + p]

            if self.verbose and not is_read_and_so_has_timestamp_to_skip:
                TrbLogReplay._print_write_address(t)

        return transactions

    def _on_read_done_match(self, line):
        if not self.partial_read_op:
            raise Exception("No partial read op to complete!")
        self.partial_read_op.transactions = self._make_transactions(True, line, self.partial_read_op.num_transactions)
        self.replay.append(self.partial_read_op)
        self.partial_read_op = None

    def _on_write_match(self, line, num_transactions):
        transactions = self._make_transactions(False, line, num_transactions)
        self.replay.append(ReplayOp("w", num_transactions, transactions))

    def _feed_inner(self, line):
        timestamp_match = TrbLogReplay.timestamp_matcher.search(line)
        if timestamp_match:
            action_start = timestamp_match.end()
            line = line[action_start:]
        else:
            if line.find("ERROR:") > -1:
                self.replay.append(ReplayOp("e", 0, None))

        read_pend_match = TrbLogReplay.read_pend_matcher.search(line)
        if read_pend_match:
            num_transactions = int(read_pend_match.group(1))
            self.partial_read_op = ReplayOp("r", num_transactions, None)
            return

        write_match = TrbLogReplay.write_matcher.search(line)
        if write_match:
            num_transactions = int(write_match.group(1))
            transaction_bytes_start = write_match.end()
            line = line[transaction_bytes_start:].lstrip()
            self._on_write_match(line, num_transactions)
            return

        write_done_match = TrbLogReplay.write_done_matcher.search(line)
        if write_done_match:
            return

        read_done_match = TrbLogReplay.read_done_matcher.search(line)
        if read_done_match:
            transaction_bytes_start = read_done_match.end()
            line = line[transaction_bytes_start:].lstrip()
            self._on_read_done_match(line)
            return

    def feed_line(self, line):
        """
        Feed the replay a log line. The line will be processed and added to the replay record.
        :param line: a log line to process
        """
        try:
            self._feed_inner(line)
        except Exception as e:
            raise InvalidTrbLogEntry(e)

    def __iter__(self):
        for r in self.replay:
            yield r

    @staticmethod
    def parse(filename, verbose=False):
        """
        Parse a log file and construct a replayable record of it.
        :param filename: a log file to process
        :return: a TrbLogReplay instance, containing the replayable log record
        """
        replayer = TrbLogReplay(verbose)
        with open(filename) as f:
            for line in f.readlines():
                replayer.feed_line(line)
        return replayer


class TrbLogReplayer(object):

    def __init__(self, trb):
        self.trb = trb
        self.had_error = False

    def _do_read(self, op, op_idx, verify_payloads):
        read_txns, num_read_txns, wrapped = self.trb.read_raw_transactions(op.num_transactions, timeout_millis=1000)
        # When we know there was an error in the original log, do the I/O, but don't validate the output.
        # We then bail out, as the results aren't meaningful.
        if self.had_error:
            return False

        for tidx in range(num_read_txns):
            trans      = read_txns[tidx]
            orig_trans = op.transactions[tidx]

            if trans.opcode_and_src_subsys_id != orig_trans.opcode_and_src_subsys_id:
                raise Exception("FAIL: opcode_and_src_subsys_id mismatch at index {0}: got {1}, expected {2}"
                                .format(op_idx, trans.opcode_and_src_subsys_id, orig_trans.opcode_and_src_subsys_id))

            if trans.src_block_id_and_dest_subsys_id != orig_trans.src_block_id_and_dest_subsys_id:
                raise Exception("FAIL: src_block_id_and_dest_subsys_id mismatch at index {0}: got {1}, expected {2}"
                                .format(op_idx, trans.src_block_id_and_dest_subsys_id,
                                        orig_trans.src_block_id_and_dest_subsys_id))

            # This won't match, as stream ID may be different now.
            # if trans.dest_block_id_and_tag != orig_trans.dest_block_id_and_tag:
            #    raise Exception("dest_block_id_and_tag Failed at {0}: got {1} != orig {2}".format(
            #        op_idx, trans.dest_block_id_and_tag, orig_trans.dest_block_id_and_tag))

            if verify_payloads:
                for i in range(0, len(trans.payload)):
                    if trans.payload[i] != orig_trans.payload[i]:
                        raise Exception("FAIL: payload mismatch at index {0}, byte {1}: got {2}, expected {3}"
                                        .format(op_idx, i, trans.payload[i], orig_trans.payload[i]))

        return True

    def replay(self, recording, verify_payloads):
        """
        Replay a log recording.
        :param recording: a TrbLogReplay instance with the replayable log data
        :param verify_payloads: Whether to verify transaction payloads against the original. May be useful to turn off
        if the on-chip data is not precisely the same as it was originally.
        """
        self.had_error = False
        for op_idx, op in enumerate(recording):
            if op.op_type == "w":
                self.trb.write_raw_transactions(op.transactions, op.num_transactions)
            elif op.op_type == "r":
                if not self._do_read(op, op_idx, verify_payloads):
                    break
            elif op.op_type == "e":
                self.had_error = True
            else:
                raise Exception("Invalid op type")
