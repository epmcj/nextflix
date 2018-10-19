import cv2
import numpy as np
import thread
import videoBuff as vb

#producer
def getFrames(vidcap,buff):
	while True:
		
		#if the other thread wants to quit
		if buff.getCode()==-1:
			break
		
		#only in order to not break buffer's limit
		if not buff.isFull():
			#read the next frame
			success,frame = vidcap.read()
			
			#if there is a new frame
			if success:
				#insert it in the buffer
				buff.write(frame)
			else:
				#wait the buffer to be consumed and quit
				buff.finished = True
				break

#consumer
def showFrames(height,width,buff,frameRate):
	
	while True:
		
		#wait for a new frame of for transmission end
		while True:
			cod = buff.getCode()
			if cod != 1:
				#there is a frame to show or the transmission finished
				break
		
		if cod==-1:
			#the transmission is finished
			break
		
		#get next frame
		cod,image = buff.read()
		#display the frame
		cv2.imshow("Nextflix", image)
		
		#wait until the next frame
		if cv2.waitKey(int(1000/frameRate)) != -1:
			#if the user pressed a key, quit
			buff.quit = True
			break

def main():
	
	#open the video
	vidcap = cv2.VideoCapture('../assets/sample.mp4')
	
	#get some parameters
	height = int(vidcap.get(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT))
	width = int(vidcap.get(cv2.cv.CV_CAP_PROP_FRAME_WIDTH))
	frameRate = vidcap.get(cv2.cv.CV_CAP_PROP_FPS)
	
	#create the buffer
	buff = vb.Buff(10000)
	
	cv2.namedWindow("Nextflix")	
	
	try:
	   thread.start_new_thread(getFrames, (vidcap,buff))
	   thread.start_new_thread(showFrames, (height,width,buff,frameRate))
	except:
	   print "Error: unable to start threads"
	
	while buff.getCode()!=-1:
		pass
	
	cv2.destroyWindow("Nextflix")
	vidcap.release()	

main()
