#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <stdio.h>
#include <stdlib.h>

#define MAX_DATA  10
#define MAX_HEIGHT 5
#define MAX_WIDTH  5

typedef struct {
    int   n_msgs;
    int msg_size;
} cat_metadata_t;

typedef struct {
    int    frame_height;
    int     frame_width;
    int           n_cat;
    cat_metadata_t* cat;
} video_metadata_t;

typedef struct {
    double               r_val;
    double               i_val;
    double r_vec_h[MAX_HEIGHT];
    double i_vec_h[MAX_HEIGHT];
    double  r_vec_w[MAX_WIDTH];
    double  i_vec_w[MAX_WIDTH];
} channel_t;

typedef struct {
    channel_t c[3];
    int      frame;
} data_t;

typedef struct {
    data_t data[MAX_DATA];
    int            n_data;
} message_t;

typedef struct {
    message_t* msgs;
    int      n_msgs;
    int    msg_size;
} category_t;

typedef struct {
    category_t* cats;
    int        n_cat;
} segment_t;



/**
 * Reads and stores the video metadata in the file fp.
 * Returns 1 in case of error and 0 otherwise.
 */
int get_video_metadata(FILE* fp, video_metadata_t* vmd);

/**
 * Loads the next video segment into the buffer.
 * Returns 1 in case of error and 0 otherwise.
 */
int load_segment(FILE* fp, segment_t* buffer);

#endif