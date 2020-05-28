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
#ifndef DAQSHM_H
#define DAQSHM_H

#include <string>
#include <map>
#include <stddef.h>		// for size_t.
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


/*!
 *  \brief A simple class to open and close a shm file using RAII
 */
class CScopedDAQShm {
  private:
    int m_fd;

  public:
    /*! \brief Open the file and cache the fd
     *
     * Any errors will arise from CDAQShm::open().
     */
    CScopedDAQShm(std::string name, int oflag);

    /*! \brief Close the file descriptor
     */
    ~CScopedDAQShm();

    /*! \returns the open file descriptor */
    int getFd() { return m_fd; }
};


/**
 * CDAQShm provides an operating system independent interface for'
 * creating and accessing shared memory segments.  The specific
 * implementations may be operating system dependent, however the class
 * provides a facade that is os independent.
 * 
 * A minimalist approach was taken, feel free to expand the interface definition
 * and implementations as special interests arise.
 */
class CDAQShm
{        
public:
  static bool        create(std::string name, size_t size, unsigned int flags);
  static void*       attach(std::string name);
  static bool        detach(void* pSharedMemory, std::string name, size_t size);
  static bool        remove(std::string name);
  static ssize_t     size(std::string name);
  static int         lastError();
  static std::string errorMessage(int errorCode);
  static int         stat(std::string name, struct stat* pStat);
  static int         open(std::string name, int oflag);
  

private:
  static ssize_t fdSize(int fd);
  static void setLastErrorFromErrno();
public:
  static const int Success;
  static const int Exists;
  static const int NonExistent;
  static const int NoAccess;
  static const int NotAttached;
  static const int CheckOSError;
  
  static const int GroupRead;
  static const int GroupWrite;
  static const int OtherRead;
  static const int OtherWrite;



  typedef struct _attachInformation {
    void*        pMappingAddress;
    size_t       mapSize;
    unsigned int refcount;
  } attachInformation, *pAttachInformation;
  typedef std::map<std::string,attachInformation> Attachments;


private:

  static const char** m_ppMessages;
  static const int    m_nMessages;
  static       int    m_nLastError;
  static       Attachments  m_attachMap;
};

#endif
