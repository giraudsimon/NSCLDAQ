/*
  Test code that's factored out.
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "testcommon.h"
#include "Asserts.h"
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdio.h>

// Map a ring buffer, return the mapping address.
// fails the test if there's an error.
void*
mapRingBuffer(const char* name)
{
  int fd = shm_open(shmName(name).c_str(), O_RDWR, 0666);
  if (fd < 0) {
    fprintf(stderr, "Failed shm_open of %s\n", name);
    perror ("shm_open");
  }
  ASSERT(fd >= 0);
  struct stat buf;

  fstat(fd, &buf);

  void* map = mmap(0, buf.st_size, PROT_READ |PROT_WRITE, MAP_SHARED,
		   fd, 0);
  close(fd);

  ASSERT(map != MAP_FAILED);

  return map;
  
}


// Takes  a shared memory name and converts it to a name fo re.g. shm_open, shm_unlink.
//
std::string shmName(std::string name)
{
 std::string full("/");
 full += name;
 return name;

}

//
// Given a base name, returns a unique ring name.
// done by qualifying the basename with the pid.
// Limitations - total ring name must be < 1000 chars.
std::string uniqueRing(std::string baseName)
{
    pid_t pid = getpid();
    char fullName[1000];
    sprintf(fullName, "%s_%d", baseName.c_str(), pid);
    return std::string(fullName);
    
}