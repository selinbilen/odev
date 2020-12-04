#include "scheduler.h"
#include "eventLoop.h"
#include <stdio.h>
#include <stdlib.h>

/* Statistics
-----------------------------------------------------------------------------*/
int nSchedReq=0; // Number of requests
int maxQsize=0; // Maximum number of entries in the queue
int nChngDir=0; // Number of times the direction was changed
float tQtime=0.0; // Total time in the queue
float mQtime=0.0; // Maximum queue wait time

typedef enum {INWARD, OUTWARD} Direction;
typedef struct request_struct {
  float queued;
  int track;
  int blocks;
  struct request_struct *next;
  struct request_struct *prev;
} Req;

Req* qHead = NULL;
Req* qTail = NULL;
int qSize = 0;
int running = 0;
// Define initial direction to inward and initial disk position to track 0 (the outermost)
Direction dir = INWARD;
int diskPos = 0;

void addRequest(float startTime, int track, int blocks) {
  Req* newReq = malloc(sizeof(Req));
  newReq->queued = startTime;
  newReq->track = track;
  newReq->blocks = blocks;
  newReq->next = NULL;
  if (qHead == NULL) {
    newReq->prev = NULL;
    qHead = newReq;
    qTail = newReq;
  }
  else {
    newReq->prev = qTail;
    qTail->next = newReq;
    qTail = newReq;
  }
  qSize++;
}

void removeRequest(Req* request) {
  if (request == qHead && request == qTail) {
    qHead = NULL;
    qTail = NULL;
  }
  else if (request == qHead) {
    qHead = request->next;
    request->next->prev = NULL;
  }
  else if (request == qTail) {
    qTail = request->prev;
    request->prev->next = NULL;
  }
  else {
    request->prev->next = request->next;
    request->next->prev = request->prev;
  }
  free(request);
  qSize--;
}

Req* findClosestInward() {
  Req* request = NULL;
  Req* temp = qHead;
  int diff, smallestDiff;
  while (temp) {
    diff = temp->track - diskPos;
    if (diff >= 0) {
      if ((request == NULL) || (diff < smallestDiff)) {
        smallestDiff = diff;
        request = temp;
      }
    }
    temp = temp->next;
  }
  return request;
}

Req* findClosestOutward() {
  Req* request = NULL;
  Req* temp = qHead;
  int diff, smallestDiff;
  while (temp) {
    diff = diskPos - temp->track;
    if (diff >= 0) {
      if ((request == NULL) || (diff < smallestDiff)) {
        smallestDiff = diff;
        request = temp;
      }
    }
    temp = temp->next;
  }
  return request;
}

Req* scan() {
  Req* request;
  if (dir == INWARD) {
    request = findClosestInward();
    if (request == NULL) {
      dir = OUTWARD;
      request = findClosestOutward();
      nChngDir++;
    }
  }
  else {
    request = findClosestOutward();
    if (request == NULL) {
      dir = INWARD;
      request = findClosestInward();
      nChngDir++;
    }
  }
  diskPos = request->track;
  return request;
}

void schedRequest(float startTime,int track,int blocks) {
  nSchedReq++;
  addRequest(startTime, track, blocks);
  if (qSize > maxQsize) maxQsize = qSize;
  if (running == 0) startRequest(startTime);
}

void startRequest(float startTime) {
  running = 0;
  if (qHead == NULL) return;
  Req* request = scan();
  addEvent(startTime + 0.1, DISKIO, request->track, request->blocks);
  float queTime = (startTime + 0.1) - request->queued;
  tQtime += queTime;
  if (queTime > mQtime) mQtime = queTime;
  removeRequest(request);
  running = 1;
}

void scheduler_stats() {
	printf("\nScan Scheduler Statistics for %d requests\n",nSchedReq);
	printf("  Average Queue Time: %6.2fms\n",tQtime/nSchedReq);
	printf("  Maximum Queue Time: %6.2fms\n",mQtime);
	printf("  Maximum Queue size = %d\n",maxQsize);
	printf("  Unprocessed Queue entries = %d\n",qSize);
	printf("  Number of times the direction changed = %d\n",nChngDir);
}
