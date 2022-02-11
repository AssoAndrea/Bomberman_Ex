import struct
from enum import Enum

class PacketType(Enum):
    DISCONNECTED = -1
    POSITION = {1,"ff"}
    COLOR = {2,"III"}
    BOMB = {3,"ff"}

TEST = (1,"FF")

print(TEST[1])

packed = struct.pack("III",3,4,5)
unpacked = struct.unpack_from("I",packed,0)

print(unpacked)