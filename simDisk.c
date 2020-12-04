#include "eventLoop.h"
#include "OS_IOmgr.h"
#include "scheduler.h"
#include "diskDrive.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {

	// In case of crashes, make sure nothing is buffered on stdout or stderr
	setbuf(stdout,0);
	setbuf(stderr,0);

	// Seed random number generator so you don't get the same pseudo-sequence every time
	time_t t;
	srand((unsigned) time(&t));

	addEvent(0.1,NEWIO,0,0);
	runEventLoop(60000.0);

	IOmgr_stats();
	scheduler_stats();
	diskDrive_stats();
	return 0;
}