import cv2
import numpy as np

import sys
sys.path.insert(0,'../VideoUtils')

import structures as st
import codec as cod

'''
decomposes the video identified by the filename and
produces a set of message payloads divided into ncat categories.
n_msgs: number of messages of each category
msg_redundancies: redundancy ratio of each category
msg_sizes: message size within each category
expoent: a greater expoent prioritizes the choice of the most important values to be redundant
fixed: the [fixed] greater values will always be redundant in all categories
'''

def preProcess(fileName,ncat,n_msgs,msg_redundancies,msg_sizes,expoent,fixed, isGray):
	#open the video
	vidcap = cv2.VideoCapture(fileName)
	
	i=0 #frame number
	
	Cats = [[] for j in range(ncat)] #the scructure that will carry all data about the video
	
	while True:
		#get the next frame
		success,frame = vidcap.read()
		
		print('Starting frame '+str(i))
		if success:
			#go to grayscale if necessary
			if isGray:
				frame = frame[:,:,0]
			#pre-process frame
			data = cod.decomposeFrame(frame,i)
			#create the payloads of the messages
			cats = data.createMsgs(ncat,n_msgs,msg_sizes,msg_redundancies,expoent,fixed)
			#increment frame number
			i = i+1	
			#insert the messages in the main structure
			for j in range(len(cats)):
				for msg in cats[j]:
					Cats[j].append(msg)
		else:
			break;
	
	vidcap.release()
	return(Cats)
