#ifndef EVENTLOOP_H
enum eventTypes_enum {
	NEWIO, // Specify that a new IO request has come from the OS I/O manager
	REQIO,  // Request an IO transfer from OS I/O manager to scheduler
	DISKIO, // Scheduler request disk IO from the device
	DRIVE_FREE, // Device tells the scheduler the drive is free
	FINISH_LOOP // End the event loop
};

void addEvent(float startTime,enum eventTypes_enum event,int arg1, int arg2);
void runEventLoop(float endTime);

#define EVENTLOOP_H
#endif