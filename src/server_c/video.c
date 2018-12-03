#include "video.h"

/**
 * Creates a metadata_t struct with all scalar fields equal to zero
 * maxNMsgs is the number of messages of the category (per frame)
 * numFrames is the total number of frames of the video
 */
metadata_t* create_metadata(uint32_t id, int maxNMsgs, int numFrames){

	metadata_t* meta = (metadata_t*) malloc(sizeof(metadata_t));
	
	meta->cat_id       = id;
	meta->frame_height = 0;
    meta->frame_width  = 0;
    meta->nChannels    = 0;
    meta->nObjects 	   = 0;
    
    meta->nElements = (uint8_t*) malloc(numFrames * maxNMsgs * sizeof(uint8_t));
    meta->frameNums = (uint8_t*) malloc(numFrames * maxNMsgs * sizeof(uint8_t));
    // meta->nElements = (int*) malloc(numFrames * maxNMsgs * sizeof(int));
    // meta->frameNums = (int*) malloc(numFrames * maxNMsgs * sizeof(int));
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
msg_set_t* create_message_set(metadata_t* meta, int maxNumMsg) {
	
	int i, maxNElements = 0;
	
	msg_set_t* cat = (msg_set_t*) malloc(sizeof(msg_set_t));
	
	cat->n_msgs = 0;
	cat->msgs = (message_t*) malloc(maxNumMsg*sizeof(message_t));
	
	for (i = 0; i < meta->nObjects; i++)
		if(maxNElements < meta->nElements[i])
			maxNElements = meta->nElements[i];
	
	int maxMsgSize = meta->nChannels
		*(meta->frame_height+meta->frame_width+1)
		*(maxNElements);
	
	for (i=0; i < maxNumMsg; i++) {
		cat->msgs[i].size = 0;
		cat->msgs[i].data = (float*) calloc(maxMsgSize, sizeof(float));
	}
	
	return cat;
}

void destroy_message_set(msg_set_t* cat, int maxNumMsg){

	int i;
	for (i=0; i < maxNumMsg; i++) {
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
		// meta->frameNums[i] = (int) x;
		meta->frameNums[i] = (uint8_t) x;
		//printf("%f\n",x);
		
		fscanf(fp,"%f\n", &x);
		// meta->nElements[i] = (int) x;
		meta->nElements[i] = (uint8_t) x;
	}
	
    return 0;
}

/**
 * Loads the next sequence of data objects into the buffer.
 * Returns 1 in case of error and 0 otherwise.
 */
int load_msg_set(FILE* fp, msg_set_t* buffer, metadata_t* meta, int next,
	int numMsg, int categoryId) {
	
	int i, j, final, objSize;
	float x;
	
	if (next >= meta->nObjects) {
		//nothing to read
		return next;
	}
	
	//set starts empty
	buffer->n_msgs = 0;
	
	final = next + numMsg;
	
	while ((next < meta->nObjects) && (next < final)){
		//size of this single object in floats
		objSize = meta->nElements[next]*meta->nChannels
			*(meta->frame_height + meta->frame_width + 1);
		//read the next [objSize] floats and place them
		for (j = 0; j < objSize; j++){
			fscanf(fp,"%f\n", buffer->msgs[buffer->n_msgs].data + j);
		}
		buffer->msgs[buffer->n_msgs].size = objSize;
		buffer->msgs[buffer->n_msgs].index = next;
		buffer->msgs[buffer->n_msgs].categoryId = categoryId;
		//updating pointers..
		next++;
		buffer->n_msgs++;
	}
	
    return next;
}


int create_and_load_metadata(FILE** files, metadata_t** vmdata, int ncat, 
							 int *maxMsgsPerCat, int numFrames) {
	int i;
	FILE *fp;
	char fname[512]; // magic number

	for (i = 0; i < ncat; i++) {
		vmdata[i] = create_metadata(i, maxMsgsPerCat[i], numFrames);
		if (get_file_metadata(files[i], vmdata[i]) == 1) {
			return 1;
		}
	}

	return 0;
}

int load_next_segment(FILE** fps, metadata_t** vmdata, int *msgsCat, 
					  segment_t* seg) {
	int i, next;

	next =  + 1;
	for (i = 0; i < seg->n_cat; i++) {
		// printf("seg[%d][%d]\n", i, seg->next[i]);
		next = load_msg_set(fps[i], seg->sets[i], vmdata[i], seg->next[i], 
							msgsCat[i], i);
		if(seg->next[i] == next) {
			printf("Failed to load msg set %d.\n", i);
			return 1;
		}
		seg->next[i] = next;
	}

	return 0;
}
