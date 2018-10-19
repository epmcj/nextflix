class Buff:
	finished = False
	quit = False
	frameList = []
	maxSize = 0
	
	def __init__(self,maxSize):
		self.maxSize = maxSize
	
	def read(self):
		if not self.frameList:
			if self.finished:
				return -1, None
			else:
				return 1, None
		else:
			frame = self.frameList.pop(0)
			return 0,frame
	
	def getCode(self):
		if self.quit:
			#the user wants to quit
			return -1
		else:
			if not self.frameList:
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
	
	def write(self,frame):
		if not self.isFull():
			self.frameList.append(frame)
	
	def isFull(self):
		return (len(self.frameList)==self.maxSize)

