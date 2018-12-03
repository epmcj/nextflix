import cv2
# import thread # for python 2.7 !!!
import threading

import videoBuff as vb

#import sys
#sys.path.insert(0,'../VideoPreProcessor')
#import preProcess as pp

#sys.path.insert(0,'../VideoUtils')
#import dataSet as ds

def startDisplayMechanism(framesBeforeStart,receiveWindow,buff,frameRate):
	try:
	# for python 2.7 !!!
	#    thread.start_new_thread(frameComposer, (framesBeforeStart,receiveWindow,buff))
	#    thread.start_new_thread(frameDisplayer, (framesBeforeStart,buff,frameRate))   
	   threading.Thread(target=frameComposer,
						args=(framesBeforeStart,receiveWindow,buff)
						).start()
	   threading.Thread(target=frameDisplayer,
        				args=(framesBeforeStart,buff,frameRate)
    				   	).start()
	except:
	   print("Error: unable to start threads")

#producer
def frameComposer(framesBeforeStart,receiveWindow,buff):
	print('Opening frame composer')
	'''
	#this avoids the use of poor data in the beginning
	cod = buff.getCode_composer()
	while len(buff.dataList)<receiveWindow and cod!=2:
		cod = buff.getCode_composer()
		if cod==-1:
			break
	'''
	while True:
		#if the other thread wants to quit
		cod = buff.getCode_composer()
		if cod==-1:
			break
		'''
		if len(buff.frameList)<framesBeforeStart:
			#compose more frames if there are few frames to display
			buff.composeNextFrame()
		el
		'''
		if len(buff.dataList)>receiveWindow or cod==2:
			#compose more frames if dataList is too big or there are no
			#more data to arrive
			buff.composeNextFrame()
	print('Closing frame composer')

#consumer
def frameDisplayer(framesBeforeStart,buff,frameRate):
	print('Opening frame displayer')
	
	#if true, wait framesBeforeStart
	waiting = True
	
	cv2.namedWindow("Nextflix")
	while True:
		cod = buff.getCode_displayer()
		if cod==1:
			waiting = True
		elif not waiting:
			if cod == 0:
				#display the frame
				cv2.imshow("Nextflix", buff.read())
			elif cod==-1:
				#the transmission is finished
				break
		elif waiting and ((len(buff.frameList)>framesBeforeStart) or buff.isEmpty_d()):
			waiting = False
			if cod == 0:
				#display the frame
				cv2.imshow("Nextflix", buff.read())
			elif cod==-1:
				#the transmission is finished
				break
		
		#wait until the next frame
		if cv2.waitKey(int(1000/frameRate)) != -1:
			#if the user pressed a key, quit
			buff.quit = True
			break
	
	cv2.destroyWindow("Nextflix")
	print('Closing frame displayer')
