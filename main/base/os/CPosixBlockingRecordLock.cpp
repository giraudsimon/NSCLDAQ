#include <CPosixBlockingRecordLock.h>
#include <ErrnoException.h>

#include <cstring>


namespace DAQ {
  namespace OS {

      CPosixBlockingRecordLock::CPosixBlockingRecordLock(int fd, AccessMode mode, Location whence, off_t startOffset, off_t nBytes)
        : m_fd(fd), m_lockInfo()
      {
        m_lockInfo.l_type   = ( mode == Read ) ? F_RDLCK : F_WRLCK;
        m_lockInfo.l_whence = computeWhence(whence);
        m_lockInfo.l_start  = startOffset;
        m_lockInfo.l_len    = nBytes;

        int status = fcntl(m_fd, F_SETLKW, &m_lockInfo);
        if (status < 0) {
          throw CErrnoException(strerror(errno));
        }
      }


      CPosixBlockingRecordLock::~CPosixBlockingRecordLock()
      {
        release();
      }

      void CPosixBlockingRecordLock::release() 
      {
        m_lockInfo.l_type = F_UNLCK;
        fcntl(m_fd, F_SETLK, &m_lockInfo);
      }


      short CPosixBlockingRecordLock::computeWhence(Location whence)
      {
        if (whence == BEG) return SEEK_SET;
        else if (whence == CUR) return SEEK_CUR;
        else return SEEK_END;
      }

  } // end OS namespace
} // end DAQ namespace
