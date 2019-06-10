
#ifndef DAQ_OS_CPOSIXBLOCKINGRECORDLOCK_H
#define DAQ_OS_CPOSIXBLOCKINGRECORDLOCK_H

#include <fcntl.h>

namespace DAQ {
  namespace OS {

class CPosixBlockingRecordLock
{
  public:
    enum Location { BEG, CUR, END };
    enum AccessMode { Read, Write };

  private:
    int          m_fd;
    struct flock m_lockInfo;

  public:
    CPosixBlockingRecordLock(int fd, AccessMode mode, Location whence, off_t startOffset, off_t nBytes);
    ~CPosixBlockingRecordLock();
    
  private:
    void release();
    short computeWhence(Location whence);
};

}  // end OS namespace
} // end DAQ namespace


#endif
