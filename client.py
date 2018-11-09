import sys
import time
import socket
import random

BUFFER_SIZE     = 1024
SOCKET_TIMEOUT  = 2.0 
FEEDBACK_PERIOD = 5.0 

LIST_CODE = 7673 
PLAY_CODE = 8076
EXIT_CODE = 9999
NDEF_CODE =   -1

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
    # HEADER = {SEQ_NUM}
    # UTILIZAR struct PARA DECODIFICAR TUDO ??
    header = {}
    header["seqNum"] = bin2int(msg)
    # translate payload to frame message structure
    payload = msg[4:] # MODIFICAR !!!
    return header, payload


# play video from server while receiving it
def receive_and_play(vid, sockt, server):
    nextSeqNum = 0
    lostMsgs   = 0
    nextFBTime = time.time() + FEEDBACK_PERIOD
    
    sockt.sendto(int2bin(PLAY_CODE) + int2bin(vid), server)
    
    msg, addr  = sockt.recvfrom(BUFFER_SIZE)
    nextSeqNum = bin2int(msg)
    print("First seqNum = " + str(nextSeqNum))
    sockt.sendto(msg, server)
    return # REMOVER
    while True:
        # receives a message from server and send it to be displayed
        msg, addr = sockt.recvfrom(BUFFER_SIZE)
        header, payload = decompose_msg(msg)

        # verifies if it is time to send feedaback to the server
        if time.time() > nextFBTime:
            # set next feedback time
            nextFBTime += FEEDBACK_PERIOD
            # create message using the statistics

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
