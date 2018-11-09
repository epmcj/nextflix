#TODO!!!!!>>> GARANTIR QUE OS DADOS MAIS SIGNIFICATIVOS SEMPRE ESTEJAM REDUNDANTES!!!!!!!!!!!!!!
import cv2
import numpy as np
import random
import structures as st
import codec as cod

def main():
	image = cv2.imread("amsterdam.jpg")
	
	data = cod.decomposeFrame(image,0)
	
	#only for tests:
	#data.shuffleData()
	#data.loseData(100)
	
	ncat = 5 #number of categories
	n_msgs = [16,8,4,2,1] #number of messages for each category
	msg_redundancies = [0,0.05,0.1,0.15,0.2] #redundancy in each category
	msg_sizes_base = [1,2,4,8,16] #proportions of the sizes of the categories
	msg_sizes = np.ceil(float(data.getNumberOfElements()\
		/np.matmul(np.array(msg_sizes_base,dtype=float),\
			np.matmul(np.diag(1-np.array(msg_redundancies,dtype=float)),\
				np.array(n_msgs,dtype=float).T)))\
		*np.array(msg_sizes_base,dtype=float))
	
	cats = data.createMsgs(ncat,n_msgs,np.array(msg_sizes,dtype=int),msg_redundancies)
	
	data2 = st.Data([],[],[],0)
	for cat in cats:
		for msg in cat:
			if random.uniform(0, 1)<0.9:#10% of msgs lost
				data2.insertData(msg)
	
	success, image2 = cod.composeFrame(data2)
	
	if(not success):
		print('Error')
	else:
		cv2.namedWindow("original")
		cv2.namedWindow("comprimida")
	
		cv2.imshow("original",image)
		cv2.imshow("comprimida",image2)
	
		cv2.waitKey(0)
	
		cv2.destroyWindow("original")
		cv2.destroyWindow("comprimida")

main()
