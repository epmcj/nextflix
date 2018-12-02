#import cv2
#import thread

import videoBuff as vb
import videoClient as vc
import sys
sys.path.insert(0,'../VideoPreProcessor')
import preProcess as pp

sys.path.insert(0,'../VideoUtils')
import dataSet as ds

def main():
	
	videoFile = '../assets/chaplinCut.mp4'
	grayScale = True
	
	frameRate = 30
	#the gap of frames that will be waited each time the video stops.
	#increase this value for the video to be more fluid
	framesBeforeStart = 30
	#The maximum number of objects that can wait for new data
	#increase this value if you want a better quality 
	receiveWindow = 30
	
	#reading the pre-processed data
	Cats=[]
	Success = False
	for catIndex in range(2):
		filename = ds.genFileName(videoFile,catIndex)
		meta, cat = ds.load(filename)
		if len(cat)>0:
			Cats.append(cat)
			Success = True
	
	#if id does not exists, create it
	if not Success:
		nCats = 2
		msgRedundancies = [0,0]
		exp = 2
		fixed = 1
		msgPeriods = [180,90]
		msgSize = [1,2]
		Cats = pp.preProcess(videoFile,nCats,msgPeriods,msgRedundancies,msgSize,exp,fixed,grayScale)
		#Cats = pp.preProcess(videoFile,2,[600,480,360,240,120],[0,0,0,0,0],[1,1.2,1.4,1.6,1.8],2,1,grayScale)
		# Cats = pp.preProcess(videoFile,5,[5,4,3,2,1],[0,0,0,0,0],[1,2,3,4,5],2,1)
		print('Saving data...')
		ds.dump(videoFile,Cats)
		
		#the video buffer
		buff = vb.Buff(10000, len(Cats[0][0].channel))
	else:
		#the video buffer
		buff = vb.Buff(10000, meta.nChannels)
	
	#the messages arrive
	for cat in Cats:
		for msg in cat:
			buff.write(msg)
	
	buff.finished = True
	
	vc.startDisplayMechanism(framesBeforeStart,receiveWindow,buff,frameRate)
	
	while buff.getCode_displayer()!=-1:
		pass
	
	print('Exiting application')	

main()
