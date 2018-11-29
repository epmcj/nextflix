/**
 * ONLY FOR TESTS !!! 
 **/
#include "video.h"

/** VIDEO IS COMPOSED BY 2 CATEGORIES:
 *  C1 = {2 msgs of size 2}
 *  C2 = {3 msgs of size 1}
 * 
 **/
#define NUM_CAT 2


/**
 * Reads and stores the video metadata in the file fp.
 * Returns 1 in case of error and 0 otherwise.
 */
int get_video_metadata(FILE* fp, video_metadata_t* vmd) {

    vmd->frame_height = 860;
    vmd->frame_width  = 940;

    vmd->n_cat = NUM_CAT;
    vmd->cat = (cat_metadata_t *) malloc(NUM_CAT * sizeof(cat_metadata_t));
    if (vmd->cat == NULL) {
        printf("FAILED TO GET MEATADATA.\n");
        return 1;
    }

    vmd->cat[0].n_msgs   = 2;
    vmd->cat[0].msg_size = 2;
    vmd->cat[1].n_msgs   = 3;
    vmd->cat[1].msg_size = 1;

    return 0;
}

/**
 * Loads the next video segment into the buffer.
 * Returns 1 in case of error and 0 otherwise.
 */
int load_segment(FILE* fp, segment_t* buffer) {
    int i, j;
    channel_t def_c;

    def_c.r_val =  3.0;
    def_c.i_val = -3.0;
    for (i = 0; i < MAX_HEIGHT; i++) {
        def_c.r_vec_h[i] = (double) i;
        def_c.i_vec_h[i] = (double) i;
    }
    for (i = 0; i < MAX_WIDTH; i++) {
        def_c.r_vec_w[i] = (double) i;
        def_c.i_vec_w[i] = (double) i;
    }

    if (fp == NULL) {
        printf("FINISHED.\n");
        return 1;
    }

    buffer->n_cat = NUM_CAT;
    buffer->cats  = (category_t *) malloc(buffer->n_cat * sizeof(category_t));
    if (buffer->cats == NULL) {
        printf("FAILED TO LOAD.\n");
        return 1;
    }

    buffer->cats[0].msg_size = 2;
    buffer->cats[0].n_msgs   = 2;
    buffer->cats[0].msgs = (message_t *) malloc(buffer->cats[0].n_msgs * 
                                                sizeof(message_t));
    if (buffer->cats[0].msgs == NULL) {
        printf("FAILED TO LOAD.\n");
        return 1;
    }
    // msgs from C1  
    for (i = 0; i < buffer->cats[0].n_msgs; i++) {
        buffer->cats[0].msgs[i].n_data = 2;
        for (j = 0; j < buffer->cats[0].msgs[i].n_data; j++) {
            buffer->cats[0].msgs[i].data[j].frame = i;
            buffer->cats[0].msgs[i].data[j].c[0] = def_c;
            buffer->cats[0].msgs[i].data[j].c[1] = def_c;
            buffer->cats[0].msgs[i].data[j].c[2] = def_c;
        }
    }


    buffer->cats[1].msg_size = 1;
    buffer->cats[1].n_msgs   = 3;
    buffer->cats[1].msgs = (message_t *) malloc(buffer->cats[1].n_msgs * 
                                                sizeof(message_t));
    if (buffer->cats[1].msgs == NULL) {
        printf("FAILED TO LOAD.\n");
        return 1;
    }
    // msgs from C2
    for (i = 0; i < buffer->cats[1].n_msgs; i++) {
        buffer->cats[1].msgs[i].n_data = 2;
        for (j = 0; j < buffer->cats[1].msgs[i].n_data; j++) {
            buffer->cats[1].msgs[i].data[j].frame = i;
            buffer->cats[1].msgs[i].data[j].c[0] = def_c;
            buffer->cats[1].msgs[i].data[j].c[1] = def_c;
            buffer->cats[1].msgs[i].data[j].c[2] = def_c;
        }
    } 

    return 0;
}