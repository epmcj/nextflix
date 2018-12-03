import sys
import time
import socket
import random
import struct
import VideoDisplay.videoClient as vc
import VideoDisplay.videoBuff   as vb
from VideoUtils.structures import Metadata
from VideoUtils.dataSet    import dataFromFloatArray
# for python3
from src.client_py.nxt     import *
from src.client_py.common  import *
from src.client_py.olist   import OrderedList
# for python2.7
# sys.path.insert(0,'./src/client_py')
# from nxt     import *
# from common  import *
# from olist   import OrderedList
######################################################################

BUFFER_SIZE     = 100000
SOCKET_TIMEOUT  = 10.0 # in seconds
FEEDBACK_PERIOD = 10.0 # in seconds
NEXT_TIMEOUT    = 1.0 # in seconds 
FLOAT_SIZE      = 4

class Category:
    def __init__(self, id, frameHeight, frameWidth, nChannels, nObjs, nElmts, 
                 frameNums):
        self.id          = id         
        self.frameHeight = frameHeight
        self.frameWidth  = frameWidth
        self.nChannels   = nChannels
        self.nObjs       = nObjs
        self.nElmts      = nElmts
        self.frameNums   = frameNums
        self.metadata    = Metadata(frameHeight, frameWidth, nChannels, 
                                    frameNums, nElmts)
    
    def __init__(self, byteArray):
        print("ba size= {}".format(len(byteArray)))
        self.id          = bin2int_l(byteArray[0:4])         
        self.frameHeight = bin2int_l(byteArray[4:8])
        self.frameWidth  = bin2int_l(byteArray[8:12])
        self.nChannels   = bin2int_l(byteArray[12:16])
        self.nObjs       = bin2int_l(byteArray[16:20])
        # the remaining of the array is composed by two arrays of same length
        fieldSize = int((len(byteArray) - (5 * 4)) / 2)
        b = 20
        self.nElmts = []
        for i in range(int(fieldSize/1)): # 4 bytes for each entry
            bi = b + i * 1
            self.nElmts.append(bin2int_l(byteArray[bi:bi+1]+b'\x00\x00\x00')) 
        b = b + fieldSize
        self.frameNums = []
        for i in range(int(fieldSize/1)): # 4 bytes for each entry
            bi = b + i * 1
            self.frameNums.append(bin2int_l(byteArray[bi:bi+1]+b'\x00\x00\x00')) 
        self.metadata = Metadata(self.frameHeight, self.frameWidth, 
                                 self.nChannels, self.frameNums, self.nElmts)

def decode_data_payload(payload):
    size  = bin2int_l(payload[0:4])
    cat   = bin2int_l(payload[4:8])
    index = bin2int_l(payload[8:12])
    fmt = "<{0:d}f".format(size)
    data = struct.unpack_from(fmt, payload[12:])
    return size, cat, index, data

# receives video list from server
def get_video_list(sockt, server):
    vlist = []
    sockt.sendto(int2bin_l(NxtCode.LIST_CODE), server)
    msg, addr = sockt.recvfrom(BUFFER_SIZE)
    numVideos = bin2int_l(msg)
    for i in range(numVideos):
        msg, addr = sockt.recvfrom(BUFFER_SIZE)
        vid   = bin2int_l(msg[0:4])
        title = msg[4:].decode("ascii")
        vlist.append([vid, title])
    return vlist

# print video list to client
def print_video_list(vlist):
    print("Videos available:")
    for video in vlist:
        print("* (" + str(video[0]) + ") " + video[1])

# play video from server while receiving it
def receive_and_play(vid, sockt, server):
    nextSeqNum   = 0
    lostMsgs     = 0
    rcvdMsgs     = 0 # for data msgs only
    fbDeadline   = time.time() + FEEDBACK_PERIOD
    nextDeadline = 0
    waitingList  = OrderedList(unique=True)
    catInfo      = {}
    ncats        = 0
    frameRate = 20
    #the gap of frames that will be waited each time the video stops.
    #increase this value for the video to be more fluid
    framesBeforeStart = 72
    #The maximum number of objects that can wait for new data
    #increase this value if you want a better quality 
    receiveWindow = 30

    buff = vb.Buff(10000,1)
    vc.startDisplayMechanism(framesBeforeStart, receiveWindow, buff, frameRate)

    sockt.sendto(int2bin_l(NxtCode.PLAY_CODE) + int2bin_l(vid), server)

    nextDeadline = time.time() + NEXT_TIMEOUT
    
    while True:
        # receives a message from server and send it to be displayed
        msg, addr = sockt.recvfrom(BUFFER_SIZE)
        npckt = NxtPacket(msg)
        print("Received a {} msg with size = {}".format(
                NxtType.to_string(npckt.header.type), str(len(msg))))

        if npckt.header.type == NxtType.FIN_TYPE:
            # ending routine
            print("Client: received {} msgs".format(rcvdMsgs))
            break
        
        elif npckt.header.type == NxtType.CTRL_TYPE:
            # LOST OF MESSAGES ARE A PROBLEM HERE !!
            cat = Category(npckt.payload)
            catInfo[cat.id] = cat
            # print("adding cat")
            # print("id = {}".format(cat.id))
            # print("frameHeight = {}".format(cat.frameHeight))
            # print("frameWidth = {}".format(cat.frameWidth))
            # print("nChannels = {}".format(cat.nChannels))
            # print("nObjs = {}".format(cat.nObjs))
            # print("nElmts[0] = {}".format(cat.nElmts[0]))
            # print("frameNums[0] = {}".format(cat.frameNums[0]))

        elif npckt.header.type == NxtType.INIT_TYPE:
            nextSeqNum = npckt.header.seq_num
            ncats      = bin2int_l(npckt.payload)
            print("{} categories".format(ncats))
            # return msg to confirm it
            sockt.sendto(msg, server)

        elif npckt.header.type == NxtType.DATA_TYPE:
            rcvdMsgs += 1
            # decoding payload
            _, cat, index, data = decode_data_payload(npckt.payload)
            buff.write(dataFromFloatArray(data, catInfo[cat].metadata, index))

            if npckt.header.seq_num < nextSeqNum:
                # received a duplicate
                print("Client: duplicated msg received (", end="")
                print("expected = " + str(nextSeqNum))
                print("got = " + str(npckt.header.seq_num) + ")")

            else:
                if npckt.header.seq_num == nextSeqNum:
                    # received the next expected msg
                    nextSeqNum   += 1
                    nextDeadline += NEXT_TIMEOUT

                elif npckt.header.seq_num > nextSeqNum:
                    # received a valid msg
                    waitingList.add(npckt.header.seq_num)

        # msg was lost
        if time.time() > nextDeadline:
            lostMsgs += 1
            while not waitingList.is_empty():
                nextSeqNum += 1
                if nextSeqNum < waitingList.get_min():
                    break
                elif nextSeqNum > waitingList.get_min():
                    print("Client: nextSeqNum > min(waitingList)")
                    exit(1)
                waitingList.remove_min()
            nextDeadline += NEXT_TIMEOUT

        # verifies if it is time to send feedaback to the server
        if time.time() > fbDeadline:
            # set next feedback time
            fbDeadline += FEEDBACK_PERIOD
            # create message using the statistics
            msg = NxtPacket.construct_to_buffer(NxtType.FBCK_TYPE, 0, 
                                                int2bin_l(lostMsgs))
            # ENVIAR SEQ-NUM DO ULTIMO PACOTE RECEBIDO ???
            sockt.sendto(msg, server)
            # reset variables for next period
            lostMsgs = 0
    
    # finishing
    buff.finished = True
    while buff.getCode_displayer()!=-1:
        pass


if len(sys.argv) < 4:
    print("Missing required parameters:" + \
          "[SERVER ADDR] [SERVER PORT] [CLIENT PORT]")
    exit()

cid = random.randint(100, 1000)
print("Client: id = " + str(cid))
# reading information for the connection between client and server
saddr = sys.argv[1]
sport = int(sys.argv[2])
cport = int(sys.argv[3])

server = (saddr, sport)
client = ("", cport)

# creating socket for communcation
sockt = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sockt.bind(client)
sockt.settimeout(SOCKET_TIMEOUT)
print("Timeout: " + str(sockt.gettimeout()))
# sending hello message
sockt.sendto(int2bin_l(cid), server)
# trying to receive answer from server
try:
    msg, addr = sockt.recvfrom(BUFFER_SIZE)
except socket.timeout:
    print("Client: server failed to respond.")
    exit(1)
    
rport = bin2int_l(msg)
if rport < 0:
    print("Client: connection denied.")
    exit()
print("Client: should connect to port " + str(rport))

# updating server info
server = (saddr, rport)

while True:
    rawInput = input("Command: ")
    # extracting info from the client command
    args = rawInput.split(" ")
    cmd  = args[0]
    # 
    if cmd == "list":
        vlist = get_video_list(sockt, server)
        print_video_list(vlist)

    elif cmd == "play":
        if len(args) < 2:
            print("Missing video title")
            continue
        vid = int(args[1])
        # receive video
        receive_and_play(vid, sockt, server)

    elif cmd == "exit":
        print("Bye")
        # finishing routine
        sockt.sendto(int2bin_l(NxtCode.EXIT_CODE), server)
        break
    else:
        print("Unknown command. Try again.")
