import os.path as p
import cPickle as pickle

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
