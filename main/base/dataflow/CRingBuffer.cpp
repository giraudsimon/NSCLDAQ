/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "CRingBuffer.h"
#include "CRingMaster.h"
#include "ringbufint.h"

#include <ErrnoException.h>
#include <RangeError.h>
#include <StateException.h>
#include <os.h>
#include <CPosixBlockingRecordLock.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>

#include <CPortManager.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <daqshm.h>

#include <iostream>


using namespace std;

// constants;

static string localhost("127.0.0.1");

/*
  This file implements the CRingBuffer class.  

*/


// Class Data.

size_t CRingBuffer::m_defaultDataSize(DEFAULT_DATASIZE);
size_t CRingBuffer::m_defaultMaxConsumers(DEFAULT_MAX_CONSUMERS);

CRingMaster* CRingBuffer::m_pMaster(NULL);
pid_t        CRingBuffer::m_myPid(-1); // no pid has this.


//////////////////////////////////////////////////////////////////////////////
// 
// Predicate requiring at least x amount of free buffer space.
//

class CRingFreeSpacePredicate : public CRingBuffer::CRingBufferPredicate
{
private:
  size_t m_sizeRequired;
public:
  CRingFreeSpacePredicate(size_t size) : m_sizeRequired(size) {}
  virtual bool operator()(CRingBuffer& ring) {
    return m_sizeRequired > ring.availablePutSpace();
  }

};

/////////////////////////////////////////////////////////////////////////////
//
// Predicate requiring at least x amount of data to be there for this
// consumer.

class CRingDataAvailablePredicate : public CRingBuffer::CRingBufferPredicate
{
private:
  size_t m_sizeRequired;
public:
  CRingDataAvailablePredicate(size_t size) : m_sizeRequired(size) {}
  virtual bool operator()(CRingBuffer& ring) {
    return m_sizeRequired > ring.availableData();
  }
};

//////////////////////////////////////////////////////////////////////////////
// Class level functions.

/*!
  Create a new ring buffer memory segment.  Note that all data that is in the
  segment will be lost and conumers will get lost too. This method is synchronized
  with other methods running in different processes by a semaphore.

  \param name       - Name of the ring buffer.  Will have '/' prepended to it.
  \param dataBytes  - Number of data bytes in the file.  This will be extended to the
                      next page boundary if it is not already a multiple of the
                      page size.
  \param maxConsumers - Maximum number of consumers allowed (number of 
                      consumer ClientInformation entries in the ring buffer.
  \param tempMasterConnection  - 
                      A temporary connection to the ring master is formed and
                      then destroyed to register this ring.  This is done when
                      the client does not want the ring master fd to be
                      inherited by child processes that may be created.

  \throw CErrnoException

*/
void
CRingBuffer::create(std::string name, 
		     size_t dataBytes,
		     size_t maxConsumer,
		     bool   tempMasterConnection)
{

    // Figure out the entire size of the shared memory region and truncate the file to that
    // size:
    size_t headerSize = sizeof(RingHeader) +
      sizeof(ClientInformation)*(maxConsumer+1);

    size_t rawSize   = dataBytes + headerSize;

    long   pageSize  = sysconf(_SC_PAGESIZE);
    size_t pages     = (rawSize + (pageSize-1))/pageSize;
    size_t shmSize   = pages*pageSize;

    std::string memoryName = shmName(name);

    // If the shared memory does not exist, just create it:

    ssize_t existingSize = CDAQShm::size(memoryName);

    bool created = false;
    if (existingSize < 0) {
        if(CDAQShm::create(shmName(memoryName), shmSize,
                           CDAQShm::GroupRead | CDAQShm::GroupWrite | CDAQShm::OtherRead | CDAQShm::OtherWrite)) {
            throw CErrnoException("Shared memory creation failed");
        } else {
          created = true;
        }
    }

    // lock
    
    CScopedDAQShm shmemFile(shmName(name), O_RDWR);
    DAQ::OS::CPosixBlockingRecordLock lock(shmemFile.getFd(),
                                  DAQ::OS::CPosixBlockingRecordLock::Write,
                                  DAQ::OS::CPosixBlockingRecordLock::BEG,
                                  0, headerSize); 
    if (created) {

        unsynchedFormat(name, maxConsumer);

    }  else if (isRing(name)) {
        // If the memory region exists - and is a ring
        //  *   If the ring master knows about it it's an error to make a new one.
        //  *   If the ring master does not know about it and it's  ring, make it known to the ring master.

        CRingMaster master;
        bool exists = true;
        try {
            std::string usage = master.requestUsage();

            // If the ring exists, the string "{ringname " will be present in
            // the usage information:

            std::string ifexists = "{";
            ifexists += name;
            ifexists += " ";
            if (usage.find(ifexists) == std::string::npos) {
                exists = false;
            }

        }
        catch (...) {
            exists = false;		// Throws happen if the ring master does _not_ know about the ring
        }
        if (exists) {
            errno  = EEXIST;
            throw CErrnoException("Ring buffer already exists");
        }


    } else {
        // If the memory region exists but is not a ring that's ean error.

        errno = EEXIST;
        throw CErrnoException("Shared memory region exists but is not formatted as a ring buffer");
    }
    // Notify the ring master this has been created.

    CRingMaster* pOld = m_pMaster;
    m_pMaster = 0;
    connectToRingMaster();
    m_pMaster->notifyCreate(name);

    if (tempMasterConnection) {
        delete m_pMaster;
        m_pMaster = pOld;
    }
    

}

/**
 * Utility function for ring producers.  If the specified ring does not exist, create it
 * then attach as the producer. This method is synchronized
  with other methods running in different processes by a semaphore.
 *
 * @param name      - Name of the ring.
 * @param dataBytes - size of the ring data segment.
 * @param maxConsumers - Maximum number of consumers.
 * @param tempMasterConnection - If true a temporary connection to the ring master is formed
 *                        then destroyed for the ring registration.
 *
 * @note The parameters other than name are only used when the ring needs to be created.
 *
 * @return CRingBuffer*
 * @retval Pointer to the producer CRingBuffer object this is dynamically allocated
 *         and must be delete'd by the caller at some point.
 */
CRingBuffer*
CRingBuffer::createAndProduce(std::string name, size_t dataBytes, size_t maxConsumer,
			      bool   tempMasterConnection)
{
  if (!isRing(name)) {
    create(name, dataBytes, maxConsumer, tempMasterConnection);
  }
  return new CRingBuffer(name, producer);

}
/*!
   Destroy an existing ring buffer.  Note that the shared memory segment 
   actually continues to live until all processes that hold maps and fds open
   unmap/close their fds. This method is synchronized
  with other methods running in different processes by a semaphore.

   \param name - The ring buffer name.

   \throw CErrnoException if the shm_unlink fails.

*/
void
CRingBuffer::remove(string name)
{

    if (!isRing(name)) {
        errno = ENOENT;
        throw CErrnoException("CRingBuffer::remove - not a ring");
    }

    // If _I_ don't own the ring don't allow deletion (unless I'm root).

    struct stat shmInfo;
    int status = CDAQShm::stat(name, &shmInfo);
    if (status == -1) {
        throw CErrnoException("CRingBuffer::remove - Getting info about shared memory");
    }
    uid_t me = getuid();
    if ((me != 0) && (me != shmInfo.st_uid)) {
        errno = EPERM;
        throw CErrnoException("CRingBuffer::remove - checking ringbuffer ownership");
    }

    // lock
    size_t headerSize = sizeof(RingHeader) +
      sizeof(ClientInformation)*(m_defaultMaxConsumers+1);
    CScopedDAQShm shmemFile(shmName(name), O_RDWR);
    DAQ::OS::CPosixBlockingRecordLock lock(shmemFile.getFd(),
                                  DAQ::OS::CPosixBlockingRecordLock::Write,
                                  DAQ::OS::CPosixBlockingRecordLock::BEG,
                                  0, headerSize); 

    // Connect to the ring master:
    connectToRingMaster();

    // At this point RM has acked so we can kill the ring itself:
    // Tell the ringmaster to forget the ring and kill the clients

    m_pMaster->notifyDestroy(name);

    // Delete the ring shared memory special file.

    string fullName   = shmName(name);

    if (CDAQShm::remove(fullName)) {
        throw CErrnoException("Shared memory deletion failed");
    }

}

/*!
   Format an existing ring buffer, but without synchronization. This is
   the implemenation of the format logic.

   - Open the shared memory segment
   - Map the shared memory segment to the full size.
   - Format the front of the ring so that there's no producer or consumers.
   - So that the header matches the layout of the buffer.

   \param name         - Name of the ring buffer. (a / will be prepended).
   \param maxConsumers - Maximum number of supported consumers.

   \throw CErrnoException

*/
void CRingBuffer::unsynchedFormat(std::string name, size_t maxConsumer)
{
    string fullName = shmName(name);
    size_t memSize = CDAQShm::size(fullName);

    // Now map the ring:

    pRingBuffer      pRing      = reinterpret_cast<pRingBuffer>(CDAQShm::attach(fullName));


    // Map the ring:

    pRingHeader        pHeader   = reinterpret_cast<pRingHeader>(pRing);
    pClientInformation pProducer = reinterpret_cast<pClientInformation>(pHeader + 1);
    pClientInformation pClients  = pProducer + 1;

    // Fill in the header:

    strcpy(pHeader->s_magicString, MAGICSTRING);

    pHeader->s_maxConsumer       = maxConsumer;
    pHeader->s_producerInfo      = (reinterpret_cast<char*>(pProducer) -
                    reinterpret_cast<char*>(pHeader));
    pHeader->s_firstConsumer     = (reinterpret_cast<char*>(pClients) -
                    reinterpret_cast<char*>(pHeader));
    pHeader->s_topOffset         = memSize-1;
    pHeader->s_dataOffset        = sizeof(RingHeader) +
                                   sizeof(ClientInformation)*(maxConsumer+1);
    pHeader->s_dataBytes         = memSize - pHeader->s_dataOffset;

    // Fill in the client information data structures:

    pProducer->s_offset          = pHeader->s_dataOffset;
    pProducer->s_pid             = -1;

    for (int i=0; i < maxConsumer; i++) {
      pClients->s_offset         = pHeader->s_dataOffset;
      pClients->s_pid            = -1;
      pClients++;
    }
    CDAQShm::detach(pRing, fullName, memSize);
}

/*!
   Format an existing ring buffer.
   - Open the shared memory segment
   - Map the shared memory segment to the full size.
   - Format the front of the ring so that there's no producer or consumers.
   - So that the header matches the layout of the buffer.

   This method is synchronized with other methods running in different
   processes by a semaphore.

   \param name         - Name of the ring buffer. (a / will be prepended).
   \param maxConsumers - Maximum number of supported consumers.

   \throw CErrnoException

*/
void
CRingBuffer::format(std::string name,
		    size_t maxConsumer)
{
  // lock
    size_t headerSize = sizeof(RingHeader) +
      sizeof(ClientInformation)*(maxConsumer+1);
    CScopedDAQShm shmemFile(shmName(name), O_RDWR);
    DAQ::OS::CPosixBlockingRecordLock lock(shmemFile.getFd(),
                                  DAQ::OS::CPosixBlockingRecordLock::Write,
                                  DAQ::OS::CPosixBlockingRecordLock::BEG,
                                  0, headerSize); 

    // need the memory size for initialization.

    unsynchedFormat(name, maxConsumer);

}
/*!
  Determine if a name corresponds to a ring buffer.
  \param name - the candidate name of the ring buffer.
  \return bool
  \retval true - Is a ring buffer.
  \retval false- Is not a ring buffer.
*/
bool
CRingBuffer::isRing(string name)
{
  std::string fullName = shmName(name);
  try {
    pRingBuffer p        = reinterpret_cast<pRingBuffer>(CDAQShm::attach(fullName));
    if (!p) return false;
    bool        result   = ringHeader(p);
    CDAQShm::detach(p, fullName, CDAQShm::size(fullName));

    return result;
  }
  catch (...) {
    return false;
  }
}				

/*!
  Set the defafult ring size.  This will be the amount of data the ring can hold
  if that's not supplied at creation time.
  \param byteCount - number of bytes.
*/
void
CRingBuffer::setDefaultRingSize(size_t byteCount)
{
  m_defaultDataSize = byteCount;
}
/*!

  \return size_t
  \retval Return the default amount of data a ring will hold:
*/
size_t
CRingBuffer::getDefaultRingSize()
{
  return m_defaultDataSize;
}
/*!
  Set the default maximum consumer count.
  \param numConsumers  - number of consumers.
*/
void
CRingBuffer::setDefaultMaxConsumers(size_t numConsumer)
{
  m_defaultMaxConsumers = numConsumer;
}
/*!
     \return size_t
     \retval Maximum number of consumers in a ring that has been created
             without specifying that value.
*/
size_t
CRingBuffer::getDefaultMaxConsumers()
{
  return m_defaultMaxConsumers;
}

/**
 * Return the name of the default ring.  This is the name of the logged in
 * user.
 * 
 * @return std::string
 * @retval the user associated with the current uid.
 * 
 * @throws std::string - if the uid could not be looked up for some reason.
 */
std::string
CRingBuffer::defaultRing()
{
  return Os::whoami();


}


/**
 * Returna fully decorated URL for the default ring e.g.
 * tcp://localhost/<user-name>
 */
std::string 
CRingBuffer::defaultRingUrl()
{
  std::string url = "tcp://localhost/";
  url += defaultRing();
  return url;
    
}

/*!
 * \brief Attach and manipulate the header of the ringbuffer 
 *
 * This should be done with synchronization. It is expected that a semaphore
 * has been locked already in the process that called it. 
 *
 * If the CRingBuffer object is has a manager mode, then this is a no-op.
 * Producers and consumers on the other hand cause the ringbuffer header
 * to be manipulated.
 *
 * \throws CErrnoException when attempting to add a second producer
 * \throws CErrnoException when mode is neither producer, consumer, or manager
 *
 */
void CRingBuffer::attach() {

  if (m_mode == producer) {
    if (m_pRing->s_producer.s_pid == -1) {
      m_pClientInfo         = &(m_pRing->s_producer);
      m_pClientInfo->s_pid  = getpid(); // leave the offset where it was.
      __sync_synchronize();		  // And flush to shm.
    }
    else {
      errno = EACCES;
      throw CErrnoException("CRingBuffer::CRingBuffer - already has a producer");
    }
  } 
  else if (m_mode == consumer) {
    allocateConsumer();
  } else if (m_mode == manager) {
  }
  else {			// Invalid connection mode.
    errno = EINVAL;
    throw CErrnoException("CRingBuffer::CRingBuffer - invalid connection mode");
  }
}

///////////////////////////////////////////////////////////////////////////////////////
// Constructors and canonicals.


/*!
   Construct a ring buffer object. This will represent a consumer or a
   producer on the specified ring.

   This method is synchronized with other methods running in different
   processes by a semaphore.

   \param name - name of the ring buffer, "/" will be prepended.
   \param mode - Type of access requested.  This should be one of:
               - CRingBuffer::producer for producer access.
               - CRingBuffer::consumer for consumer access.

    \throw CErrnoException Some special errnos though:
    - ENOMEM - No free consumer data structs for consumer access.
    - EACCES - Producer access requested but there's already a producer. connected.
    - EINVAL - The connection mode is invalid.

*/
CRingBuffer::CRingBuffer(string name, CRingBuffer::ClientMode mode) :
  m_pRing(0),
  m_pClientInfo(0),
  m_mode(mode),
  m_pollInterval(DEFAULT_POLLMS),
  m_ringName(name)
{

    if (!isRing(name)) {
      errno = ENOENT;
      throw CErrnoException("CRingBuffer::CRingBuffer - not a ring");
    }
    m_pRing = mapRingBuffer(shmName(name));
    if (m_pRing == nullptr) {
      throw std::string("CRingBuffer::CRingBuffer - failed to map shared memory region.");
    }

    if (m_mode == manager) return;

    // lock
    size_t headerSize = sizeof(RingHeader) +
      sizeof(ClientInformation)*(m_defaultMaxConsumers+1);
    CScopedDAQShm shmemFile(shmName(name), O_RDWR);
    DAQ::OS::CPosixBlockingRecordLock lock(shmemFile.getFd(),
                                  DAQ::OS::CPosixBlockingRecordLock::Write,
                                  DAQ::OS::CPosixBlockingRecordLock::BEG,
                                  0, headerSize); 

    // Now that we're mapped the remainder of the constructor must execute in a try
    // block so that failure will allow us to unmap the ring.
    //
    try {
      attach();
    } catch (...) {
      unMapRing();
      throw;
    }
    // We got this far we have a connection.  If this is not a manager connection,
    // we can register with the ringmaster:

    try {
      connectToRingMaster();	// This will only really connect the first time.
      notifyConnection();		// Let the ring master know of our connection
    }
    catch(...) {
      // on any failure, we give up the ring and throw:

      m_pClientInfo->s_pid = -1;
      unMapRing();
      throw;
    }
}
/*!
  Destructor ... release our our client info and unmap.

   This method is synchronized with other methods running in different
   processes by a semaphore.

*/
CRingBuffer::~CRingBuffer()
{

  if (m_mode != manager) {
    //lock
    size_t headerSize = sizeof(RingHeader) +
      sizeof(ClientInformation)*(m_defaultMaxConsumers+1);
    CScopedDAQShm shmemFile(shmName(m_ringName), O_RDWR);
    DAQ::OS::CPosixBlockingRecordLock lock(shmemFile.getFd(),
                                  DAQ::OS::CPosixBlockingRecordLock::Write,
                                  DAQ::OS::CPosixBlockingRecordLock::BEG,
                                  0, headerSize); 

    string ringname = m_ringName;
    m_pClientInfo->s_pid = -1;
    // Let the ringmaster know we're disconnecting.
    // the client pointer is still valid as is the map so the notification
    // can still find the 'slot number.

    notifyDisconnection();

    // Unmap the ring and ensure that any use of this ring will fail utterly.

    unMapRing();

  } else {
    // Unmap the ring and ensure that any use of this ring will fail utterly.

    unMapRing();
  }
  
  // Zeroing pointers ensures that attempts to use this object will segflt.

  m_pRing       = 0;	      
  m_pClientInfo = 0;

}
/////////////////////////////////////////////////////////////////////////
// Member functions that manipulate the ring buffer.
//

/*!
   Put data into the ring buffer.
   - Block until there's sufficient space to do the put.
   - Put the data, wrapping as needed.
   - Update the put pointer.

   \note  The ring buffer must have been attached to in producer mode or
          a CStateException will be thrown.

    \param pBuffer - Pointer to the buffer containing the data to put in the buffer.
    \param nBytes  - Number of bytes to transfer.
    \param timeout - The timeout for the write.  The amount of time to block for the 
                     data to become available is determined by this.  The default value
		     is ULONG_MAX which works out to approximately 136 years.. which is
		     close enough to infinite that it can be treated as infinite.
		     A value of 0 will not block.

    \return size_t
    \retval nBytes  - The buffer was written to the ring buffer, all of it.
    \retval 0       - Wait for available space timed out.

    \throw CRangeException - The amount of data to write is larger than the
                            size of the ring buffer data segment.
    \throw CStateException - This CRingBufferObject is not open for producer use.
*/

size_t
CRingBuffer::put(const void* pBuffer, size_t nBytes, unsigned long timeout)
{
  validateTransferAccess(producer, "CRingBuffer::put");

  // Ensure the ring is big enough for the data:

  if (nBytes > m_pRing->s_header.s_dataBytes) {
    throw CRangeError(0, m_pRing->s_header.s_dataBytes, nBytes,
		      "CRingBuffer::put");

  }
  // Block until we have space. 

  CRingFreeSpacePredicate condition(nBytes);
  int status = blockWhile(condition, timeout);
  if (status) {
    return 0;			// timed out.
  }
  // Now we need to figure out how to put the data in the ring.
  // We may need to wrap the data across the top of the buffer.
  //

  off_t ringBase = m_pRing->s_header.s_dataOffset;
  off_t ringTop  = m_pRing->s_header.s_topOffset;
  char* pDataBase= reinterpret_cast<char*>(m_pRing) + ringBase;
  char* pPut     = reinterpret_cast<char*>(m_pRing) + m_pClientInfo->s_offset;

  if ((m_pClientInfo->s_offset + nBytes) <=  (ringTop+1)) {

    // Can move all at once...

    memcpy(pPut, pBuffer, nBytes);


  }
  else {
    // Need to move in two chunks:

    size_t firstSize = ringTop+1 - m_pClientInfo->s_offset;
    size_t secondSize= nBytes - firstSize;

    

    memcpy(pPut, pBuffer, firstSize);                     // Move the first chunk.

    const char* pSecond = reinterpret_cast<const char*>(pBuffer) + firstSize;
    memcpy(pDataBase, pSecond, secondSize);              // Move the second chunk. 

  }
  Skip(nBytes);

  // If we got this far success... issue a memory barrier to ensure this all is
  // written to the shm:

  __sync_synchronize();

  return nBytes;
}

/*!
   Get data from the ring buffer.  This object must have been opened 
   in consumer mode else a CStateException is thrown.

   \param pBuffer  - Pointer to the buffer in which the data from the ring will be put.
   \param maxBytes - Number of bytes the buffer can hold... or rather the maximum number 
                     of bytes the caller wants transferred.
   \param minBytes - The minimum number of bytes the caller wants transferred.
                     The caller will block until this number of bytes is available for it,
		     or until the timeout specified.
   \param timeout  - The maximum number of seconds to block for data.  The default is
                     ULONG_MAX which is about 136 years or essentially forever.  A value
		     of zero will never block.

   \return size_t
   \retval > 0  - The number of data bytes transferred to the buffer. this should be
                  between minBytes and maxBytes.  It will be the number of available bytes
                  at the time at least minBytes became available (but no more than
                  maxBytes).
    \retval 0   - The wait timed out.

    \throw  CStateException - this object is not a consumer object.
    \throw  CRangeException - minBytes is larger than the size of the ring buffer.
                              If we did not treat this as an exception, the caller would
                              block forever.
*/
size_t
CRingBuffer::get(void*        pBuffer, 
		 size_t       maxBytes, 
		 size_t       minBytes, 
		 unsigned long timeout)
{
  validateTransferAccess(consumer, "CRingBuffer::get");
  

  // Ensure that we won't block forever:

  if (minBytes > m_pRing->s_header.s_dataBytes) {
    throw CRangeError(0, m_pRing->s_header.s_dataBytes, minBytes,
		      "CRingBuffer::get");
  }

  // Wait until we have at least the desired numbe of bytes:

  CRingDataAvailablePredicate condition(minBytes);
  int status = blockWhile(condition, timeout);

  if (status) {
    return 0;			// Timed out.
  }
  // Figure out how much data we'll transfer:

  size_t transferSize =availableData();
  if (transferSize > maxBytes) {
    transferSize = maxBytes;
  }

  peek(pBuffer, transferSize);
  Skip(transferSize);



  return transferSize;
  
}
/*!
   Peek at the contents of the ring buffer.   
   This code returns data from the ring buffer for a consumer without
   advancing the get pointer.  This function never blocks

   \param pBuffer - Pointer to the buffer that will contain the data transferred.
   \param maxBytes- Maximum number of bytes to transfer to pBuffer

   \return size_t
   \retval Number of bytes read (could be zero, can't be larger than maxBytes).

   \throw CStateException - This object is not a consumer.

   \note if maxBytes is larger than the ring buffer size this is not an error, just
         a gaurantee that the returned value will be less than maxBytes.
*/
size_t
CRingBuffer::peek(void*   pBuffer,
		  size_t  maxBytes)
{
  validateTransferAccess(consumer, "CRingBUffer::get");

  size_t transferSize = availableData();
  if (transferSize == 0) {
    return 0;			// no data.
  }
  if (transferSize > maxBytes) {
    transferSize = maxBytes;
  }


  // Get some key values

  off_t ringBase = m_pRing->s_header.s_dataOffset;
  off_t ringTop  = m_pRing->s_header.s_topOffset;
  char* pDataBase= reinterpret_cast<char*>(m_pRing) + ringBase;
  char* pGet     = reinterpret_cast<char*>(m_pRing) + 
                   m_pClientInfo->s_offset;	// get data starting here.

  // Decide if this can be transferred in one or two chunks:

  if (m_pClientInfo->s_offset + transferSize <= (ringTop+1)) {

    // only need a single transfer:

    memcpy(pBuffer, pGet, transferSize);

  }
  else {
    // Need two chunks worth of transfer.

    size_t firstSize = ringTop+1 - m_pClientInfo->s_offset;
    size_t secondSize= transferSize - firstSize;

    memcpy(pBuffer, pGet, firstSize);
    memcpy(reinterpret_cast<char*>(pBuffer) + firstSize,
	   pDataBase, secondSize);
  }
  return transferSize;
  
}
/*!
   Skip the user's get/put pointer ahead.  In zero copy mode
   a consumer or a producer can call this because producers
   do a skip once data has been placed directly into the ring buffer
   to increment the put pointer.

   \param nBytes - Number of bytes to skip.

   \throw CRangeError     - nBytes > than the ring buffer.
   \throw CStateException - This is not a consumer object

   \note it's actually the private Skip utility that is going to throw
   CRangeError exceptions.
*/
void
CRingBuffer:: skip(size_t nBytes)
{
#ifdef NOZEROCOPY
  if (m_mode != consumer) {
    throw CStateException(modeString().c_str(), "consumer",
			  "CRingBuffer::skip");

  }
#endif
  Skip(nBytes);
}
/////////////////////////////////////////////////////////////////////////////////
// Manage the blocking latencies.

/*!
   Set the blocking poll interval (the length of the usleep) in milliseconds.

   \param newValue - Number of milliseconds between calls of the
                     blocking predicate.
   \return unsigned long
   \retval the prior value of this parameter.
*/
unsigned long
CRingBuffer::setPollInterval(unsigned long newValue)
{
  unsigned long ret = m_pollInterval;
  m_pollInterval = newValue;
  return ret;
}
/*!
    \return unsigned long
    \retval The time between calls to the blocking predicate.
*/
unsigned long
CRingBuffer::getPollInterval()
{
  return m_pollInterval;
}


/////////////////////////////////////////////////////////////////////////////
// Methods belwo provide support for (nearly) zero copy access.

/**
 * getPointer
 *   Returns the pointer into the ring (put or get) for the
 *   client.
 * @return void* - pointer to the current position in the ring.
 * @note - you'd better know what you're doing if you're accessing this
 *         as a producer... 'cause I don't..
 */
void*
CRingBuffer::getPointer()
{
  off_t ringBase = m_pRing->s_header.s_dataOffset;
  char* pDataBase= reinterpret_cast<char*>(m_pRing) + ringBase;
  char* pGet     = reinterpret_cast<char*>(m_pRing) + 
                   m_pClientInfo->s_offset;	// get data starting here.
                   
  return pGet;
}
/**
 * wouldWrap
 *   @param nBytes -number of bytes to check.
 *   @return bool - true if pGet + nBytes would wrap the buffer.
 */
bool
CRingBuffer::wouldWrap(size_t nBytes)
{
 off_t ringTop  = m_pRing->s_header.s_topOffset;
 off_t desiredTop = m_pClientInfo->s_offset + nBytes;
 return desiredTop > ringTop;
}

/**
 * @return size_t - number of bytes from get pointer to top.
 */
size_t
CRingBuffer::bytesToTop()
{
  return m_pRing->s_header.s_topOffset - m_pClientInfo->s_offset + 1;
}

///////////////////////////////////////////////////////////////////////////////
//  Inquiry member functions.


/*!
   \return size_t
   \retval the number of bytes of space available in which to put new data in
           the ring buffer.
*/
size_t
CRingBuffer::availablePutSpace()
{
  pRingHeader pHeader   = reinterpret_cast<pRingHeader>(m_pRing);
  size_t      consumers = pHeader->s_maxConsumer;

  pClientInformation pClients = reinterpret_cast<pClientInformation>(reinterpret_cast<char*>(m_pRing) + 
								      pHeader->s_firstConsumer);

  // figure out the minimum free space:

  size_t minFree =  pHeader->s_dataBytes-1;
  for (int i = 0; i < consumers;  i++) {
    // If the ring is in transition (a new consumer joining)... we need to 
    // wait a bit and try again so that we are not looking at get pointers in flux.
    //
    if (pClients->s_pid == 0) {
      Os::usleep(100);		// Wait 100usec.
      i = 0;			// and reset the loop.
      minFree  = pHeader->s_dataBytes-1;
      pClients = reinterpret_cast<pClientInformation>(reinterpret_cast<char*>(m_pRing) + 
						      pHeader->s_firstConsumer);
    }
    
    if(pClients->s_pid > 0) {	// -1  - unused 0 - initializing > 0 fully  in use.
      size_t avail     = availableData(pClients);
      size_t freeBytes = pHeader->s_dataBytes - avail - 1; // 
      if (freeBytes < minFree) minFree = freeBytes;
    }

    pClients++;
  }
  return minFree;
}


/*!
   Return the number of bytes of available data for the object represented
   by us.  This can be called for producers, as well as consumers, producers
   will always get a 0.

   \return size_t
   \retval Number of bytes a get can do without blocking.

*/
size_t
CRingBuffer::availableData() 
{

  return availableData(m_pClientInfo);

    
}


/*! 
  Get information about the usage of the ring buffer.
  This can be used in a management/diagnostic tool that determines and reports
  ring buffer utilization.
  \return CRingBuffer::Usage
  \retval Describes a snapshot of the ring buffer usage.

*/
CRingBuffer::Usage
CRingBuffer::getUsage()
{

  pRingHeader         pHead      = &(m_pRing->s_header);
  pClientInformation  pProducer  = &(m_pRing->s_producer);
  pClientInformation  pConsumers = m_pRing->s_consumers;

  Usage  result;
  result.s_bufferSpace = pHead->s_dataBytes;
  result.s_putSpace    = availablePutSpace();
  result.s_maxConsumers= pHead->s_maxConsumer;
  result.s_producer    = pProducer->s_pid;

  // Get information about all the consumers:

  for (int i =0; i < result.s_maxConsumers; i++) {
    if (pConsumers->s_pid != -1) {
      pair<pid_t, size_t> info;
      info.first  = pConsumers->s_pid;
      info.second = difference(*pProducer, *pConsumers);
      result.s_consumers.push_back(info);
    }
    pConsumers++;
  }
  // Figure out the max/min data available.
  // Special case of no consumers means that the 0 space is available for both.
  
  if (result.s_consumers.size() == 0) {
    result.s_maxGetSpace = result.s_minGetSpace = 0;
  }
  else {
    result.s_maxGetSpace = 0;
    result.s_minGetSpace = result.s_bufferSpace;
    for (int i =0; i < result.s_consumers.size(); i++) {
      if (result.s_consumers[i].second > result.s_maxGetSpace) 
	result.s_maxGetSpace = result.s_consumers[i].second;
      if (result.s_consumers[i].second < result.s_minGetSpace) 
	result.s_minGetSpace = result.s_consumers[i].second;
    }
  }
  return result;
}


/*!
  Return the client slot.
  \return off_t
  \retval  -1   - The slot is a producer, or manager.
  \retval  >= 0 - The consumer slot number.
*/
off_t
CRingBuffer::getSlot() 
{
  if (m_mode == producer) return -1;
  if (m_mode == manager)  return -1;

  for (int i = 0; i < m_pRing->s_header.s_maxConsumer; i++) {
    pClientInformation p = &(m_pRing->s_consumers[i]);
    if (p == m_pClientInfo) return i;
  }
  return -2;			// Really 'impossible'.
}

///////////////////////////////////////////////////////////////////////////////
//  Blocking functions:

/*!
    Implements a generalized blocking function.  The idea is that we will
    block the caller until some condition is no longer met.  The condition
    is captured in the form of a functional object that is derived from 
    CRingBuffer::CRingBufferPredicate.

    \param pred    - The redicate object that controls how long we block.
    \param timeout - The maximum number of seconds we'll block.  The
                     default value is ULONG_MAX which is about 136 years
		     or essentially indefinitely.  A value of 0 will
		     never block.
    \return int
    \retval 0    - Blocking ended normally.
    \retval -1   - Blocking timed out.
*/
int 
CRingBuffer::blockWhile(CRingBuffer::CRingBufferPredicate& pred, unsigned long timeout)
{
  // Lower the latencey by special casing the timeout == 0:

  if (timeout) {
    time_t start = time(NULL);
    while (pred(*this)) {
      time_t now = time(NULL);
      if ((now - start) >= timeout) 
        return -1; // timeout
      pollblock(); // wait a bit before checking condition.
    }
    
    return 0;			// condition no longer true.
  }
  else {
    return pred(*this) ? -1 : 0;
  }
}
/*!
   Iterate in some way while a predicate is true.  This can be used
   to skip forward in the ring buffer until a specific chunk of data
   is found e.g.
   \param pred   - the predicate.  The predicate is an object of class
                   derived from the abstract base class of 
                   CRingBufferPredicate.

*/
void
CRingBuffer::While(CRingBuffer::CRingBufferPredicate& pred)
{
  while(pred(*this)) 
    ;
}

/*!
  Block for the polling interval
*/
void
CRingBuffer::pollblock()
{
  if (m_pollInterval> 0) {
    Os::usleep(m_pollInterval * 1000); // wait a bit before checking condition.
  }
}

//////////////////////////////////////////////////////////////////////////////
//  Management functions:

/*!
   Force the release of the producer.  This requires that we are connected as a manager.
   
*/
void
CRingBuffer::forceProducerRelease()
{
  if (m_mode != manager) {
    throw CStateException(modeString().c_str(), "manager",
			  "CRingBuffer::forceProducerRelease");
  }
  m_pRing->s_producer.s_pid = -1;
}

/*!
  Force the release of a consumer.  This requires that we are connected as a manager.
  \param slot - the number of the slot of the consumer to release. 

 \throw CStateException - the mode is not manager.
 \trhow CRangeException - the slot is out of range.

*/
void
CRingBuffer::forceConsumerRelease(unsigned slot)
{
  if (m_mode != manager) {
    throw CStateException(modeString().c_str(), "manager",
			  "CRingBuffer::forceConsumerRelease");
  }
  if (slot >= m_pRing->s_header.s_maxConsumer) {
    throw CRangeError(0, m_pRing->s_header.s_maxConsumer -1, 
		      slot,
		      "CRingBuffer::forceConsumerRelease");
  }
  m_pRing->s_consumers[slot].s_pid = -1;
}

//////////////////////////////////////////////////////////////////////////////
// Private utility functions

/*******************************************************************/
/*  Return the amount of data available for a single client.       */
/******************************************************************/

size_t
CRingBuffer::availableData(ClientInformation* pInfo)
{
  // Locate the producer client information.

  pRingHeader pHeader          = reinterpret_cast<pRingHeader>(m_pRing);
  pClientInformation pProducer = reinterpret_cast<pClientInformation>(reinterpret_cast<char*>(m_pRing) + 
								       pHeader->s_producerInfo);

  return difference(*pProducer, *pInfo);
  
}

/*******************************************************************/
/*  Construct the full device special file for a shared memory     */
/*  segment given the 'simple' name. This just prepends a '/' to   */
/*  the simple name                                                */
/*******************************************************************/
string
CRingBuffer::shmName(string rawName)
{
  return string("/") + rawName;
}

/*******************************************************************/
/*  Maps to the specified full name ring buffer.                   */
/*******************************************************************/

RingBuffer* 
CRingBuffer::mapRingBuffer(std::string fullName)
{



  return reinterpret_cast<RingBuffer*>(CDAQShm::attach(fullName));

}


/******************************************************************/
/* Unmap the ring buffer.                                         */
/******************************************************************/

void
CRingBuffer::unMapRing()
{
  std::string fullName = shmName(m_ringName);
  CDAQShm::detach(m_pRing, fullName, CDAQShm::size(fullName));
  
}
/******************************************************************/
/* Allocate an unused consumer description record.  On success    */
/* the consumer is filled in with our pid, and an offset that's   */
/* equal to the put pointer.  On failure a CErrnoException is     */
/* thrown that with ENOMEM as the reason, since the only error    */
/* is for there to be no free consumer blocks.                    */
/******************************************************************/ 
void
CRingBuffer::allocateConsumer()
{
  pRingHeader pHeader    = &(m_pRing->s_header);
  size_t      nConsumers = pHeader->s_maxConsumer;
  pClientInformation p   = reinterpret_cast<pClientInformation>(reinterpret_cast<char*>(m_pRing) + 
							   pHeader->s_firstConsumer);
  pClientInformation put= reinterpret_cast<pClientInformation>(reinterpret_cast<char*>(m_pRing) +
							   pHeader->s_producerInfo);

  for (int i =0; i < nConsumers; i++) {
    if (p->s_pid == -1) {
      p->s_pid = 0;		// Claim it as in use but not active.
      __sync_synchronize();	// Flush to shm as well.

      // The loop below deals with any cases where the put pointer moved
      // While we were joining up.

      while(p->s_offset != put->s_offset) {
	p->s_offset = put->s_offset;
	__sync_synchronize();
      }

      p->s_pid = getpid();	   // now unstall any free space computations
      m_pClientInfo = p;
      __sync_synchronize();	// Flush to shm as well.
      return;
    }

    p++;
  }
  errno = ENOMEM;
  throw CErrnoException("CRingBuffer::allocateConsumer");

}
/******************************************************************/
/*  Compute the 'space' between two pointers, dealing with wraps. */
/*  The first pointer is assumed to be ahead of the second.       */
/******************************************************************/

size_t
CRingBuffer::difference(ClientInformation& producer, ClientInformation& consumer)
{
  // If the producer is bigger than the consumer it's a simple difference:

  if (producer.s_offset >= consumer.s_offset) {
    return producer.s_offset - consumer.s_offset;
  }
  // Otherwise, the answer is the sum of how far the consumer offset is from the
  // top of the ring and how far the producer is from the bottom.
  // +1.
  pRingHeader pHeader = &(m_pRing->s_header);

  // The + 1 below.. If my offset is right at the top,
  // I have one byte..the byte I point to but the difference is 0.

  size_t topSize = pHeader->s_topOffset - consumer.s_offset +1 ;
  size_t botSize = producer.s_offset   - pHeader->s_dataOffset;
  return topSize + botSize;
}

/******************************************************************/
/* Move the object's pointer ahead the designated number of bytes */
/* wrapping if needed.                                            */
/******************************************************************/
void
CRingBuffer::Skip(size_t nBytes)
{
  pRingHeader pHeader = &(m_pRing->s_header);

  m_pClientInfo->s_offset += nBytes;
  if (m_pClientInfo->s_offset > pHeader->s_topOffset) {
    m_pClientInfo->s_offset = (m_pClientInfo->s_offset - pHeader->s_topOffset) +
                              pHeader->s_dataOffset - 1;
  }
  // Issue a memory barrier to ensure this is flushed out to the shared memory?

  __sync_synchronize();
}
/***************************************************************/
/* Return the stringified mode                                 */
/**************************************************************/
string
CRingBuffer::modeString() const
{
  switch (m_mode) {
  case consumer:
    return string("consumer");
  case producer:
    return string("producer");
  case manager:
    return string("manager");
  default:
    return string("*INVALID MODE*");
  }

}
/*****************************************************************/
/* Connect to the ring master if we are not already connected    */
/*****************************************************************/

void CRingBuffer::connectToRingMaster()
{
  pid_t pid = getpid();
  if (m_pMaster && (pid == m_myPid)) {
    return;		  // Only connect once per process, at the class level.
  }
  m_myPid  = pid;
  m_pMaster = new CRingMaster;
  
}
/***********************************************************************/
/*  notify the ring master we are connecting to a ring.  We send the   */
/*  CONNECT ringname type  pid {RingBuffer client connection}          */
/*  message to the ring master and expect the OK result back.          */
/*  type depends on the consumer/producer-ness of the connection.      */
/*  consumers include a consumer slot number on the type.              */
/*  Faiulure to communicate results in errno exceptions being thrown   */
/***********************************************************************/

void
CRingBuffer::notifyConnection()
{
  if (m_mode == producer) {
    m_pMaster->notifyProducerConnection(m_ringName, "CRingBuffer:: client connection");
  }
  else {
    m_pMaster->notifyConsumerConnection(m_ringName, getSlot(), 
				      "CRingBuffer:: client connection");
  }
}

/**********************************************************************/
/* Notify the ring master that we are disconnecting from a ring.  We  */
/* Send the message:                                                  */
/* DISCONNECT ringname connecttype pid                                */
/* And expect "OK\r\n" in response.                                     */
/**********************************************************************/

void
CRingBuffer::notifyDisconnection()
{


  if (m_mode == producer) {
    m_pMaster->notifyProducerDisconnection(m_ringName);
  }
  else {
    m_pMaster->notifyConsumerDisconnection(m_ringName, getSlot());
  }
}
/**********************************************************************/
/* Return true if the pointer passed in points to a ring header       */
/**********************************************************************/
bool
CRingBuffer::ringHeader(RingBuffer* p)
{
return strcmp(p->s_header.s_magicString, 
	       MAGICSTRING) == 0;
}
/**
 * validateTransferAccess
 *    For a process to access ring data access/transfer
 *    methods requires that this object be attached with an appropriate
 *    mode and that the pid associated with our header be our own.
 *    If that's not the case exceptions get thrown.
 *   @param mode  - required access mode.
 *   @param doing - what's being attempted that requires access.
 */
void
CRingBuffer::validateTransferAccess(ClientMode mode, const char* doing)
{
if (m_mode != mode) {
    throw CStateException(modeString().c_str(), "consumer",
			  doing);
  }
  // Ensure our PID matches the consumer struct for us.
  // note that the class variable m_myPid must be set to our PID because
  // to connect to this ring we must have contacted the RingMaster and that's
  // what sets that.

  if(m_myPid != m_pClientInfo->s_pid) {
    throw CStateException("My PID", "Someone else's pid",
			  doing);
  }
}