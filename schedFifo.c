#include "scheduler.h"
#include "eventLoop.h"
#include <assert.h>
#include <stdio.h>
#define MAXREQ 1024

/* Statistics
-----------------------------------------------------------------------------*/
int nSchedReq=0; // Number of requests
int maxQsize=0; // Maximum number of entries in the queue
float tQtime=0.0; // total time in the queue
float mQtime=0.0; // Maximum queue wait time

struct request_struct {
	int track;
	int blocks;
	float queued;
} requests[MAXREQ];

int ih=0;
int it=0;
int running=0;

int queSize() {
	if (ih >= it) return (ih - it);
	return ih + (MAXREQ  - it);
}

void schedRequest(float startTime,int track,int blocks) {
	nSchedReq++;
	int newh=ih+1;
	if (newh>=MAXREQ) newh=0;
	assert(newh!=it);
	requests[ih].track=track;
	requests[ih].blocks=blocks;
	requests[ih].queued=startTime;
	ih=newh;
	if (queSize() > maxQsize) maxQsize=queSize();
	if (running==0) startRequest(startTime);
}

void startRequest(float startTime) {
	running=0;
	if (it==ih) return; // Nothing on the que... nothing to schedule
	int newt = it+1;
	if (newt>=MAXREQ) newt=0;
	addEvent(startTime+0.1,DISKIO,requests[it].track,requests[it].blocks);
	float queTime=(startTime+0.1)-requests[it].queued;
	tQtime+=queTime;
	if (queTime > mQtime) mQtime=queTime;
	it=newt;
	running=1;
}

void scheduler_stats() {
	printf("\nFIFO Scheduler Statistics for %d requests\n",nSchedReq);
	printf("  Average Queue Time: %6.2fms\n",tQtime/nSchedReq);
	printf("  Maximum Queue Time: %6.2fms\n",mQtime);
	printf("  Maximum Queue size = %d\n",maxQsize);
	printf("  Unprocessed Queue entries = %d\n",queSize());
}