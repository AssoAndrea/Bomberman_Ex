from ctypes import sizeof
import socket
import struct
import time
import codecs
from tkinter import Pack
from turtle import update

from numpy import number, uint

#region exception
class InvalidPacketSize(Exception):
    pass


class DosAttemptDetected(Exception):
    pass

class InvalidAuthField(Exception):
    pass
#endregion






class Packet:
    def __init__(self,type,format,dimension):
        self.type = type
        self.format = format
        self.dimension = dimension

#packet type (type_id,format,dimension) second int is player_id
P_DISCONNECTED = Packet("DSCN","4sI",8)
P_KEEP_ALIVE = Packet("KEEP","4sI",8)
P_POSITION = Packet("PSTN","4sIff",16)
P_COLOR = Packet("COLR","4sIIII",20)
P_BOMB = Packet("BOMB","4sIff",16)
P_REQ_AUTH = Packet("RQAU","4s",4)
P_SND_AUTH = Packet("SEAU","4sI",8)

last_auth = 0

class Player:

    def __init__(self, signature):
        global last_auth
        self.signature = signature
        self.x = 0
        self.y = 0
        self.r = 0
        self.g = 0
        self.b = 0
        self.last_update = time.time()
        self.auth = last_auth + 1
        last_auth = self.auth
        self.warning = 0
        self.last_warning = None
        print("new user " + str(self.signature) + " with auth " + str(self.auth))
        




    def check_auth(self,auth):
        if self.auth is None:
            raise InvalidAuthField()
        elif self.auth != auth:
            raise InvalidAuthField()

    def keep_alive(self,auth):
        self.check_auth(auth)
        self.last_update = time.time()

    def update_pos(self, auth, x, y):
        self.check_auth(auth)
        self.x = x
        self.y = y
        self.last_update = time.time()

    def update_color(self,auth,r,g,b):
        self.check_auth(auth)
        self.r = r
        self.g = g
        self.b = b
        self.last_update = time.time()



class Server:

    def __init__(self, address='0.0.0.0', port=900, tolerance=20):

        self.address = address
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind((address, port))
        self.players = {}
        self.tolerance = tolerance


    def process_packet(self,packet,sender):

        type = struct.unpack_from("4s",packet)[0]
        type = codecs.decode(type,"UTF-8")
        
        if (len(packet)>4):
            auth = struct.unpack_from("I",packet,4)[0] #padding
        
        if(type == P_POSITION.type):
            if(len(packet)!= P_POSITION.dimension):
                raise InvalidPacketSize()
            else:
                _,_,x,y = struct.unpack(P_POSITION.format,packet)
                self.players[sender].update_pos(auth,x,y)
                new_packet = struct.pack(P_POSITION.format,bytes(type,'utf-8_'),auth,x,y)
                #print("position for {0} is {1}/{2}".format(auth,x,y))
                self.broadcast(sender,new_packet)
                return

        if(type == P_COLOR.type):
            if(len(packet)!= P_COLOR.dimension):
                raise InvalidPacketSize()
            else:
                _,_,r,g,b = struct.unpack(P_COLOR.format,packet)
                self.players[sender].update_color(auth,r,g,b)
                new_packet = struct.pack(P_COLOR.format,type,auth,r,g,b)
                self.broadcast(sender,new_packet)
                return

        if(type == P_KEEP_ALIVE.type):
            if(len(packet)!= P_KEEP_ALIVE.dimension):
                raise InvalidPacketSize()
            else:
                self.players[sender].keep_alive(auth)
                return

        if(type == P_REQ_AUTH.type):
            if(len(packet)!= P_REQ_AUTH.dimension):
                raise InvalidPacketSize()
            else:
                print("request for auth")
                auth_packet = struct.pack("4sI",bytes(P_SND_AUTH.type,"UTF-8"),self.players[sender].auth)
                sent = self.socket.sendto(auth_packet,self.players[sender].signature)
                return
        print("invalid packet type " + type)
                





    def run_once(self):
        try:
            packet, sender = self.socket.recvfrom(8192)

            if sender in self.players:
                if self.players[sender] is None:  # banned?
                    return
                now = time.time()
                if now - self.players[sender].last_update < (1 / self.tolerance):
                    self.players[sender].warning+=1
                    if self.players[sender].warning > 10:
                        raise DosAttemptDetected()
                if self.players[sender].warning is not None:
                    if now - self.players[sender].warning > 4:
                        self.players[sender].warning = 0
            else:
                self.players[sender] = Player(sender)
            
            self.process_packet(packet,sender)
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