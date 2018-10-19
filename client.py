import sys
import socket
import random

BUFFER_SIZE    = 1024
SOCKET_TIMEOUT = 2.0 

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
sockt.sendto(str(cid), server)

try:
    msg, addr = sockt.recvfrom(BUFFER_SIZE)
except socket.timeout:
    print("Client: server failed to respond.")
    exit(1)
    
rport = int(msg)
if rport < 0:
    print("Client: connection denied.")
    exit()
print("Client: should connect to port " + str(rport))