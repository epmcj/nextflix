import os.path
import structures as st
import numpy as np

import itertools
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

#load the header
def loadHeader(filename):
	#check if file exists
	if not os.path.isfile(filename):
		return False, None
	
	input_file = open(filename, 'r')
	
	#get header
	pLen = int(float(input_file.next()))
	qLen = int(float(input_file.next()))
	nChannels = int(float(input_file.next()))
	nDataObj = int(float(input_file.next()))
	
	#framenums of each data object
	frameNums = []
	#nElements of each data object
	nElements = []
	
	for i in range(nDataObj):
		#get new specific metadata
		frameNums.append(int(float(input_file.next())))
		nElements.append(int(float(input_file.next())))
	
	return True,st.Metadata(pLen,qLen,nChannels,frameNums,nElements)

#load a single category from a file. Expects the metadata, the next index and the number of data
#objects to be loaded. Returns a list of each list of floats
def loadSegmentAsFloats(filename,meta,nextIndex,numObjects):
	#check if file exists
	if not os.path.isfile(filename):
		return []
	
	#load the whole file
	input_file = open(filename, 'r')
	
	#last object index that can be loaded
	lastIndex = min(nextIndex+numObjects,len(meta.frameNums))
	
	#nothing to load
	if lastIndex<= nextIndex:
		return []
	
	#data as lists of floats
	floatArrays = []
	
	#process the first object
	firstValue = 4+2*len(meta.nElements)+sum(meta.nElements[:nextIndex])*meta.nChannels\
		*(meta.width+meta.height+1)
	objSize = (meta.height+meta.width+1)*meta.nElements[nextIndex]*meta.nChannels	
	floatArray = []
	for row in itertools.islice(input_file, firstValue, firstValue+objSize):
		floatArray.append(float(row))
	
	floatArrays.append(floatArray)
	
	#process the rest of the objects
	for obj in range(nextIndex+1,lastIndex):
		#the size of this object
		objSize = (meta.height+meta.width+1)*meta.nElements[obj]*meta.nChannels
		
		floatArray = []
		#islice considers the next value of the file as index 0
		for row in itertools.islice(input_file, 0, objSize):
			floatArray.append(float(row))
		
		floatArrays.append(floatArray)
	
	return floatArrays

#load the entire file into a metadata object and a data list
def load(filename):
	success, meta = loadHeader(filename)
	if not success:
		return meta,[]
	#number of data elements in the file
	num = len(meta.nElements)
	dataList = []
	first = 0
	while first<=num:
		#load next 100 elements
		fArrayList = loadSegmentAsFloats(filename,meta,first,100)
		#transform each one into a data object
		for i in range(len(fArrayList)):
			
			if len(fArrayList[i])>0:
				data = dataFromFloatArray(fArrayList[i],meta,first+i)
				dataList.append(data)
		first = first+100
	return meta, dataList

#write a float array (list) in the disk
def dumpArray(output_file, floatArray):
	writer = csv.writer(output_file,delimiter='\n')
	writer.writerow(floatArray)

def dataFromFloatArray(floatArray,meta,objIndex):
	#(metadata)
	pLen = meta.height
	qLen = meta.width
	nElements = meta.nElements[objIndex]
	nChannels = meta.nChannels
	frameNum = meta.frameNums[objIndex]
	
	#empty list, empty object
	if not floatArray:
		return st.Data([],frameNum)
	
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








