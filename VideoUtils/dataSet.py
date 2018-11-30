import os.path
import structures as st
import numpy as np

#from array import array
import csv

def genFileName(fileName_base,catIndex):
	return fileName_base+'_'+str(catIndex)+'.nextflix'

#write a category list into ncat files
def dump(fileName_base,cats):
	#extracting some metadata
	nChannels = len(cats[0][0].channel)
	pLen, qLen = cats[0][0].channel[0].dim()
	for cat in range(len(cats)):
		filename = genFileName(fileName_base,cat)
		output_file = open(filename, 'a')
		print('Saving cat '+str(cat)+' into '+filename)
		
		#the header follows the structure
		#[pLen|qLen|nChannels|#objects|frameNumobj1|nElementsobj1|...|frameNumobjn|nElementsobjn]
		#the first 3 fields are redundant considering all files, but it gives them some independence
		header = [float(pLen),float(qLen),float(nChannels),float(len(cats[cat]))]
		for data in cats[cat]:
			header = header+[float(data.frame),float(len(data.channel[0]))]
		dumpArray(output_file, header)
		
		for data in cats[cat]:
			dumpArray(output_file, floatArrayFromData(data))
		
		output_file.close()

#load a single category from a file
def load(filename):
	#check if file exists
	if not os.path.isfile(filename):
		return False,None
	
	#load the whole file
	floatArray = loadArray(filename)
	
	#get header
	pLen = int(floatArray[0])
	qLen = int(floatArray[1])
	nChannels = int(floatArray[2])
	nDataObj = int(floatArray[3])
	
	#data for this category
	dataList = []
	
	#first position for the rest of the header
	inicioH = 4
	
	#first position with real image data
	inicio = 4+2*nDataObj
	for i in range(nDataObj):
		#get new specific metadata
		frameNum = int(floatArray[inicioH])
		nElements = int(floatArray[inicioH+1])
		inicioH = inicioH+1
		
		#get real image data and compose the object
		dataList.append(dataFromFloatArray(\
			floatArray[inicio:inicio+(pLen+qLen+1)*nElements*nChannels],\
			pLen,qLen,nElements,nChannels,frameNum))
		inicio = inicio + (pLen+qLen+1)*nElements*nChannels
	
	return True,dataList

#write a float array (list) in the disk
def dumpArray(output_file, floatArray):
	#float_array = array('f', floatArray)
	#float_array.tofile(output_file)
	
	writer = csv.writer(output_file,delimiter='\n')
	writer.writerow(floatArray)

#load a float array from the disk
def loadArray(filename):
	input_file = open(filename, 'r')
	#float_array = array('f')
	#float_array.fromstring(input_file.read())
	float_array = []
	for y in input_file.read().split('\n'):
		if y.isdigit():
			float_array.append(float(y))
	return float_array

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

def floatArrayFromData(data):
	if data.isEmpty():
		print('I cannot get a float array from an empty object')
		exit(0)
	
	#extracting some metadata
	pLen, qLen = data.channel[0].dim()
	
	#allocating statically the array
	floatArray = [0.0]*((pLen+qLen+1)*data.totalSize())
	
	#fill the array
	inicio = 0
	for channel in data.channel:
		for cElement in channel.list:
			floatArray[inicio:inicio+pLen] = cElement.P_column.T.tolist()[0]
			floatArray[inicio+pLen] = float(cElement.D_value)
			floatArray[inicio+pLen+1:inicio+pLen+qLen+1] = cElement.Q_line.tolist()[0]
			inicio = inicio + pLen + qLen + 1
	
	return floatArray








