############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import types

class downloadFile(list):
    """Container format that extends the built-in list to inclue comment strings
    for the output kdc file."""

    def append(self, value, comment=""):
        """Extend the built-in list append method to include the comment.

        Note the behaviour of this method is different to the built-in method
        when handling lists. In this case the append method behaves as extend
        does."""
        if type(value) == list:
            list.append(self, (value[0], comment))
            self.extend(value[1:])
        else:
            list.append(self, (value, comment))

    def extend(self, value, comment=""):
        "Extend the built-in list extend method to include the comment."
        if len(value) == 0:
            return
        self.append(value[0], comment)
        list.extend( self, [(x, "") for x in value[1:]] )

    def dumpToTextFile(self, fileT, write_mode="w", index=0):
        with open(fileT, write_mode) as f:
            for i in range(len(self)):
                if self[i][1] != "":
                    f.write("%06X      %04X        # %s\n" % (index + i, self[i][0], self[i][1]))
                else:
                    f.write("%06X      %04X\n" % (index + i, self[i][0]))
        return index + len(self)

    def dumpToBinaryFile(self, fileB, write_mode="wb"):
        with open(fileB, write_mode) as f:
            f.write(b"".join([b"%c%c" % (k_s[0]>>8, k_s[0]&0xFF) for k_s in self]))
            
    def dumpToDRAMFile(self, fileD, write_mode="w"):
        with open(fileD, write_mode) as f:
            word32 = ""
            words16bit = ["%04x" % (k_s1[0]) for k_s1 in self]
            words32bit = []
            wordready = False
            for word16 in words16bit:
                word32 = word32 + word16
                if wordready == True:
                    words32bit.append("0x" + word32)
                    wordready = False
                    word32 = ""
                else:
                    wordready = True
            size = len(words32bit)
            words32bit.insert(0, "@40000000 " + str(size))
            f.write( "\n".join( words32bit ) )
            f.write("\n")
            # return whether we left a 16 bit word behind
            return wordready, word16, len(words32bit)