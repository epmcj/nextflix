from threading import Lock
import sys

sys.path.insert(0,'../')
import VideoUtils.structures as st
import codec as cod

class Buff:
	finished = False
	quit = False
	dataList = []
	frameList = []
	maxSize = 0
	firstFrame = 0
	mutexDatas = None
	mutexFrames = None
	
	def __init__(self,maxSize,nChannels):
		if maxSize==0:
			print('Please make sure that maxSize is greater than 0')
			exit(0)
		
		if self.maxSize != 0:
			print('This is a singleton class')
			exit(0)
		
		self.nChannels = nChannels
		
		self.maxSize = maxSize
		#creates the structure for frame 0 data
		ChannelList = []
		for i in range(self.nChannels):
			ChannelList.append(st.Channel([]))
		self.dataList.append(st.Data(ChannelList,0))
		
		self.mutexDatas = Lock()
		self.mutexFrames = Lock()
	
	#wait until nFrames be ready to display
	def waitReadyFrames(self, nFrames):
		while len(self.frameList)<nFrames or self.quit:
			pass
	
	#generates the next frame
	def read(self):
		code = self.getCode_displayer()
		if code==0:
			#get the lower-frameNum data
			return self.frameList.pop(0)
		else:
			return None
	
	#for the frame composer
	def composeNextFrame(self):
		code = self.getCode_composer()
		if code==0 or code==2:
			#be sure that the list will never be empty
			if len(self.dataList)==1:
				ChannelList = []
				for i in range(self.nChannels):
					ChannelList.append(st.Channel([]))
				self.dataList.append(st.Data(ChannelList,self.firstFrame))
			
			#get the lower-frameNum data
			data = self.dataList.pop(0)
			#move the pointer for the next frame
			self.firstFrame = self.firstFrame+1
			
			#recomposes the frame based on the received data
			success, frame = cod.composeFrame(data)
			
			if success:
				self.frameList.append(frame)
	
	#codes of interest for the frame displayer
	def getCode_displayer(self):
		if self.quit:
			#the user wants to quit
			return -1
		else:
			if not self.frameList:
				#there aren't any more frame to show
				if self.finished and self.dataList[0].isEmpty():
					#the video is finished
					return -1
				else:
					#some frames are expected to arrive
					return 1
			else:
				#there are ready frames
				return 0
	
	#codes of interest for the frame composer
	def getCode_composer(self):
		if self.quit:
			#the user wants to quit
			return -1
		else:
			if self.dataList[0].isEmpty():
				#there aren't any more frame to compose
				if self.finished:
					#the video is finished
					return -1
				else:
					#some frames are expected to arrive
					return 1
			else:
				#there are ready frames
				if self.finished:
					return 2
				else:
					return 0
	
	def write(self,data):
		#if the buffer has space
		if not self.isFull():
			#if the frame still matter
			if data.frame>=self.firstFrame:
				lastFrame = self.firstFrame+len(self.dataList)-1
				
				#if there is not an initialized structure for this frame
				if data.frame>lastFrame:
					#creates the structures for each frame
					for frameNum in range(lastFrame+1,data.frame):
						ChannelList = []
						for i in range(self.nChannels):
							ChannelList.append(st.Channel([]))
						self.dataList.append(st.Data(ChannelList,frameNum))
					self.dataList.append(data)
				else:
					#insert the data in the right place
					self.dataList[data.frame-self.firstFrame].insertData(data)
	
	def isFull(self):
		return (len(self.dataList)==self.maxSize)
	
	def isEmpty_d(self):
		return self.dataList[0].isEmpty()

