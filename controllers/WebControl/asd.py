import asyncio
import websockets
import socket

dhost=""
dport=5556

s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


def sender(msg):
	data=msg.split("\t")
	data=[float(i) for i in data]
	#print(data)
	LM=-data[1]
	RM=-data[1]
	LM=LM-data[0]
	RM=RM+data[0]
	LM=int(round(LM*25))
	RM=int(round(RM*25))
	SM=int(data[3])
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
	s.sendto(tosend, (dhost,dport))



async def hello(websocket, path):
	while(True):
		data = await websocket.recv()
		sender(data)

	

start_server = websockets.serve(hello, '', 8765)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()