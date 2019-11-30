'''
Kalimba Command (kalcmd) generic message header
'''

import kalcmd2


class MsgHeader(kalcmd2.kalcmd_msg_header):
    '''
    Generic class for translating header as list[int] to kalcmd2.kalcmd_msg_header
    '''

    def __init__(self, message):
        self._message = message
        super(MsgHeader, self).__init__()

    def length(self):
        '''
        Returns:
            int: Length of the header in octets when implemented
        '''
        return len(self._message)

    def header(self):
        '''
        Returns:
            list[int]: actual header
        '''
        return self._message
