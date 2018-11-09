import cv2
import numpy as np

'''
decomposes a video identified using the corresponding filename and
produces a set of message payloads divided into ncat categories.
cat_metadata is a vector which relates each category to its number
of messages (per segment) and message size in bytes.
'''

def preProcess(fileName,ncat,cat_metadata):
	#open the video
	vidcap = cv2.VideoCapture(fileName)
	
	#get some parameters
	height = int(vidcap.get(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT))
	width = int(vidcap.get(cv2.cv.CV_CAP_PROP_FRAME_WIDTH))
	frameRate = vidcap.get(cv2.cv.CV_CAP_PROP_FPS)
	
	#get each frame and decompose it
	while True:
		success,frame = vidcap.read()
		if success:
			R,G,B = splitChannels(frame)
			
		else:
			break;
	
	vidcap.release()


