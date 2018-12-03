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
	mutexDatas = None
	
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
			self.mutexDatas.acquire()
			try:
				#get the lower-frameNum data
				data = self.dataList.pop(0)
				#be sure that the list will never be empty
				if not self.dataList:
					ChannelList = []
					for i in range(self.nChannels):
						ChannelList.append(st.Channel([]))
					self.dataList.append(st.Data(ChannelList,data.frame+1))
			except:
				print('Error: Composition')
			finally:
				self.mutexDatas.release()
			print('Data {} composed'.format(data.frame))
			#recomposes the frame based on the received data
			success, frame = cod.composeFrame(data)
			
			if success:
				self.frameList.append(frame)
	
	#codes of interest for the frame displayer
	def getCode_displayer(self):
		self.mutexDatas.acquire()
		try:
			if self.quit:
				#the user wants to quit
				ret = -1
			else:
				if not self.frameList:
					#there aren't any more frame to show
					if self.finished and (self.dataList[0].isEmpty() and len(self.dataList) == 1):
						#the video is finished
						print("d-1 :fmlen = {}".format(len(self.frameList)))
						ret = -1
					else:
						#some frames are expected to arrive
						ret = 1
				else:
					#there are ready frames
					ret = 0
		finally:
			self.mutexDatas.release()
			return ret	
	
	#codes of interest for the frame composer
	def getCode_composer(self):
		self.mutexDatas.acquire()
		try:
			if self.quit:
				#the user wants to quit
				ret = -1
			else:
				if (self.dataList[0].isEmpty() and len(self.dataList) == 1):
					#there aren't any more frame to compose
					if self.finished:
						#the video is finished
						print("c-1 :dllen = {}".format(len(self.dataList)))
						ret = -1
					else:
						#some frames are expected to arrive
						ret = 1
				else:
					#there are ready frames
					if self.finished:
						ret = 2
					else:
						ret = 0
		finally:
			self.mutexDatas.release()
			return ret
	
	def write(self,data):
		#if the buffer has space
		if not self.isFull():
			#if the frame still matter
			if data.frame>=self.dataList[0].frame:
				self.mutexDatas.acquire()
				try:
					lastFrame = self.dataList[-1].frame
					#if there is not an initialized structure for this frame
					if data.frame>lastFrame:
						#creates the structures for each frame
						for frameNum in range(lastFrame+1,data.frame):
							ChannelList = [st.Channel([]) for i in range(self.nChannels)]
							self.dataList.append(st.Data(ChannelList,frameNum))
						self.dataList.append(data)
					else:
						#insert the data in the right place
						firstFrame = self.dataList[0].frame
						self.dataList[data.frame-firstFrame].insertData(data)
				finally:
					self.mutexDatas.release()
		
		print(len(self.dataList))

	def isFull(self):
		return (len(self.dataList)==self.maxSize)
	
	def isEmpty_d(self):
		return (self.dataList[0].isEmpty() and len(self.dataList) == 1)

