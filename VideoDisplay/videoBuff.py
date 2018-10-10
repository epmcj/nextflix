class Buff:
	finished = False
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
			return 0,self.frameList.pop()
	
	def getCode(self):
		if not self.frameList:
			if self.finished:
				return -1
			else:
				return 1
		else:
			return 0
	
	def write(self,frame):
		if not self.isFull():
			self.frameList.append(frame)
	
	def end(self):
		self.finished = True
	
	def isFull(self):
		return (len(self.frameList)==self.maxSize)

