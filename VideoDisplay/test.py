import sys
sys.path.insert(0,'../VideoPreProcessor')
import preProcess as pp

sys.path.insert(0,'../VideoUtils')
import dataSet as ds

import videoBuff as vb

import cv2

inputFile = '../assets/sample2.mp4'
saveFile = '../assets/sample2.p'
frameRate = 60.0

success,Cats = ds.load(saveFile)

if not success:
	Cats = pp.preProcess(inputFile,5,[16,8,4,2,1],[0,0,0,0,0],[1,2,4,8,16],2,1)
	#ds.dump(saveFile,Cats)

buff = vb.Buff(10000)

for cat in Cats:
	for msg in cat:
		buff.write(msg)

buff.finished = True

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
	#if cv2.waitKey(int(1000/frameRate)) != -1:
	if cv2.waitKey(27) != -1:
		#if the user pressed a key, quit
		buff.quit = True
		break

cv2.destroyWindow("Nextflix")
