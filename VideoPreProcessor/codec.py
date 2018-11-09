import numpy as np
import structures as st

#return a data object containing an entire frame
def decomposeFrame(frame,frameNum):
	red = frame[:,:,2]
	green = frame[:,:,1]
	blue = frame[:,:,0]
	return(st.Data(decomposeMatrix(red),decomposeMatrix(green),\
		decomposeMatrix(blue),frameNum))

#apply svd decomposition to a single channel of an image.
def decomposeMatrix(mat):
	P, D, Q = np.linalg.svd(np.matrix(mat,dtype=float), full_matrices=False)
	ret = []
	for i in range(len(D)):
		ret.append(st.Channel(P[:,i],D[i],Q[i,:]))
	return(ret)

#recomposes the frame after the transformation into transferable data	
def composeFrame(data):
	if(data.isEmpty()):
		return False,None
	else:
		height = len(data.channel[0][0].P_column)
		width = len(data.channel[0][0].Q_line.T)
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
def composeMatrix(channelList):
	P = np.zeros((len(channelList[0].P_column),len(channelList)));
	D = np.zeros(len(channelList));
	Q = np.zeros((len(channelList),len(channelList[0].Q_line.T)));
	
	for i in range(len(channelList)):
		P[:,i] = channelList[i].P_column.flatten()
		D[i] = channelList[i].D_value
		Q[i,:] = channelList[i].Q_line.flatten()
	
	return(np.matmul(np.matmul(P, np.diag(D)), Q))
