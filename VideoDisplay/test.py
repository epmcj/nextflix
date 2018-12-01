import sys
sys.path.insert(0,'../VideoPreProcessor')
import preProcess as pp

sys.path.insert(0,'../VideoUtils')
import dataSet as ds

import videoBuff as vb

import cv2

videoFile = '../assets/sample2.mp4'

frameRate = 30

Cats=[]
Success = False
for catIndex in range(5):
	filename = ds.genFileName(videoFile,catIndex)
	print('Loading '+filename)
	meta, cat = ds.load(filename)
	if len(cat)>0:
		Cats.append(cat)
		Success = True

if not Success:
	Cats = pp.preProcess(videoFile,5,[5,4,3,2,1],[0,0,0,0,0],[1,2,3,4,5],2,1)
	print('Saving data...')
	ds.dump(videoFile,Cats)

buff = vb.Buff(10000)

for cat in Cats:
	for msg in cat:
		buff.write(msg)

buff.finished = True

cv2.namedWindow("Nextflix")	

#------------------------------------------------------------------------------------
frameList = []


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
	
	frameList.append(image)
#------------------------------------------------------------------------------------

#while True:
while frameList:
	'''
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
	'''
	cv2.imshow("Nextflix", frameList.pop(0))
	#wait until the next frame
	
	if cv2.waitKey(int(1000/frameRate)) != -1:
		#if the user pressed a key, quit
		buff.quit = True
		break

cv2.destroyWindow("Nextflix")
