import sys
import time
import socket
import random
from olist import OrderedList

BUFFER_SIZE     = 1024
SOCKET_TIMEOUT  = 2.0 # in seconds
FEEDBACK_PERIOD = 5.0 # in seconds
NEXT_TIMEOUT    = 0.5 # in seconds 

LIST_CODE = 7673 
PLAY_CODE = 8076
EXIT_CODE = 9999
NDEF_CODE =   -1

class NxtHeader:
    def __init__(self, seqNum):
        self.seq_num = seqNum

class NxtPayload:
    # MSG
    # DATA
    # CHANNEL
    def __init__(self, payload):
        return

class NxtPacket:
    def __init__(self, msg):
        seqNum  = bin2int(msg)
        payload = msg[4:]
        self.header  = NxtHeader(seqNum)
        self.payload = NxtPayload(payload)

# goes from binary (4 bytes) to int
def bin2int(buffer):
    vint = 0
    for i in range(4):
        vint = vint << 8
        vint += buffer[i]
    return vint

# goes from int to 4 bytes
def int2bin(value):
    return value.to_bytes(4, byteorder="big")

# receives video list from server
def get_video_list(sockt, server):
    vlist = []
    sockt.sendto(int2bin(LIST_CODE), server)
    msg, addr = sockt.recvfrom(BUFFER_SIZE)
    numVideos = bin2int(msg)
    for i in range(numVideos):
        msg, addr = sockt.recvfrom(BUFFER_SIZE)
        vid   = bin2int(msg[0:4])
        title = msg[4:].decode("ascii")
        vlist.append([vid, title])
    return vlist

# print video list to client
def print_video_list(vlist):
    print("Videos available:")
    for video in vlist:
        print("* (" + str(video[0]) + ") " + video[1])

# decompose a message into header + payload
def decompose_msg(msg):
    return NxtPacket(msg)


# play video from server while receiving it
def receive_and_play(vid, sockt, server):
    nextSeqNum   = 0
    lostMsgs     = 0
    fbDeadline   = time.time() + FEEDBACK_PERIOD
    nextDeadline = 0
    waitingList  = OrderedList(unique=True)
    
    sockt.sendto(int2bin(PLAY_CODE) + int2bin(vid), server)
    
    msg, addr  = sockt.recvfrom(BUFFER_SIZE)
    nextSeqNum = bin2int(msg)
    print("First seqNum = " + str(nextSeqNum))
    sockt.sendto(msg, server)

    nextDeadline = time.time() + NEXT_TIMEOUT
    
    while True:
        # receives a message from server and send it to be displayed
        msg, addr = sockt.recvfrom(BUFFER_SIZE)
        npckt = decompose_msg(msg)

        if npckt.header.seq_num < nextSeqNum:
            # received a duplicate

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
            lostMsgs     += 1
            while True:
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
            # ENVIAR SEQ-NUM DO ULTIMO PACOTE RECEBIDO ???
            sockt.sendto(int2bin(lostMsgs), server)
            # reset variables for next period
            lostMsgs = 0



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
sockt.sendto(int2bin(cid), server)
# trying to receive answer from server
try:
    msg, addr = sockt.recvfrom(BUFFER_SIZE)
except socket.timeout:
    print("Client: server failed to respond.")
    exit(1)
    
rport = bin2int(msg)
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
        ##
        # rotina de finalizacao
        ##
        sockt.sendto(int2bin(EXIT_CODE), server)
        break
    else:
        print("Unknown command. Try again.")
