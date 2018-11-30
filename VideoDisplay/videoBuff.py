import sys
sys.path.insert(0,'../VideoUtils')

import structures as st
import codec as cod

class Buff:
	finished = False
	quit = False
	dataList = []
	maxSize = 0
	firstFrame = 0
	
	def __init__(self,maxSize):
		if maxSize==0:
			print('Please make sure that maxSize is greater than 0')
			exit(0)
		
		if self.maxSize != 0:
			print('This is a singleton class')
			exit(0)
		
		self.maxSize = maxSize
		#creates the structure for frame 0 data
		self.dataList.append(st.Data([st.Channel([]),st.Channel([]),st.Channel([])],0))
	
	def read(self):
		code = self.getCode()
		if code==0:
			#get the lower-frameNum data
			data = self.dataList.pop(0)
			#move the pointer for the next frame
			self.firstFrame = self.firstFrame+1
			#be sure that the list will never be empty
			if not self.dataList:
				self.dataList.append(st.Data([\
					st.Channel([]),st.Channel([]),st.Channel([])],self.firstFrame))
			
			#recomposes the frame based on the received data
			success, frame = cod.composeFrame(data)
			
			if not success:
				return -1, None
			else:
				return 0, frame
		else:
			return code,Node
	
	def getCode(self):
		if self.quit:
			#the user wants to quit
			return -1
		else:
			if self.dataList[0].isEmpty():
				#there aren't any more frame to show
				if self.finished:
					#the video is finished
					return -1
				else:
					#some frames are expected to arrive
					return 1
			else:
				#there are ready frames
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
						self.dataList.append(st.Data([\
							st.Channel([]),st.Channel([]),st.Channel([])],frameNum))
					self.dataList.append(data)
				else:
					#insert the data in the right place
					self.dataList[data.frame-self.firstFrame].insertData(data)
	
	def isFull(self):
		return (len(self.dataList)==self.maxSize)

