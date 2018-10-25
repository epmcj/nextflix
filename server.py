import socket
import struct

HOST = ''              # Endereco IP do Servidor
PORT = 5000            # Porta que o Servidor esta
udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
orig = (HOST, PORT)
udp.bind(orig)

while True:
    msg, client = udp.recvfrom(1024)
    value3, value1, value2 = struct.unpack_from("iif", msg)
    print(str(client) + " " + str(msg))
    print("v1={} v2={} v2={}".format(value1, value2, value3))

udp.close()