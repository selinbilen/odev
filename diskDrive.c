#include "diskDrive.h"
#include "eventLoop.h"
#include <stdio.h>
#include <assert.h>

/* Disk Spinning Assumptions
		If, when we get a request, the request is longer than DISK_STOP_TIME
			since the last time the disk was used, we can assume the disk is no
			longer spinning.
		The last time the disk was used is kept in the global lastDiskAccess variable.
		If the disk is no longer spinning, we need to start spinning, and it will
		   take DISK_START_TIME msecs to get it going again.
		Note that starting the disk spinning can overlap with armature movement.
-----------------------------------------------------------------------------*/
#define DISK_STOP_TIME 2000.0
#define DISK_START_TIME 300.0
float lastDiskAccess= (-DISK_STOP_TIME) -1;

/* Seek time Assumptions...
		- When moving the read/write armature from one track to another,
			we need to accelerate the armature up to max speed, travel at
			that max speed, and then decellerate the armature.
		- We model accelleration/deceleration with ARMATURE_ACCELERATION,
			the avg number of msecs required to travel from one track to another while
			accelerating or decelarating
		- We assume that it takes ACCELERATION_TRACKS/2 to accelerate the arm,
		  and ACCELERATION_TRACKS/2 to decelerate the arm.
		- Once the arm is up to speed, we assume it takes TRACK_TO_TRACK_DELAY
		  msecs to travel from one track to another.
		- If the armature travels less than ACCELERATION_TRACKS, we assume the
			travel time is the actual number of tracks traveled * ARMATURE_ACCELERATION
-----------------------------------------------------------------------------*/
#define ARMATURE_ACCELERATION 1.8
#define ACCELERATION_TRACKS 6
#define TRACK_TO_TRACK_DELAY 1.3
int currentTrack=0;

/* Rotational Latency Assumptions
		The hard disk has a specific SPINDLE_SPEED defined in terms of RPM
		So rotational latency to get an average of 180 degrees of rotation is
		(1/SPINDLE_SPEED RPM) * (3600 sec/Min) * (1000msec/sec) * (1/2 Rev)
-----------------------------------------------------------------------------*/
#define SPINDLE_SPEED 12000
float const rotationLatency = 3600000.0 / (2 *SPINDLE_SPEED);

/* Data transfer assumptions
		Based on the Wikipedia article, sustained transfer rates are higher than 1030MBPS
			at 7200 RPM
		Assume that goes up proportional to the SPINDLE_SPEED, and a block is 4K
		So: block transfer time = 1030 * SPINDLE_SPEED/7200 * 1s/1000ms * 1M/256B
-----------------------------------------------------------------------------*/
const float blockTransferTime = (1030 * SPINDLE_SPEED) / (7200*1000.0 * 256.0);

// Utility Functions
double fabs(double x) { return x>0.0?x:-x; }
float max(float x,float y) { return x>y?x:y; }

/* Statistics...
-----------------------------------------------------------------------------*/
int nDiskReq=0; // number of requests
float tElap=0.0; // total elapsed time to satisfy those requests
int nStart=0; // Number of times needed to start the disk
float tSeek=0.0; // total seek time
int tTrks=0; // Total number of track movements
int tBlks = 0; // total number of blocks transfered
float tIdle=0.0;

/* the diskIO function is called by the event loop in response to...
	addEvent(start,DISKIO,track,blocks)
-----------------------------------------------------------------------------*/
void diskIO(float start,int track,int nblocks) {
	/* simulates a single IO request
			- Calculate the time required to satisfy the request
			- Post an event... request satisfied at a given time in the future
	--------------------------------------------------------------------------*/
	assert(track>=0 && track<TRACKS);

	nDiskReq++;
	if (lastDiskAccess < 0) tIdle+=start;
	else tIdle+=start-lastDiskAccess;
	float accessTime=0.0;
	// First, move the armature to the requested track. See seek time assumptions above
	int move=fabs(currentTrack-track); // Number of tracks we need to move
	tTrks+=move;

	if (move <= ACCELERATION_TRACKS) accessTime = move * ARMATURE_ACCELERATION;
	else accessTime = (ARMATURE_ACCELERATION * ACCELERATION_TRACKS) +
		(move-ACCELERATION_TRACKS) * TRACK_TO_TRACK_DELAY;
	tSeek+=accessTime;
	if (start > (lastDiskAccess + DISK_STOP_TIME)) { // Need to start spinning again
		nStart++;
		accessTime = max(DISK_START_TIME,accessTime);
	}
	currentTrack=track; // Keep the current location of the armature

	// Next, Rotational delay to get the sector under the head
	// Assume an average of 180 degree rotation to get to the requested sector
	accessTime+=rotationLatency;

	// Finally, transfer time to read or write nblocks of data
	accessTime+=nblocks * blockTransferTime;
	tBlks+=nblocks;

	// Update last access time
	lastDiskAccess=start + accessTime;
	tElap+=accessTime;

	// printf("Disk I/O: Start=%6.1fms Track=%4d blocks=%3d time=%6.2fms\n",
	//	start,currentTrack,nblocks,accessTime);

	// Que a callback to the scheduler to indicate you are available when done
	addEvent(lastDiskAccess,DRIVE_FREE,0,0);
}

void diskDrive_stats() {
	float avgBlks=((float)tBlks)/nDiskReq;
	printf("\nDisk drive statistics for %d I/O requests...\n",nDiskReq);
	printf("   Average elapsed time      : %6.2fms\n",tElap/nDiskReq);
	printf("      avg seek time          : %6.2fms\n",tSeek/nDiskReq);
	printf("      avg rotation latency   : %6.2fms\n",rotationLatency);
	printf("      avg data transfer      : %6.2fms\n",avgBlks*blockTransferTime);
	printf("   Average number of blocks transfered = %4.2f blocks\n",avgBlks);
	printf("   Average number of tracks moved per request = %6.2f tracks\n",((float)tTrks)/nDiskReq);
	printf("   Average elapsed time per block: %6.2fms\n",tElap/tBlks);
	printf("   Started spinning for %d requests.\n",nStart);
	printf("   Total idle time: %6.2fms\n",tIdle);
}