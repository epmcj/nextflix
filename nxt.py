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
    CTRL_TYPE = b'\x05\x00\x00\x00'

class NxtHeader:
    def __init__(self, mtype, seqNum):
        self.type    = mtype
        self.seq_num = seqNum

    def to_buffer(self):
        buffer  = self.type
        buffer += int2bin_l(self.seq_num)     

class NxtPayload:
    # MSG
    # DATA
    # CHANNEL
    def __init__(self, payload):
        return

class NxtPacket:
    def __init__(self, msg):
        mtype   = msg[:4]
        seqNum  = bin2int_l(msg[4:9])
        payload = msg[9:]
        self.header  = NxtHeader(mtype, seqNum)
        self.payload = payload
        # self.payload = NxtPayload(payload)

    @classmethod
    def construct_to_buffer(self, mtype, seqNum, payload):
        buffer  = NxtHeader(mtype, seqNum).to_buffer()
        buffer += payload

    def to_buffer(self):
        buffer  = self.header.to_buffer()
        buffer += self.payload
        return buffer