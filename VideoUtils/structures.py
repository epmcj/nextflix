import math
import random
import bisect
import numpy as np
##################################################################################################
class Metadata:
	def __init__(self,height,width,nChannels,frameNums,nElements):
		#frame dimensions
		self.height = height
		self.width = width
		
		self.nChannels = nChannels
		
		#vectors that relates the number of the frame and the number of channelElements o
		#i-th data object
		self.frameNums = frameNums
		self.nElements = nElements

##################################################################################################
#single element of a single channel inside a data object
class ChannelElement:
	def __init__(self, P_column, D_value, Q_line):
		self.P_column = P_column
		self.D_value = D_value
		self.Q_line = Q_line
	
	def __len__(self):
		return 1
	
	def __eq__(self, other):
		return self.D_value==other.D_value
	
	def __lt__(self, other):
		return self.D_value<other.D_value
	
	def __le__(self, other):
		return self.D_value<=other.D_value
	
	def __ne__(self, other):
		return self.D_value!=other.D_value
	
	def __gt__(self, other):
		return self.D_value>other.D_value
	
	def __ge__(self, other):
		return self.D_value>=other.D_value
	
	#height,width of a frame
	def dim(self):
		height = len(self.P_column)
		width = len(self.Q_line.T)
		return height, width
	
	#D_value can be used as identifier and corresponds to the importance
	#of this data element
	def __getitem__(self, k):
		return self.D_value

##################################################################################################
#single channel inside a data object
class Channel:
	def __init__(self, channelElements):
		self.list = channelElements
	
	#dimensions of a frame
	def dim(self):
		if self.list:
			return self.list[0].dim()
		else:
			return 0,0
	
	def isEmpty(self):
		if(self.list):
			return False
		else:
			return True
	
	def __len__(self):
		return len(self.list)
	
	#only for test purposes. keep only n data elements
	def loseData(self,n):
		if(n>len(self)):
			print('The maximum number of elements is '+str(len(self)))
		else:
			self.list = self.list[:n]
	
	#only for test purposes
	def shuffleData(self):
		random.shuffle(self.list)
	
	#add more data about this channel of this frame. data is also a Channel object
	def insertData(self,data):
		#while there is data to be inserted
		while(not data.isEmpty()):
			#find the position to insert the new data
			pos = bisect.bisect_left(self.list,data.list[0])
			if(pos==len(self)):
				#insert the new data at the end
				self.list.append(data.list[0])
				data.list = data.list[1:]
			else:
				if(self.list[pos]==data.list[0]):
					#repeated data, just through it away
					#obs: we consider that the chances of two singular values
					#being exactly equal is too small to be considered
					data.list = data.list[1:]
				else:
					#insert the new data
						self.list.insert(pos,data.list[0])
						data.list = data.list[1:]
	
	#select [length] elements starting with [first]
	def getSubDataset(self,first,length):
		return Channel(self.list[first:(first+length)])
	
	#select [length] elements from the interval [first,last)
	def getRandomSubDataset(self,first,last,length,expoent,fixed):
		#no data to return
		if length<=0:
			return []
		#expected more data than we have
		if(last>len(self)):
			print('Warning: The last index is out of range. f:'\
				+str(first)+'; l:'+str(last)+'; s:'+str(len(self)))
			last = len(self)
		#expected more data than possible within the interval. Then, return the entire interval
		if(last-first<length):
			print('Warning: The redundant sub-message cannot be completly fulfilled')
			return Channel(self.list[first:last])
		#fixed is the number of data elements deterministically
		if(fixed>length):
			fixed = length
		#insert the fixed data
		subList = list(self.list[inicio:inicio+fixed])
		#The discrete probability distribution for the choice of the redundant data is
		#proportional to the square of the importance of each data element
		weights = []
		for i in range(first+fixed,last):
			w = self.list[i].D_value
			weights.append(w**expoent)
		S = sum(weights)
		
		#choose the data according to the weights
		refList = list(self.list[first+fixed:last])
		while(len(subList)!=length):
			rand = random.uniform(0,S)
			s = 0.0
			for i in range(len(weights)):
				s = s + weights[i]
				if rand<=s or i==len(weights)-1:
					subList.append(refList.pop(i))
					S = S-weights.pop(i)
					break
		return Channel(subList)

##################################################################################################
#slice of a frame (may be the entire frame)
class Data:
	def __init__(self, channels, frameNum):
		for i in range(1,len(channels)):
			if(len(channels[i])!=len(channels[i-1])):
				print('Inconsistant data')
				exit()
		self.channel = channels
		self.frame = frameNum
	
	#dimensions of the frame
	def dim(self):
		return self.channel[0].dim()
	
	#assumes this object has the same quantity of data about each channel
	def isEmpty(self):
		return self.channel[0].isEmpty()
	
	def totalSize(self):
		s = 0
		for channel in self.channel:
			s = s + len(channel)
		return s
		
	#only for test purposes. n is the number of elements to maintain
	def loseData(self,n):
		for channel in self.channel:
			channel.loseData(n)
	
	#only for test purposes
	def shuffleData(self):
		for channel in self.channel:
			channel.shuffleData()
		
	#add more data about this frame.
	def insertData(self,data):
		#if it does not refer to the same frame
		if(self.frame!=data.frame):
			print('Inconsistant data append')
			return()
		#insert the data of each channel into the corresponding channel within this object
		for i in range(len(self.channel)):
			self.channel[i].insertData(data.channel[i])
	
	#select [length] elements starting with [first]
	def getSubDataset(self,first,length):
		subChannels = []
		for channel in self.channel:
			subChannels.append(channel.getSubDataset(first,length))
		return(Data(subChannels, self.frame))
	
	#select [length] elements from the interval [first,last)
	def getRandomSubDataset(self,first,last,length,expoent,fixed):
		subChannels = []
		for channel in self.channel:
			subChannels.append(channel.getRandomSubDataset(first,last,length,expoent,fixed))
		return(Data(subChannels,self.frame))
	
	#segments the data of this object within ncat categories. Each category is formed by
	#n_msgs[cat] messages with msg_sizes[cat] elements and with 100*msg_redundancies[cat]%
	#of redundancy. Fixed is the number of data elements that will deterministically be chosen
	def createMsgs(self,ncat,n_msgs,msg_sizes,msg_redundancies,expoent,fixed):
		
		if(self.checkMsgMetadata(ncat,n_msgs,msg_sizes,msg_redundancies)):
			first = 0
			cats = []
			for cat in range(ncat):
				msgs = []
				for i in range(n_msgs[cat]):
					
					#the data for this category
					msg = self.getSubDataset(first,\
						int(math.floor((1-msg_redundancies[cat])*msg_sizes[cat])))
					
					#if there is redundancy to insert
					if(msg_redundancies[cat]>0):
						#the redundancy (a specific percentage of the message is composed by
						#random past elements)
						red = self.getRandomSubDataset(0,first,\
							int(math.ceil(msg_redundancies[cat]*msg_sizes[cat])),expoent,fixed)
						
						#merge the data from this category and the redundancy
						msg.insertData(red)
					
					#list of messages for this category
					if not msg.isEmpty():
						msgs.append(msg)
					
					first = first + int(math.floor((1-msg_redundancies[cat])*msg_sizes[cat]))
					
				#insert the new messages into the corresponding category
				cats.append(msgs)
			return cats
		else:
			print('Inconsistant msg metadata for createMsgs')
			return None
	
	#check is the parameters used with message generation are consistant
	def checkMsgMetadata(self,ncat,n_msgs,msg_sizes,msg_redundancies):
		ok = True
		#first level: the list must have one element for each category
		ok = ok and (ncat>0) and (len(n_msgs)==ncat) and (len(msg_sizes)==ncat) \
			and (len(msg_redundancies)==ncat)
		if not ok:
			return False
		else:
			#second level: first element of each list
			ok = ok and (n_msgs[0]>0) and (msg_sizes[0]>0) and (msg_redundancies[0]==0)
			if not ok:
				return False
			else:
				#third level: the rest of the lists must not decrease and must be positive.
				#The msg size also must be at least enough to suport the amount of data inside
				#this object. (only the redundancy may be 0)
				size = 0
				for cat in range(ncat):
					ok = ok and (n_msgs[cat]>0) and (msg_sizes[cat]>0)\
						and (msg_redundancies[cat]>=0)
					size = n_msgs[cat]*int(math.floor((1-msg_redundancies[cat])*msg_sizes[cat]))
				if (not ok) or (size>self.totalSize()):
					return False
				else:
					return True
	
	#generates a valid configuration for the Msg Metadata according to the informed parameters.
	#msg_sizes_base is the desired proportion between the elements of msg_sizes
	def generateNiceMsg_Sizes(self,ncat, n_msgs, msg_redundancies, msg_sizes_base):
		msg_sizes = np.ceil(float(self.totalSize()\
			/np.matmul(np.array(msg_sizes_base,dtype=float),\
				np.matmul(np.diag(1-np.array(msg_redundancies,dtype=float)),\
					np.array(n_msgs,dtype=float).T)))\
			*np.array(msg_sizes_base,dtype=float))
		return np.array(msg_sizes,dtype=int)
