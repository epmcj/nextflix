import math
import random
import bisect

#single channel inside a data object
class Channel:
	def __init__(self, P_column, D_value, Q_line):
		self.P_column = P_column
		self.D_value = D_value
		self.Q_line = Q_line

#slice of a frame (may be the entire frame)
class Data:
	def __init__(self, rChannel, gChannel, bChannel, frameNum):
		if((len(rChannel)!=len(gChannel)) or (len(gChannel)!=len(bChannel))):
			print('Inconsistant data')
		self.channel = [bChannel, gChannel, rChannel]
		self.frame = frameNum
	
	def isEmpty(self):
		if(self.channel[0]):
			return False
		else:
			return True
	
	def getNumberOfElements(self):
		return(len(self.channel[0]))
		
	#only for test purposes. n is the number of elements to maintain
	def loseData(self,n):
		if(n>len(self.channel[0])):
			print('The maximum number of elements is '+str(len(self.channel[0])))
		else:
			for i in range(len(self.channel)):
				self.channel[i] = self.channel[i][:n]
	
	#only for test purposes
	def shuffleData(self):
		c = list(zip(self.channel[0],self.channel[1],self.channel[2]))
		random.shuffle(c)
		self.channel[0],self.channel[1],self.channel[2] = zip(*c)
		
	#add more data about this frame
	def insertData(self,data):
		
		#if it does not refer to the same frame
		if(self.frame!=data.frame):
			print('Inconsistant data append')
			return()
		
		#while there is data to be inserted
		while(data.channel[0]):
			#find the position to insert the new data
			pos = bisect.bisect_left(self.channel[0],data.channel[0][0])
			if(pos==len(self.channel[0])):
				#insert the new data at the end
				for i in range(len(self.channel)):
					self.channel[i].append(data.channel[i][0])
					data.channel[i] = data.channel[i][1:]
			else:
				if(self.channel[0][pos]==data.channel[0][0]):
					#repeated data, just through it away
					#obs: we consider that the chances of two singular values
					#being exactly equal is too small to be considered
					for i in range(len(self.channel)):
						data.channel[i] = data.channel[i][1:]
				else:
					#insert the new data
					for i in range(len(self.channel)):
						self.channel[i].insert(pos,data.channel[i][0])
						data.channel[i] = data.channel[i][1:]
	
	def checkMsgMetadata(self,ncat,n_msgs,msg_sizes,msg_redundancies):
		#the lists must have one element for each category
		if(len(n_msgs)!=ncat or len(msg_sizes)!=ncat or len(msg_redundancies)!=ncat or ncat<=0):
			disp('checkMsgMetadata: please review the length of the params')
			return False
		else:
			#the first category is the most superior. It cannot keep data from other categories
			if(msg_redundancies[0]!=0 or msg_redundancies[0]>=1):
				print('The first msg_redundancie is always 0')
				return False
			elif(msg_sizes[0]<=0):
				print('All msg sizes must be positive')
				return False
			elif(n_msgs[0]<=0):
				print('All n_msgs must be positive')
				return False
			else:
				#the content of the msgs is the content of this object plus the redundancy
				#check if the metadata is consistant with the amount of data in this object
				size = 0
				for cat in range(ncat):
					if(msg_redundancies[cat]<0 or msg_redundancies[cat]>=1):
						print('Please be sure that 0<=msg_redundancies<1')
						return False
					elif(msg_sizes[cat]<=0):
						print('All msg sizes must be positive')
						return False
					elif(n_msgs[cat]<=0):
						print('All n_msgs must be positive')
						return False
					else:
						size = size + int(math.floor(n_msgs[cat]*(1-msg_redundancies[cat])*msg_sizes[cat]))
				
				if(self.isEmpty()):
					print('This object is empty')
					return False
				elif(size<self.getNumberOfElements()):
					print('The proposed metadata does not match with the number of elements of the object')
					return False
				else:
					return True	
	
	#select [length] elements starting with [first]
	def getSubDataset(self,first,length):
		rChannel = self.channel[2][first:(first+length)]
		gChannel = self.channel[1][first:(first+length)]
		bChannel = self.channel[0][first:(first+length)]
		return(Data(rChannel, gChannel, bChannel, self.frame))
	
	#select [length] elements from the interval [first,last)
	#obs: I know this is not very efficient, but it will execute once and never again
	def getRandomSubDataset(self,first,last,length):
		c = list(zip(self.channel[0][first:last],self.channel[1][first:last],self.channel[2][first:last]))
		random.shuffle(c)
		bChannel,gChannel,rChannel = zip(*c)
		rChannel = rChannel[:length]
		gChannel = gChannel[:length]
		bChannel = bChannel[:length]
		return(Data(rChannel, gChannel, bChannel, self.frame))
	
	#segments the data of this object within ncat categories. Each category is formed by
	#n_msgs[cat] messages with msg_sizes[cat] elements and with 100*msg_redundancies[cat]%
	#of redundancy
	def createMsgs(self,ncat,n_msgs,msg_sizes,msg_redundancies):
		if(self.checkMsgMetadata(ncat,n_msgs,msg_sizes,msg_redundancies)):
			first = 0
			cats = []
			for cat in range(ncat):
				msgs = []
				for i in range(n_msgs[cat]):
					#the data for this category
					msg = self.getSubDataset(first,int(math.floor((1-msg_redundancies[cat])*msg_sizes[cat])))
					first = first + int(math.floor((1-msg_redundancies[cat])*msg_sizes[cat]))
					#the redundancy (a specific percentage of the message is composed by random past elements)
					red = self.getRandomSubDataset(0,first,int(math.ceil(msg_redundancies[cat]*msg_sizes[cat])))
					#merge the data from this category and the redundancy
					msg.insertData(red)
					
					msgs.append(msg)
				
				#insert the new messages into the corresponding category
				cats.append(msgs)
			return cats
		else:
			print('Inconsistant msg metadata for createMsgs')
			return None

