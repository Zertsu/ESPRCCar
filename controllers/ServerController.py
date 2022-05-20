import socket
from inputs import get_gamepad
from threading import Thread
from time import sleep
import queue

class Contr:
    def __init__(self) -> None:
        self.LSX = 0
        self.LT  = 0
        self.RT  = 0
        self.LB  = 0
        self.RB  = 0


d = Contr()

def gamepad_reader():
    global d
    while True:
        events = get_gamepad()
        for event in events:
            if(event.code=="ABS_Z"): # Left trigger
                d.LT = event.state
            elif(event.code=="ABS_RZ"): # Right trigger
                d.RT = event.state
            elif(event.code=="ABS_X"): # Left joystick X
                d.LSX = event.state
            elif(event.code=="BTN_TL"): # Left bumper
                d.LB = event.state
            elif(event.code=="BTN_TR"): # Right bumper
                d.RB = event.state
    


host="192.168.0.185"
port=5556
s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
s.bind(("0.0.0.0", 16001))
t=Thread(target=gamepad_reader)
t.daemon=True
t.start()
data, remote = s.recvfrom(512)
print("got it", remote)
s.sendto("Hi".encode(), remote)
while 1:
    LM=int(d.RT - d.LT + d.LSX / 160)
    RM=int(d.RT - d.LT - d.LSX / 160)
    SM=int(d.LB - d.RB)
    t = 0
    if LM < 0:
        t |= 1
    if RM < 0:
        t |= 1 << 1
    if not SM == 0:
        t |= 1 << 3
        if SM < 0:
            t |= 1 << 2
    LM = abs(LM)
    RM = abs(RM)
    if(LM>255):
        LM=255
    if(RM>255):
        RM=255
    s.sendto(bytearray([2, LM, RM, t]), remote)
    sleep(0.1)