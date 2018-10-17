import cv2
import numpy as np
import thread
import videoBuff as vb

#producer
def getFrames(vidcap,buff):
	i = 0;
	while True:
		
		#if the other thread wants to quit
		if buff.getCode()==-1:
			break
		
		#only in order to not break buffer's limit
		if not buff.isFull():
			#read the next frame
			vidcap.set(cv2.cv.CV_CAP_PROP_POS_FRAMES, i)
			success,frame = vidcap.read()
			i = i+1
			
			#if there is a new frame
			if success:
				#insert it in the buffer
				buff.write(frame)
			else:
				#quit
				buff.end()
				break

#consumer
def showFrames(height,width,buff,frameRate):
	cv2.namedWindow("Nextflix")
	
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
			buff.end()
			break
	
	cv2.destroyWindow("Nextflix")

def main():
	
	#open the video
	vidcap = cv2.VideoCapture('../assets/sample.mp4')
	
	#get some parameters
	height = int(vidcap.get(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT))
	width = int(vidcap.get(cv2.cv.CV_CAP_PROP_FRAME_WIDTH))
	frameRate = vidcap.get(cv2.cv.CV_CAP_PROP_FPS)
	
	#create the buffer
	buff = vb.Buff(100)
	
	try:
	   thread.start_new_thread(getFrames, (vidcap,buff))
	   thread.start_new_thread(showFrames, (height,width,buff,frameRate))
	except:
	   print "Error: unable to start threads"
	
	while buff.getCode()!=-1:
		pass	

main()
