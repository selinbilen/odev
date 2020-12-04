#include "OS_IOmgr.h"
#include "eventLoop.h"
#include "diskDrive.h"
#include <stdlib.h>
#include <stdio.h>

int prevTrack=0;
int nReq=0;
int nBlks=0;

void newRequest(float startTime,int track,int blocks) {
	// First, pass this request on to the scheduler
	addEvent(startTime+0.1,REQIO,track,blocks);
	nReq++;
	nBlks+=blocks;

	// Second, schedule another user request
	float interval=1.0+(rand()%2500)/10.0; // Next request is in the next 250 ms
	if ( (rand()%100) > 50) prevTrack=rand()%TRACKS;
	int nb=1+rand()%20;
	addEvent(startTime+interval,NEWIO,prevTrack,nb);
}

void IOmgr_stats() {
	printf("\nIO Manager: %d IO requests average %5.2f blocks/request\n",
		nReq,((float) nBlks)/nReq);
}