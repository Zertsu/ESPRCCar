import socket
from inputs import get_gamepad
from threading import Thread
from time import sleep
host=""
port=5556


s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
Z=0
ZR=0
X=0
TL=0
TR=0
tosend=b'2552551'

def keepAlive():
	while 1:
		s.sendto(tosend, (host,port))
		sleep(0.8)
		
t=Thread(target=keepAlive)
t.daemon=True
t.start()
while 1:
	events = get_gamepad()
	for event in events:
		if(event.code=="ABS_Z"):
			Z=event.state
		elif(event.code=="ABS_RZ"):
			ZR=event.state
		elif(event.code=="ABS_X"):
			X=event.state
		elif(event.code=="BTN_TL"):
			TL=event.state
		elif(event.code=="BTN_TR"):
			TR=event.state
	
	LM=int(ZR-Z+X/160)
	RM=int(ZR-Z-X/160)
	SM=int(TL-TR)
	if(LM>255):
		LM=255
	elif(LM<-255):
		LM=-255
	if(RM>255):
		RM=255
	elif(RM<-255):
		RM=-255
	tosend=(format(LM+255,"03d")+format(RM+255,"03d")+format(SM+1,"01d")).encode()
	print(tosend)	
	s.sendto(tosend, (host,port))