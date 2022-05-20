import socket

fhost=''
fport=5555
dhost=""
dport=5556

sr=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ss=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sr.bind((fhost, fport))
Spin=0
while 1:
	message, address = sr.recvfrom(8192)
	data=message.decode().split(",")
	data=[float(i) for i in data]
	#print(data)
	LM=-data[3]
	RM=-data[3]
	LM=LM-data[2]
	RM=RM+data[2]
	try:
		if(data[6]<-500):
			Spin=1
		elif(data[6]>500):
			Spin=-1
		else:
			Spin=0
	except(IndexError):
		pass
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
	tosend=(format(LM+255,"03d")+format(RM+255,"03d")+format(Spin+1,"01d")).encode()
	print(tosend)
	ss.sendto(tosend, (dhost,dport))
	
#format(4, '03d')