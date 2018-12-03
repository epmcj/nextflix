#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// #define MAX_DATA  10
// #define MAX_HEIGHT 5
// #define MAX_WIDTH  5

//typedef struct {
//    int   n_msgs;
//    int msg_size;
//} cat_metadata_t;

#define NUM_CATS     1
#define NUM_FRAMES   145
#define MSGS_PER_CAT {180,90}
// #define MSGS_PER_CAT {5,4,3,2,1}

typedef struct {
    uint32_t  cat_id; 
    uint32_t  frame_height;
    uint32_t  frame_width;
    uint32_t  nChannels;
    uint32_t  nObjects;//number of data objects written in the file
    uint8_t* nElements;//number of elements of each data object in the file
    uint8_t* frameNums;//frame to which each data object belongs
    //int           n_cat;
    //cat_metadata_t* cat;
//} video_metadata_t;
} metadata_t;

typedef struct {
    uint32_t size;
	uint32_t categoryId;//a pedido do Eduardo
	uint32_t index;//index of this message inside the file
    float   *data;
} message_t;

typedef struct {
    message_t* msgs;
    int      n_msgs;
    //int    msg_size;
//} category_t;
}msg_set_t;

typedef struct {
   msg_set_t**  sets;
   int         n_cat;
   int*         next;
} segment_t;


/**
 * Creates a metadata_t struct with all scalar fields equal to zero
 * maxNMsgs is the number of messages of the category (per frame)
 * numFrames is the total number of frames of the video
 */
metadata_t* create_metadata(uint32_t id, int maxNMsgs, int numFrames);
void destroy_metadata(metadata_t* meta);

/**
 * Creates a superscaled set of messages
 */
msg_set_t* create_message_set(metadata_t* meta, int maxNumMsg);
void destroy_message_set(msg_set_t* cat, int maxNumMsg);

/**
 * Reads and stores the video metadata in the file fp.
 * Returns 1 in case of error and 0 otherwise.
 */
int get_file_metadata(FILE* fp, metadata_t* meta);


/**
 * Loads the next video segment into the buffer.
 * Returns 1 in case of error and 0 otherwise.
 * Obtain meta using get_file_metadata and initialize
 * next with 0
 */
int load_msg_set(FILE* fp, msg_set_t* buffer, metadata_t* meta, int next,
	int numMsg, int categoryId);

/**
 * 
 **/
int create_and_load_metadata(FILE** files, metadata_t** vmdata, int ncat, 
                             int *maxMsgsPerCat, int numFrames);

/**
 * 
 **/
int load_next_segment(FILE** files, metadata_t** vmdata, int *msgsCat, 
                      segment_t* seg);

#endif
