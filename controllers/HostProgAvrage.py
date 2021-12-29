import socket
import time

fhost=''
fport=5555
dhost=""
dport=5556

def getTime():
	return time.perf_counter()

class Client:
	def __init__(self,address):
		self.address=address
		self.lastpacket=getTime()
	def setvals(self,LM,RM):
		self.LM=LM
		self.RM=RM
		self.lastpacket=getTime()



clients=[]
numofclients=0
sr=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ss=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sr.bind((fhost, fport))
Spin=0
while 1:
	message,address=sr.recvfrom(8192)
	address=address[0]
	data=message.decode().split(",")
	data=[float(i) for i in data]
	#print(data)
	LM=-data[3]
	RM=-data[3]
	LM=LM-data[2]
	RM=RM+data[2]
	#print(Spin,round(LM,2),round(RM,2))
	LM=int(round(LM*25))
	RM=int(round(RM*25))
	if(LM>255):
		LM=255
	elif(LM<-255):
		LM=-255
	if(RM>255):
		RM=255
	elif(RM<-255):
		RM=-255
	knownclient=False
	for client in clients:
		if(client.address==address):
			client.setvals(LM,RM)
			knownclient=True
	if(not knownclient):
		clients.append(Client(address))
		clients[len(clients)-1].setvals(LM,RM)
		numofclients=numofclients+1
		print("Added client: "+address)
	for client in clients:
		if(client.lastpacket+1<getTime()):
			print("Removed Client: "+client.address)
			clients.remove(client)
			numofclients=numofclients-1
	LM=0
	RM=0
	for	client in clients:
		LM=LM+(client.LM/numofclients)
		RM=RM+(client.RM/numofclients)
	LM=round(LM)
	RM=round(RM)
	tosend=(format(LM+255,"03d")+format(RM+255,"03d")+format(Spin+1,"01d")).encode()
	print(tosend)
	#ss.sendto(tosend, (dhost,dport))
	
#format(4, '03d')