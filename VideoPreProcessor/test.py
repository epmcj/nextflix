import cv2
import numpy as np
import random
import structures as st
import codec as cod

def main():
	image = cv2.imread("../assets/amsterdam.jpg")
	#image = cv2.imread("../assets/tiny.png")
	
	data = cod.decomposeFrame(image,0)
	
	#only for tests:
	#data.shuffleData()
	#data.loseData(100)
	
	ncat = 5 #number of categories
	n_msgs = [16,8,4,2,1] #number of messages for each category
	#msg_redundancies = [0,0.05,0.1,0.15,0.2] #redundancy in each category
	msg_redundancies = [0,0,0,0,0] #redundancy in each category
	msg_sizes_base = [1,2,4,8,16] #proportions of the sizes of the categories
	
	msg_sizes = data.generateNiceMsg_Sizes(ncat, n_msgs, msg_redundancies, msg_sizes_base)
	
	expoent = 10
	fixed = float('inf')
	
	cats = data.createMsgs(ncat,n_msgs,msg_sizes,msg_redundancies,expoent,fixed)
	
	d_msg = 0
	t_msg = 0
	data2 = st.Data(st.Channel([]),st.Channel([]),st.Channel([]),0)
	for cat in cats:
		for msg in cat:
			if random.uniform(0, 1)<=0.9:#10% of msgs lost
				data2.insertData(msg)
			else:
				d_msg = d_msg+1
			t_msg = t_msg+1
				
	print('Dropped msg: '+str(d_msg)+' of '+str(t_msg))
	
	success, image1 = cod.composeFrame(data)
	success, image2 = cod.composeFrame(data2)
	
	if(not success):
		print('Error')
		exit(0)
	else:
		cv2.namedWindow("original")
		cv2.namedWindow("comprimida")
	
		cv2.imshow("original",image1)
		cv2.imshow("comprimida",image2)
	
		cv2.waitKey(0)
	
		cv2.destroyWindow("original")
		cv2.destroyWindow("comprimida")

main()
