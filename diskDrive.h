#ifndef DISKDRIVE_H
/* References used...
	https://en.wikipedia.org/wiki/Hard_disk_drive_performance_characteristics
	https://www.ntfs.com/hard-disk-basics.htm
-----------------------------------------------------------------------------*/

/* Track model...
		- The disk is divided up into TRACKS tracks, where each track is a specific distance
		   from the center of the disk.
		- The outermost track is track number 0
		- The innermost track is track number TRACKS-1
		- The current track is where the actuator arm currently resides
*----------------------------------------------------------------------------*/
#define TRACKS 1024

/* the diskIO function is called by the event loop in response to...
	addEvent(start,DISKIO,track,blocks)
-----------------------------------------------------------------------------*/
void diskIO(float start,int track,int nblocks);
void diskDrive_stats();
#define DISKDRIVE_H
#endif