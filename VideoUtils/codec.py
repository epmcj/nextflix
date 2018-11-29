import numpy as np
import structures as st

#return a data object containing an entire frame
def decomposeFrame(frame,frameNum):
	channelList = []
	for i in range(frame.shape[2]):
		channelList.append(decomposeMatrix(frame[:,:,i]))
	return(st.Data(channelList, frameNum))

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
		frame = np.zeros((height,width,len(data.channel)), np.uint8)
	
		#recompose each channel
		for i in range(len(data.channel)):
			frame[:,:,i] = np.uint8(composeMatrix(data.channel[i]));
		
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
	m = np.matmul(np.matmul(P, np.diag(D)), Q)
	
	return(m)

