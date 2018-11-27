import numpy as np
import structures as st

#return a data object containing an entire frame
def decomposeFrame(frame,frameNum):
	red = frame[:,:,2]
	green = frame[:,:,1]
	blue = frame[:,:,0]
	return(st.Data(\
		decomposeMatrix(red),\
		decomposeMatrix(green),\
		decomposeMatrix(blue),\
		frameNum))

#apply svd decomposition to a single channel of an image.
def decomposeMatrix(mat):
	P, D, Q = np.linalg.svd(np.matrix(mat,dtype=float), full_matrices=False)
	ceList = []
	for i in range(len(D)):
		ceList.append(st.ChannelElement(P[:,i],D[i],Q[i,:]))
	return(st.Channel(ceList))

#recomposes the frame after the transformation into transferable data	
def composeFrame(data):
	if(data.isEmpty()):
		return False,None
	else:
		#get the dimensions
		height, width = data.dim()
		#create blank image
		frame = np.zeros((height,width,3), np.uint8)
	
		#recompose each channel
		red = composeMatrix(data.channel[2]);
		green = composeMatrix(data.channel[1]);
		blue = composeMatrix(data.channel[0]);
	
		#compose the entire frame
		frame[:,:,2] = np.uint8(red)
		frame[:,:,1] = np.uint8(green)
		frame[:,:,0] = np.uint8(blue)
	
		return True,frame

#recompose a simple 1-channel image (double)
def composeMatrix(channel):
	#get the dimensions
	height, width = channel.dim()
	
	#the matrices for svd
	P = np.zeros((height,len(channel)));
	D = np.zeros(len(channel));
	Q = np.zeros((len(channel),width));
	
	#fulfill the matrices
	for i in range(len(channel)):
		P[:,i] = channel.list[i].P_column.flatten()
		D[i] = channel.list[i].D_value
		Q[i,:] = channel.list[i].Q_line.flatten()
	
	#wayback from svd
	return(np.matmul(np.matmul(P, np.diag(D)), Q))
