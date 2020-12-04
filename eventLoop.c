#include "eventLoop.h"
#include "OS_IOmgr.h"
#include "scheduler.h"
#include "diskDrive.h"
#include <stdio.h>
#include <stdlib.h>

struct event_struct {
	float startTime;
	enum eventTypes_enum event;
	int arg1;
	int arg2;
	struct event_struct * prev;
	struct event_struct * next;
};

typedef struct event_struct * eventData;
eventData queHead;

eventData createEvent(float startTime,enum eventTypes_enum event,int arg1,int arg2) {
	eventData ev = (eventData)malloc(sizeof(struct event_struct));
	ev->startTime = startTime;
	ev->event = event;
	ev->arg1 = arg1;
	ev->arg2 = arg2;
	ev->prev=NULL;
	ev->next=NULL;
	return ev;
}

void addEvent(float startTime,enum eventTypes_enum event,int arg1, int arg2) {
	eventData ev = createEvent(startTime,event,arg1,arg2);
	// Insert into QUE based on start time
	if (queHead==NULL) {
		queHead = ev;
		return;
	}
	for(eventData qv = queHead;qv!=NULL; qv=qv->next) {
		if (qv->startTime > ev->startTime) { // Insert ev before qv
			if (qv==queHead) queHead=ev;
			ev->prev = qv->prev;
			if (ev->prev != NULL) ev->prev->next = ev;
			ev->next=qv;
			qv->prev=ev;
			break;
		}
		if (qv->next==NULL) { // at tail, insert after
			ev->prev=qv;
			qv->next=ev;
			break;
		}
	}
}

void runEventLoop(float endTime) {
	float currentTime=0.0;
	addEvent(endTime,FINISH_LOOP,0,0);
	while(queHead != NULL) {
		// Pop an event
		eventData ev = queHead;
		queHead = queHead->next;
		if (queHead!=NULL) queHead->prev=NULL;
		currentTime = ev->startTime;
		if (ev->event==FINISH_LOOP) {
			free(ev);
			break;
		}
		switch(ev->event) {
			case NEWIO:
				newRequest(currentTime,ev->arg1,ev->arg2);
				break;
			case REQIO:
				schedRequest(currentTime,ev->arg1,ev->arg2);
				break;
			case DISKIO:
				diskIO(currentTime,ev->arg1,ev->arg2);
				break;
			case DRIVE_FREE:
				startRequest(currentTime);
				break;
			case FINISH_LOOP:
				// Should never get here
				break;
		}
		free(ev);
	}
	int nq=0;
	while(queHead!=NULL) {
		eventData ev = queHead;
		// printf("Unprocessed event at time %8.2f\n",ev->startTime);
		nq++;
		queHead=ev->next;
		free(ev);
	}
	printf("Total of %d unprocessed events on queue when the exit occured\n",nq);
}

