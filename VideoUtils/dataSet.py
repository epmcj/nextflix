import os.path as p
import structures as st
import numpy as np

#https://wiki.python.org/moin/UsingPickle

def dump(filename,Cats):
	pickle.dump(Cats, open(filename, "wb"), pickle.HIGHEST_PROTOCOL)

def load(filename):
	#if the file exists
	if p.isfile(filename):
		#load the structure
		return True,pickle.load(open(filename, "rb"))
	else:
		return False,None

#pLen = length of each P column
#qLen = length of each Q line
#nElements = number of ChannelElements of each channel
def dataFromFloatArray(floatArray,pLen,qLen,nElements,nChannels,frameNum):
	first = 0
	#total length of a channelElement
	tLen = pLen+qLen+1
	#create an empty list of channels
	Channels = [None] * nChannels
	for chn in range(nChannels):
		#create an empty list of channel elements
		ceList = [None]*nElements
		for el in range(nElements):
			P_column = np.array(floatArray[first:first+pLen])
			D_value = floatArray[first+pLen]
			Q_line = np.array(floatArray[first+pLen+1:first+pLen+qLen+1])
			ceList[el] = st.ChannelElement(P_column, D_value, Q_line)
			first = first + tLen
		#insert the channel
		Channels[chn]=st.Channel(ceList)
	return st.Data(Channels,frameNum)
