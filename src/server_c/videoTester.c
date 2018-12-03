#include <stdio.h>
#include "video.h"

//gcc videoTester.c video.c
// ./a.out

int main(){
	int i, old;
	int maxNMsgs = 180;//number of messages per frame for cat1
	int numFrames = 145;//for the chosed video
	int next = 0;//next object to be loaded
	int maxNumMsgs = 50;//amount of messages to be loaded at once
	
	FILE* fp = fopen("../../assets/chaplinCut.mp4_0.nextflix","r");
	
	metadata_t* meta = create_metadata(0, maxNMsgs, numFrames);
	
	get_file_metadata(fp, meta);
	printf("Height: %d\n", meta->frame_height);
	printf("Width: %d\n", meta->frame_width);
	printf("nChannels: %d\n", meta->nChannels);
	printf("nObj: %d\n", meta->nObjects);
	
	for(i=0; i < meta->nObjects; i++){
		printf("i: %d/%d, ", i, meta->nObjects-1);
		printf("nFrames: %d, ", meta->frameNums[i]);
		printf("nElements: %d\n", meta->nElements[i]);
	}
	
	msg_set_t* buffer = create_message_set(meta, maxNumMsgs);
	
	//read the rest of the file (using the same buffer structure)
	old = 0;
	while (1){
		//load the next maxNumMsgs messages. maxNumMsgs is the upper limit
		//only for this buffer instancy.
		next = load_msg_set(fp, buffer, meta, next, maxNumMsgs, 0);
		if(next==old){
			//if the index did not increase, end
			break;
		}else{
			printf("Loaded %d messages\n", buffer->n_msgs);
			for (i=0; i < buffer->n_msgs; i++){
				printf("Msg id %d com %d floats\n", buffer->msgs[i].index,
					buffer->msgs[i].size);
			}
			old = next;
		}
	}
	destroy_metadata(meta);
	destroy_message_set(buffer, maxNumMsgs);
	
	fclose(fp);
	
	return 0;
}
