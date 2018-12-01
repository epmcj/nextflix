#include <stdio.h>
#include "video.h"

//gcc videoTester.c video.c
// ./a.out

int main(){
	int i;
	int maxNMsgs = 5;//number of messages per frame for cat1
	int numFrames = 65;//for the chosed video
	
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
	destroy_metadata(meta);
	
	fclose(fp);
	
	return 0;
}
