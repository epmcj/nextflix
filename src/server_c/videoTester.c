#include <stdio.h>
#include "video.h"

//gcc videoTester.c video.c
// ./a.out

int main(){
	int i, old;
	int maxNMsgs = 5;//number of messages per frame for cat1
	int numFrames = 65;//for the chosed video
	int next = 0;//next object to be loaded
	
	FILE* fp = fopen("../../assets/sample2.mp4_0.nextflix","r");
	
	metadata_t* meta = create_metadata(maxNMsgs, numFrames);
	
	get_file_metadata(fp, meta);
	printf("Height: %d\n", meta->frame_height);
	printf("Width: %d\n", meta->frame_width);
	printf("nChannels: %d\n", meta->nChannels);
	printf("nObj: %d\n", meta->nObjects);
	
	for(i=0; i < meta->nObjects; i++){
		printf("nFrames: %d\n", meta->frameNums[i]);
		printf("nElements: %d\n", meta->nElements[i]);
		printf("\n");
	}
	
	msg_set_t* buffer = create_message_set(meta);
	
	//read the rest of the file (using the same buffer structure)
	old = 0;
	while (1){
		//load the next MAX_MSG_SET messages
		next = load_msg_set(fp, buffer, meta, next);
		if(next==old){
			//if the index did not increase, end
			break;
		}else{
			printf("Loaded %d messages\n", buffer->n_msgs);
			old = next;
		}
	}
	destroy_metadata(meta);
	destroy_message_set(buffer);
	
	fclose(fp);
	
	return 0;
}
