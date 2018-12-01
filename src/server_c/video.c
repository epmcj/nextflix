#include "video.h"

/**
 * Creates a metadata_t struct with all scalar fields equal to zero
 * maxNMsgs is the number of messages of the category (per frame)
 * numFrames is the total number of frames of the video
 */
metadata_t* create_metadata(int maxNMsgs, int numFrames){

	metadata_t* meta = (metadata_t*) malloc(sizeof(metadata_t));
	
	meta->frame_height = 0;
    meta->frame_width = 0;
    meta->nObjects = 0;
    meta->nChannels = 0;
    
    meta->nElements = (int*) malloc(numFrames * maxNMsgs * sizeof(int));
    meta->frameNums = (int*) malloc(numFrames * maxNMsgs * sizeof(int));
    return meta;
}

void destroy_metadata(metadata_t* meta) {

	free(meta->nElements);
	free(meta->frameNums);
	free(meta);
}

/**
 * Creates a superscaled set of messages
 */
msg_set_t* create_message_set(metadata_t meta) {
	
	int i;
	
	msg_set_t* cat = (msg_set_t*) malloc(sizeof(msg_set_t));
	
	cat->n_msgs = 0;
	cat->msgs = (message_t*) malloc(MAX_MSG_SET*sizeof(message_t));
	
	int maxMsgSize = meta.nChannels
		*(meta.frame_height+meta.frame_width+1)
		*((meta.frame_height<meta.frame_width)?meta.frame_height:meta.frame_width);
	
	for (i=0; i < MAX_MSG_SET; i++) {
		cat->msgs[i].size = 0;
		cat->msgs[i].data = (float*) calloc(maxMsgSize, sizeof(float));
	}
}

void destroy_message_set(msg_set_t* cat){

	int i;
	for (i=0; i < MAX_MSG_SET; i++) {
		free(cat->msgs[i].data);
	}
	free(cat->msgs);
	free(cat);
}



/**
 * Reads metadata from file fp.
 * Returns 1 in case of error and 0 otherwise.
 */
int get_file_metadata(FILE* fp, metadata_t* meta) {
	
	int i;
	float x;
	
    if (fp == NULL) {
        printf("Invalid file pointer.\n");
        return 1;
    }
	
	//make sure that the file pointer is at the beginning
	rewind(fp);
	
	//Read sequencially the header
	fscanf(fp,"%f\n", &x);
	meta->frame_height = (int) x;
	
	fscanf(fp,"%f\n", &x);
	meta->frame_width = (int) x;
	
	fscanf(fp,"%f\n", &x);
	meta->nChannels = (int) x;
	
	fscanf(fp,"%f\n", &x);
	meta->nObjects = (int) x;
	
	//read the attributes of each specific data object
	for(i=0; i < meta->nObjects; i++){
	
		fscanf(fp,"%f\n", &x);
		meta->frameNums[i] = (int) x;
		//printf("%f\n",x);
		
		fscanf(fp,"%f\n", &x);
		meta->nElements[i] = (int) x;
	}
	
    return 0;
}

void initialize_ctrl_index(int *ctrlIndex, metadata_t meta){
	*ctrlIndex = 4+2*meta.nObjects;
}

/**
 * Loads the next sequence of data objects into the buffer.
 * Returns 1 in case of error and 0 otherwise.
 */
int load_msg_set(FILE* fp, msg_set_t* buffer, metadata_t meta,
	int* ctrlIndex, int* next) {
	
	int i;
	
	if (*next >= meta.nObjects) {
		//nothing to read
		return 1;
	}
	
	buffer->n_msgs = 0;
	
    fseek(fp, *ctrlIndex, SEEK_SET);
	
	
    return 0;
}
