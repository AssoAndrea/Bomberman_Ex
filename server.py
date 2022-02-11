from ctypes import sizeof
import socket
import struct
import time
from tkinter import Pack
from turtle import update

from numpy import uint

#region exception
class InvalidPacketSize(Exception):
    pass


class DosAttemptDetected(Exception):
    pass

class InvalidAuthField(Exception):
    pass
#endregion


# T_DISCONNECTED = {-1,"I",4}
# T_POSITION = {1,"Iff",12}
# T_COLOR = {2,"IIII",16}
# T_BOMB = {3,"Iff",12}



class Packet:
    def __init__(self,type,format,dimension):
        self.type = type
        self.format = format
        self.dimension = dimension

#packet type (type_id,format,dimension) second int is player_id
P_DISCONNECTED = Packet(-1,"II",8)
P_POSITION = Packet(1,"IIff",16)
P_COLOR = Packet(2,"IIIII",20)
P_BOMB = Packet(3,"IIff",16)

class Player:

    def __init__(self, signature):
        self.signature = signature
        self.x = 0
        self.y = 0
        self.r = 0
        self.g = 0
        self.b = 0
        self.last_update = None
        self.auth = None
        print('new player', self.signature)

    def update_auth(self,auth):
        if self.auth is None:
            self.auth = auth
            print('auth for {0} is {1}', self.signature, self.auth)
        elif self.auth != auth:
            raise InvalidAuthField()

    def update_pos(self, auth, x, y):
        self.x = x
        self.y = y
        self.update_auth(auth)
        self.last_update = time.time()
        print(self.signature, self.x, self.y)

    def update_color(self,auth,r,g,b):
        self.r = r
        self.g = g
        self.b = b
        self.update_auth(auth)
        self.last_update = time.time()
        print(self.signature, self.x, self.y)



class Server:

    def __init__(self, address='0.0.0.0', port=900, tolerance=20):
        self.address = address
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind((address, port))
        self.players = {}
        self.tolerance = tolerance


    def process_packet(self,packet,sender):
        type,auth = struct.unpack_from("II",packet,0)
        
        if(type == P_POSITION.type):
            if(len(packet)!= P_POSITION.dimension):
                raise InvalidPacketSize()
            else:
                _,_,x,y = struct.unpack(P_POSITION.format,packet)
                self.players[sender].update_pos(auth,x,y)
                return struct.pack(P_POSITION.format,type,auth,x,y)
        if(type == P_COLOR.type):
            if(len(packet)!= P_COLOR.dimension):
                raise InvalidPacketSize()
            else:
                _,_,r,g,b = struct.unpack(P_COLOR.format,packet)
                self.players[sender].update_pos(auth,x,y)
                return struct.pack(P_COLOR.format,type,auth,r,g,b)
                





    def run_once(self):
        try:
            packet, sender = self.socket.recvfrom(64)

            if sender in self.players:
                if self.players[sender] is None:  # banned?
                    return
                now = time.time()
                if now - self.players[sender].last_update < (1 / self.tolerance):
                    raise DosAttemptDetected()
            else:
                self.players[sender] = Player(sender)
            
            new_packet = self.process_packet(packet,sender)
            self.broadcast(sender,new_packet)
            self.check_dead_clients()
        except DosAttemptDetected:
            print('Dos detected from {0}, kicking it out'.format(sender))
            # del(self.players[sender])
            self.players[sender] = None  # banned!
        except InvalidAuthField:
            print('Invalid packet auth detected from {0}'.format(sender))
        except InvalidPacketSize:
            print('Invalid packet size detected from {0}'.format(sender))
        except OSError:
            import sys
            print(sys.exc_info())
            print('packet discarded')

    def broadcast(self, sender, packet):
        for signature in self.players:
            if self.players[signature] is not None:
                if signature != sender:  # avoid loop
                    self.socket.sendto(packet,signature)

    def check_dead_clients(self):
        dead_clients = []
        now = time.time()
        for signature in self.players:
            if self.players[signature] is not None:
                if self.players[signature].last_update + 10 < now:
                    dead_clients.append(signature)

        for dead_client in dead_clients:
            print('{0} is dead, removing from the list of players'.format(dead_client))
            del(self.players[dead_client])

    def run(self):
        print('running Bomberman server...')
        while True:
            self.run_once()


a = 17

if __name__ == '__main__':
    server = Server()
    server.run()
