from threading import Lock

class Buff:
	finished = False
	frameList = []
	maxSize = 0
	lock = None
	
	def __init__(self,maxSize):
		self.maxSize = maxSize
		self.lock = Lock()
	
	def read(self):
		if not self.frameList:
			if self.finished:
				return -1, None
			else:
				return 1, None
		else:
			self.lock.acquire()
			frame = self.frameList.pop(0)
			self.lock.release()
			return 0,frame
	
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
			self.lock.acquire()
			self.frameList.append(frame)
			self.lock.release()
	
	def end(self):
		self.finished = True
	
	def isFull(self):
		return (len(self.frameList)==self.maxSize)

