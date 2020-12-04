CC=gcc
CFLAGS=-ggdb -Wall

test : simDisk simScan
	date > results.txt
	./simDisk 2>&1 | tee -a results.txt
	./simScan 2>&1 | tee -a results.txt
	
checkMem : simDisk simScan
	date > results.txt
	echo "checking memory" >> results.txt
	valgrind --leak-check=full --show-leak-kinds=all ./simDisk | tee -a results.txt
	valgrind --leak-check=full --show-leak-kinds=all ./simScan | tee -a results.txt
	
simDisk : simDisk.c eventLoop.c eventLoop.h diskDrive.c diskDrive.h schedFifo.c scheduler.h OS_IOmgr.c OS_IOmgr.h
	${CC} ${CFLAGS} -o simDisk simDisk.c eventLoop.c OS_IOmgr.c schedFifo.c diskDrive.c 2>&1 | tee compMsg.txt
	
simScan : simDisk.c eventLoop.c eventLoop.h diskDrive.c diskDrive.h schedScan.c scheduler.h OS_IOmgr.c OS_IOmgr.h
	${CC} ${CFLAGS} -o simScan simDisk.c eventLoop.c OS_IOmgr.c schedScan.c diskDrive.c 2>&1 | tee compMsg.txt	
	
clean :
	-rm simDisk simScan results.txt compMsg.txt