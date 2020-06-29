#ifndef TESTCOMMON_H
#define TESTCOMMON_H

#include <string>

// file containing test code that's factored out.

//  Default the directory that has shared memory special files
//  to the one used by linux.
//  This can be overridden for other systems and places.
//


#ifndef SHM_DIRECTORY
#define SHM_DIRECTORY "/dev/shm/"
#endif


void* mapRingBuffer(const char* name);
std::string shmName(std::string name);
std::string uniqueRing(std::string basename);

#endif
