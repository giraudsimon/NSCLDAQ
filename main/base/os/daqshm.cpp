/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * This file is the Linux specific implementation of the
 * shared memory API.
 * It really should work in all systems that support POSIX shm_open/shm_close/mmap.
 */

#include "daqshm.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

/**
 * Constant definitions:
 */

// Error codes.

const int CDAQShm::Success      = 0;
const int CDAQShm::Exists       = 1;
const int CDAQShm::NonExistent  = 2;
const int CDAQShm::NoAccess     = 3;
const int CDAQShm::NotAttached  = 4;
const int CDAQShm::CheckOSError = 5;


//  Be sure to update this if additional error codes are added:
//
static const int lastError= CDAQShm::CheckOSError;


// Permissions bits:

const int CDAQShm::GroupRead  = 0x01;
const int CDAQShm::GroupWrite = 0x02;
const int CDAQShm::OtherRead  = 0x10;
const int CDAQShm::OtherWrite = 0x20;

// Static members.

int CDAQShm::m_nLastError(CDAQShm::Success);
CDAQShm::Attachments CDAQShm::m_attachMap;


// Error message:

static const char*  pMessages[] = {
  "Successful completion",
  "Shared memory region already exists",
  "No such shared memory region",
  "Access to shared memory denied",
  "Check the operating system error return value"
};
const char** CDAQShm::m_ppMessages = pMessages;
const int    CDAQShm::m_nMessages  = sizeof(pMessages)/sizeof(const char*);

// This is Linux specific -- directory holding the shared memory files.:


CScopedDAQShm::CScopedDAQShm(std::string name, int oflags)
  : m_fd(-1)
{
  m_fd = CDAQShm::open(name, oflags);
}

CScopedDAQShm::~CScopedDAQShm()
{
  close(m_fd);
  m_fd = -1;
}

/**
 * Create a new shared memory region.
 *
 * @param name - Name of the shared memory should be a filesystem like name e.g. /test
 * @param size - Minimum number of bytes the shared memory should be big.
 * @param flags- Bit mask of the permissinos bits:
 *    - CDAQShm::GroupRead  - members of the group have read access.
 *    - CDAQShm::GroupWrite - Members of the group have write access.
 *    - CDAQShm::OtherRead  - Non group members have read access.
 *    - CDAQShm::OtherWrite - Non group members have write access. 
 * @return bool
 * @retval false - success.
 * @retval true  - failure with the last error code available from lastError().
 */
bool
CDAQShm::create(std::string name, size_t size, unsigned int flags)
{
  m_nLastError = CDAQShm::Success;

  // Figure out the mode:

  mode_t mode = S_IRUSR | S_IWUSR;	// Start up wide open to user.
  

  // or in appropriate group bits:

  if (flags & GroupRead) {
    mode |= S_IRGRP;
  }
  if (flags & GroupWrite) {
    mode |= S_IWGRP;
  }

  // or in appropriate other bits:

  if (flags & OtherRead) {
    mode |= S_IROTH;
  }
  if (flags & OtherWrite) {
    mode |= S_IWOTH;
  }

  // Create the special file:

  mode_t old = umask(0);
  int fd = shm_open(name.c_str(), O_CREAT | O_EXCL |O_RDWR, mode);
  int e = errno;		// We want shm_open's errno not umask's
  umask(old);
  errno = e;
  if (fd < 0) {
    setLastErrorFromErrno();
    return true;
  }
  // Now set its length:

  if (ftruncate(fd, size)) {
    m_nLastError = CheckOSError;
    return true;
  }
  // And we're done with it for now

  close(fd);

  return false;
}
/**
 * Delete a shared memory region.
 *
 * @param name - name of the section to destroy.
 * @return bool
 * @retval false - success.
 * @retval true  - failure with the last error code available from lastError().
 */
bool
CDAQShm::remove(std::string name)
{
  m_nLastError = CDAQShm::Success;
  if(shm_unlink(name.c_str())) {
    setLastErrorFromErrno();
    return true;
    
  }
  return false;
}
/**
 *  Connect the program with a shared memory region.
 *  
 * @param name - Name of the shared memory region.
 *
 * @return void*
 * @retval 0 - Error, see lastError() for reason.
 * @retval non-zero - pointer to the base of the attached region.
 *
 */
void*
CDAQShm::attach(std::string name)
{
  m_nLastError = Success;

  // Open the region.. not that if O_RDWR fails we try to open
  // with O_RDONLY

  int fd = shm_open(name.c_str(), O_RDWR, 0);
  if (fd < 0) {
    fd = shm_open(name.c_str(), O_RDONLY, 0);
  }

  if (fd < 0) {
    setLastErrorFromErrno();
    
    return 0;
  }


  // fd has the file descriptor figure out the size of the file in bytes:

  ssize_t fileSize = fdSize(fd);
  if (fileSize < 0)  {
    return 0;
  }

  // IF there's already a mapping for this shm, just return it:

  if (m_attachMap.find(name) != m_attachMap.end()) {
    m_attachMap[name].refcount++;
    return m_attachMap[name].pMappingAddress;
  }

  // Mmap the dev special.. if a read/write mapping failed try readonly:

  void *pMemory = mmap(NULL, fileSize, PROT_READ | PROT_WRITE,
		       MAP_SHARED, fd, 0);
  if (!pMemory) {
    pMemory = mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fd, 0);
  }
  if (pMemory == reinterpret_cast<void*>(-1)) {
    setLastErrorFromErrno();
    
    return 0;
  }

  close(fd);
  
  // Add a new mapping entry with refcount 1:
  // Note that fileSize is only negative in errors which were already handled.
  // so the cast below is safe as fileSize will be size_t  by now.
  
  attachInformation initialInfo = {pMemory, static_cast<size_t>(fileSize), 1};
  m_attachMap[name] = initialInfo;

  return pMemory;
}

/**
 * Return the number of bytes in a shared memory region:
 *
 * @param name - name of the region.
 */
ssize_t
CDAQShm::size(std::string name)
{
  int fd = shm_open(name.c_str(), O_RDONLY, 0);
  if (fd < -1) {
    setLastErrorFromErrno();
    
    return -1;
  }
  // The gymnastics below ensures that errno contains the value
  // from the fdSize not the close call.

  ssize_t result = fdSize(fd);
  int e = errno;
  close(fd);
  errno = e;
  return result;
}
/**
 * Detach the mapping.
 * Really this just decrements the reference count on the m_attachMap entry
 * and only actually unmaps if when it becomes zero.
 *
 * @param p    - Pointer to mapped virtual address.
 * @param name - Name of shared memory region.
 * @param size - Size f shared memory region.
 * 
 * @return bool
 * @retval false - success.
 * @retval true  - Some sort of failure that can be analyzed by lastError().
 */
bool
CDAQShm::detach(void* p, std::string name, size_t size)
{
  // Need to have an entry in the map else NotAttached.

  Attachments::iterator pEntry = m_attachMap.find(name);
  if (pEntry == m_attachMap.end()) {
    m_nLastError = NotAttached;
    return true;
  }
  // Finding it and having the wrong size or va is also an erro:

  if ((p != pEntry->second.pMappingAddress) ||
      (size != pEntry->second.mapSize)) {
    m_nLastError = NotAttached;
    return true;
  }


  pEntry->second.refcount--;
  if (pEntry->second.refcount == 0) {
    munmap(pEntry->second.pMappingAddress, pEntry->second.mapSize);
    m_attachMap.erase(pEntry);
  }

  return false;
}

/**
 * Return the value of the last status:
 *
 * @return
 * @retval last status value.
 */
int
CDAQShm::lastError()
{

  return m_nLastError;
}


/**
 * Return a message corresponding to an error code.
 * 
 * @param code - Error code returned from lastError()
 *
 * @return std::string
 * @retval - Error message corresponding to the code or, if the code is not legal,
 *           some message indicating that.
 */
std::string
CDAQShm::errorMessage(int errorCode)
{
  if ((errorCode < 0) || (errorCode >= m_nLastError)) {
    return std::string("Invalid error code");
  }
  return std::string(m_ppMessages[errorCode]);

}

/**
 * stat
 *    Return a stat struct for the shared memory special file.  This is POSIX
 *    compliant via shm_open/fstat.
 *
 * @param name - name of the shared memory region
 * @param pStat - Pointer to buffer to hold the status info.
 * @return int  - 0 success, -1 failure with m_nLastError set with the reason.
 */
int
CDAQShm::stat(std::string name, struct stat* pStat)
{
  // Open the file:
  
  int fd = shm_open(name.c_str(), O_RDONLY, 0);
  if (fd < 0) {
    m_nLastError = CDAQShm::CheckOSError;
    return -1;
  }
  
  // Do the stat:

  int result = fstat(fd, pStat);
  int eno = errno;                        // Close modifies errno
  close(fd);
  errno = eno;
  if (result < 0) {
    setLastErrorFromErrno();
  } else {
    m_nLastError = Success;
  }
  return result;
  
}

int CDAQShm::open(std::string name, int oflag)
{
  return shm_open(name.c_str(), oflag, 0);
}


/*-----------------------------------------------------------------------------------*/
/* Private methods:                                                                 */


/**
 * Return the size given a good fd:
 *  
 *  @param fd  - file descriptor to query.
 *  @return ssize_t
 *  @retval >=0 - success.
 *  @retval < 0 - Failure.
 */
ssize_t
CDAQShm::fdSize(int fd)
{
  struct stat fileInfo;
  int status = fstat(fd, &fileInfo);
  if (status) {
    m_nLastError = CheckOSError;
    return -1;
  }
  size_t fileSize = fileInfo.st_size;
  return fileSize;
}
/**
 * setLastErrorFromErrno
 *    Sets the m_nLastError from the current value of errno.
 */
void
CDAQShm::setLastErrorFromErrno()
{
  switch errno {
    case EEXIST:
      m_nLastError = Exists;
      break;
    case ENOENT:
      m_nLastError = NonExistent;
      break;
    case EACCES:
      m_nLastError = NoAccess;
      break;
    default:
      m_nLastError = CheckOSError;
      break;
  }
}
