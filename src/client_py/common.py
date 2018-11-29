# goes from binary (4 bytes) to int
def bin2int_b(buffer):
    vint = 0
    for i in range(4):
        vint = vint << 8
        vint += buffer[i]
    return vint

def bin2int_l(buffer):
    vint = 0
    for i in range(4):
        vint = vint << 8
        vint += buffer[3-i]
    return vint

# goes from int to 4 bytes
def int2bin_b(value):
    return value.to_bytes(4, byteorder="big")

def int2bin_l(value):
    return value.to_bytes(4, byteorder="little")