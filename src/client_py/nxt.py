from common import *#int2bin_l, bin2int_l

class NxtCode:
    LIST_CODE = 7673 
    PLAY_CODE = 8076
    EXIT_CODE = 9999
    NDEF_CODE =   -1

class NxtType:
    INIT_TYPE = b'\x00\x00\x00\x00'
    DATA_TYPE = b'\x01\x00\x00\x00'
    FIN_TYPE  = b'\x02\x00\x00\x00'
    FBCK_TYPE = b'\x04\x00\x00\x00'
    CTRL_TYPE = b'\x08\x00\x00\x00'

    @classmethod
    def to_string(self, mtype):
        if mtype == self.INIT_TYPE:
            return "INIT_TYPE"
        elif mtype == self.DATA_TYPE:
            return "DATA_TYPE"
        elif mtype == self.FIN_TYPE:
            return "FIN_TYPE"
        elif mtype == self.FBCK_TYPE:
            return "FBCK_TYPE"
        elif mtype == self.CTRL_TYPE:
            return "CTRL_TYPE"
        return "UNKN_TYPE {}".format(mtype)


class NxtHeader:
    def __init__(self, mtype, seqNum):
        self.type    = mtype
        self.seq_num = seqNum

    def to_buffer(self):
        buffer  = self.type
        buffer += int2bin_l(self.seq_num)  
        return buffer   


class NxtPacket:
    def __init__(self, msg):
        mtype   = msg[:4]
        seqNum  = bin2int_l(msg[4:8])
        payload = msg[8:]
        self.header  = NxtHeader(mtype, seqNum)
        self.payload = payload
        self.psize   = len(msg) - 8 # msg size - header size

    @classmethod
    def construct_to_buffer(self, mtype, seqNum, payload):
        buffer  = NxtHeader(mtype, seqNum).to_buffer()
        buffer += payload
        return buffer

    def to_buffer(self):
        buffer  = self.header.to_buffer()
        buffer += self.payload
        return buffer