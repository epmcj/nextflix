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

//get 100 msgs from disk each reading
#define MAX_MSG_SET  100
#define NUM_CATS     5
#define MSGS_PER_CAT {5,4,3,2,1}

typedef struct {
    int    frame_height;
    int    frame_width;
    int    nChannels;
    int    nObjects;//number of data objects written in the file
    int*   nElements;//number of elements of each data object in the file
    int*   frameNums;//frame to which each data object belongs
    //int           n_cat;
    //cat_metadata_t* cat;
//} video_metadata_t;
} metadata_t;

typedef struct {
    uint32_t size;
    float   *data;
} message_t;

typedef struct {
    message_t* msgs;
    int      n_msgs;
    //int    msg_size;
//} category_t;
}msg_set_t;

typedef struct {
   msg_set_t*  sets;
   int        n_cat;
} segment_t;


/**
 * Creates a metadata_t struct with all scalar fields equal to zero
 * maxNMsgs is the number of messages of the category (per frame)
 * numFrames is the total number of frames of the video
 */
metadata_t* create_metadata(int maxNMsgs, int numFrames);
void destroy_metadata(metadata_t* meta);

/**
 * Creates a superscaled set of messages
 */
msg_set_t* create_message_set(metadata_t* meta);
void destroy_message_set(msg_set_t* cat);

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
int load_msg_set(FILE* fp, msg_set_t* buffer, metadata_t* meta, int next);

#endif
