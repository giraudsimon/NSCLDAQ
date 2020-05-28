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
#include <config.h>
#include <openssl/evp.h>
#include "eventlogMain.h"
#include "eventlogargs.h"

#include "RingChunk.h"

#include <CRingBuffer.h>


#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <CRemoteAccess.h>
#include <DataFormat.h>
#include <CAllButPredicate.h>
#include <CRingItemFactory.h>
#include <io.h>
#include "CZCopyRingBuffer.h"

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <system_error>

using std::string;
using std::cerr;
using std::endl;
using std::cout;

// constant definitions.

static const uint64_t K(1024);
static const uint64_t M(K*K);
static const uint64_t G(K*M);

static const int RING_TIMEOUT(5);	// seconds in timeout for end of run segments...need no data in that time.

static const size_t BUFFERSIZE(M);   // ZFS Blocksize - let's write blocks that size:


///////////////////////////////////////////////////////////////////////////////////
// Local classes:
//

class noData :  public CRingBuffer::CRingBufferPredicate
 {
 public:
   virtual bool operator()(CRingBuffer& ring) {
     return (ring.availableData() == 0);
   }
 };

 ///////////////////////////////////////////////////////////////////////////////////
 //
 // Constructor and destructor  are basically no-ops:

 EventLogMain::EventLogMain() :
   m_pRing(0),
   m_eventDirectory(string(".")),
   m_segmentSize((static_cast<uint64_t>(1.9*G))),
   m_exitOnEndRun(false),
   m_nSourceCount(1),
   m_fRunNumberOverride(false),
   m_pChecksumContext(0),
   m_nBeginsSeen(0),
   m_fChangeRunOk(false),
   m_prefix("run"),
   m_pItem(nullptr),
   m_nItemSize(0),
   m_pChunker(0)
 {
 }

 EventLogMain::~EventLogMain()
 {}
 //////////////////////////////////////////////////////////////////////////////////
 //
 // Object member functions:
 //


 /*!
    Entry point is pretty simple, parse the arguments, 
    Record the data
 */
 int
 EventLogMain::operator()(int argc, char**argv)
 {
   parseArguments(argc, argv);

   try {
    recordData();
   }
   catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
   }


   return 0;
 }

 ///////////////////////////////////////////////////////////////////////////////////
 //
 // Utility functions...well really this is where all the action is.
 //



 /*
 ** Open an event segment.  Event segment filenames are of the form
 **   run-runnumber-segment.evt
 **
 ** runnumber - the run number. in %04d
 ** segment   - The run ssegment in %02d
 **
 ** Note that all files are stored in the directory pointed to by
 ** m_eventDirectory.
 **
 ** Parameters:
 **     runNumber   - The run number.
 **     segment     - The segment number.
 **
 ** Returns:
 **     The file descriptor or exits with an error if the file could not be opened.
 **
 */
 int
 EventLogMain::openEventSegment(uint32_t runNumber, unsigned int segment)
 {
   // Create the filename:

   string fullPath  = m_eventDirectory;

   char nameString[1000];
   sprintf(nameString, "/%s-%04d-%02d.evt", m_prefix.c_str(), runNumber, segment);
   fullPath += nameString;

   int fd = open(fullPath.c_str(), O_RDWR | O_CREAT | O_EXCL, 
		 S_IWUSR | S_IRUSR | S_IRGRP);
   if (fd == -1) {
     perror("Open failed for event file segment"); 
     exit(EXIT_FAILURE);
   }
   // Try to pre-allocate the file - it's not fatal if we can't might not
   // be enough disk space yet but the file may not fill it; or the
   // underlying file system might just not support it.
   
   int status = fallocate(fd, FALLOC_FL_KEEP_SIZE, 0, m_segmentSize);
   if (status) {
    std::cerr << "Failed to preallocate event segment " << nameString
      << " " << strerror(errno) << std::endl;
    std::cerr << " Continuing to record data\n";
    
   }
   m_pChunker->setFd(fd);
   return fd;

 } 


 /*
 **
 ** Record the data.  We're ready to roll having verified that
 ** we can open an event file and write data into it, the ring is also 
 ** open.
 */
 void
 EventLogMain::recordData()
 {
   // if we are in one shot mode, indicate to whoever started us that we are ready to
   // roll.  That file goes in the event directory so that we don' thave to keep hunting
   // for it like we did in ye olde version of NSCLDAQ:

   if (m_exitOnEndRun) {
     string startedFile = m_eventDirectory;
     startedFile += "/.started";
     int fd = open(startedFile.c_str(), O_WRONLY | O_CREAT,
		   S_IRWXU );
     if (fd == -1) {
       perror("Could not open the .started file");
       exit(EXIT_FAILURE);
     }

     close(fd);
   }

   // Now we need to hunt for the BEGIN_RUN item...however if there's a run
   // number override we just use that run number unconditionally.

   bool warned = false;
   CRingItem* pItem;
   CRingItem* pFormatItem(0);
   CAllButPredicate all;

   // Loop over all runs.

   while(1) {

     // If necessary, hunt for the begin run.

     if (!m_fRunNumberOverride) {
        while (1) {
          pItem = CRingItem::getFromRing(*m_pRing, all);
          
          /*
            As of NSCLDAQ-11 it is possible for the item just before a begin run
            to be one or more ring format items.
          */
          
          if (pItem->type() == RING_FORMAT) {
            pFormatItem = pItem;
          } else if (pItem->type() == BEGIN_RUN) {
            break;
          } else {
            // If not a begin or a ring_item we're tossing it out.
            delete pItem;
            pItem = 0;
          }
          
          if (!warned && !pFormatItem) {
            warned = true;
            cerr << "**Warning - first item received was not a begin run. Skipping until we get one\n";
          }
        }
       
       // Now we have the begin run item; and potentially the ring format item
       // too. Alternatively we have been told the run number on the command line.
       
       CRingStateChangeItem item(*pItem);
       recordRun(item, pFormatItem);
       delete pFormatItem;    // delete 0 is a no-op.
       delete pItem;
       pFormatItem = 0;
       
       
     } else {
      //
      // Run number is overidden we don't need a state change item.  Could,
      // for example, be a non NSCLDAQ system or a system without
      // State change items.
      //
       recordRun(*(reinterpret_cast<const CRingStateChangeItem*>(0)), 0);
     }
     // Return/exit after making our .exited file if this is a one-shot.

     if (m_exitOnEndRun) {
       string exitedFile = m_eventDirectory;
       exitedFile       += "/.exited";
       int fd = open(exitedFile.c_str(), O_WRONLY | O_CREAT,
		     S_IRWXU);
       if (fd == -1) {
        perror("Could not open .exited file");
        exit(EXIT_FAILURE);
        return;
       }
       close(fd);
       return;
     }


   }



 }

 /*
 ** Record a run to disk.  This must
 ** - open the initial event file segment
 ** - Write items to the segment keeping track of the segment size,
 **   opening new segments as needed until: 
 ** - The end run item is gotten at which point the run ends.
 **
 ** @param item - The state change item.
 ** @param pFormatitem - possibly null pointer, if not null this points to the
 **                      ring format item that just precedes the begin run.
 **                      
 */
 void
 EventLogMain::recordRun(const CRingStateChangeItem& item, CRingItem* pFormatItem)
 {
   unsigned int segment        = 0;
   uint32_t     runNumber;
   uint64_t     bytesInSegment = 0;
   int          fd;
   unsigned     endsRemaining  = m_nSourceCount;
   CAllButPredicate p;

   CRingItem*   pItem;   // Going to use our expanding storage at m_pItem.
   uint16_t     itemType;
   
   


   // Figure out what file to open and how to set the pItem:
    
    if (m_fRunNumberOverride) {
      runNumber  = m_nOverrideRunNumber;
      fd         = openEventSegment(runNumber, segment);
      pItem      = CRingItem::getFromRing(*m_pRing, p);
    } else {
      runNumber  = item.getRunNumber();
      fd         = openEventSegment(runNumber, segment);
      pItem      = new CRingStateChangeItem(item);
    }
    if (pItem->type() == BEGIN_RUN) {
      std::cerr << "Begin run in recordRun \n";
      m_nBeginsSeen++;
    }

    // If there is a format item, write it out to file:
    // Note there won't be if the run number has been overridden.
    
    if (pFormatItem) {
      bytesInSegment += itemSize(*pFormatItem);
      writeItem(fd, *pFormatItem);
 
    }
    writeItem(fd, *pItem);
    bytesInSegment += itemSize(*pItem);
    
    writeInterior(fd, runNumber, bytesInSegment);    // Write the rest of the run.
    
    // Note that when writeInterior returns the last event segment is
    // closed.
    
    // If requested, write the checksum file:
    
    if (m_pChecksumContext) {
      EVP_MD_CTX* pCtx = reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext);
       unsigned char* pDigest = reinterpret_cast<unsigned char*>(OPENSSL_malloc(EVP_MD_size(EVP_sha512())));
       unsigned int   len;
         
       // Not quite sure what to do if pDigest failed to malloc...for now
       // silently ignore...
  
      if (pDigest) {
         EVP_DigestFinal_ex(pCtx, pDigest, &len);
         std::string digestFilename = shaFile(runNumber);
         FILE* shafp = fopen(digestFilename.c_str(), "w");
  
       
         // Again not quite sure what to do if the open failed.
         if (shafp) {
          unsigned char* p = pDigest;
          for (int i =0; i < len;i++) {
            fprintf(shafp, "%02x", *p++);
          }
          fprintf(shafp, "\n");
          fclose(shafp);
        }
         // Release the digest storage and the context.
        OPENSSL_free(pDigest);
  
      }
      EVP_MD_CTX_destroy(pCtx);
      m_pChecksumContext = 0;
       
    }  
    return;

 }

 /*
 ** Parse the command line arguments, stuff them where they need to be
 ** and check them for validity:
 ** - The ring must exist and be open-able.
 ** - The Event segment size, if supplied must decode properly.
 ** - The Event directory must be writable.
 **
 ** Parameters:
 **   argc  - Count of command words.
 **   argv  - Array of pointers to the command words.
 */
 void
 EventLogMain::parseArguments(int argc, char** argv)
 {
   gengetopt_args_info parsed;
   cmdline_parser(argc, argv, &parsed);

   // Figure out where we're getting data from:

   string ringUrl = defaultRingUrl();
   if(parsed.source_given) {
     ringUrl = parsed.source_arg;
   }
   // Figure out the event directory:

   if (parsed.path_given) {
     m_eventDirectory = string(parsed.path_arg);
   }

   if (parsed.oneshot_given) {
     m_exitOnEndRun = true;
   }
   if (parsed.run_given && !parsed.oneshot_given) {
     std::cerr << "--oneshot is required to specify --run\n";
     exit(EXIT_FAILURE);
   }
   if (parsed.run_given) {
     m_fRunNumberOverride = true;
     m_nOverrideRunNumber = parsed.run_arg;
   }
   // And the segment size:

   if (parsed.segmentsize_given) {
     m_segmentSize = segmentSize(parsed.segmentsize_arg);
   }

   m_nSourceCount = parsed.number_of_sources_arg;

   // The directory must be writable:

   if (!dirOk(m_eventDirectory)) {
     cerr << m_eventDirectory 
	  << " must be an existing directory and writable so event files can be created"
	  << endl;
     exit(EXIT_FAILURE);
   }

   if (parsed.prefix_given) {
    m_prefix = parsed.prefix_arg;
   }

   // And the ring must open:

   try {
     m_pRing = CRingAccess::daqConsumeFrom(ringUrl);
     m_pRing->setPollInterval(0);
   }
   catch (...) {
     cerr << "Could not open the data source: " << ringUrl << endl;
     exit(EXIT_FAILURE);
   }
   // Checksum flag:

   m_fChecksum = (parsed.checksum_flag != 0);
   m_fChangeRunOk = (parsed.combine_runs_flag != 0);
   
   m_pChunker = new CRingChunk(m_pRing, m_fChangeRunOk);

 }

 /*
 ** Return the default URL for the ring.
 ** this is tcp://localhost/username  where
 ** username is the name of the account running the program.
 */
 string
 EventLogMain::defaultRingUrl() const
 {
   return CRingBuffer::defaultRingUrl();

 }

 /*
 ** Given a segement size string either returns the size of the segment in bytes
 ** or exits with an error message.
 **
 ** The form of the string can be one of:
 **    number   - The number of bytes.
 **    numberK  -  The number of kilobytes.
 **    numberM  - The number of megabytes.
 **    numberG  - The number o gigabytes.
 */
 uint64_t
 EventLogMain::segmentSize(const char* pValue) const
 {
   char* end;
   uint64_t size = strtoull(pValue, &end, 0);

   // The remaning string must be 0 or 1 chars long:

   if (strlen(end) < 2) {

     // If there's a multiplier:

     if(strlen(end) == 1) {
       if    (*end == 'g') {
	 size *= G;
       } 
       else if (*end == 'm') {
	 size *= M;
       }
       else if (*end == 'k') {
	 size *= K;
       }
       else {
	 cerr << "Segment size multipliers must be one of g, m, or k" << endl;
	 exit(EXIT_FAILURE);
       }

     }
     // Size must not be zero:

     if (size == (uint64_t)0) {
       cerr << "Segment size must not be zero!!" << endl;
     }
     return size;
   }
   // Some conversion problem:

   cerr << "Segment sizes must be an integer, or an integer followed by g, m, or k\n";
   exit(EXIT_FAILURE);
 }

 /*
 ** Determine if a path is suitable for use as an event directory.
 ** - The path must be to a directory.
 ** - The directory must be writable by us.
 **
 ** Parameters:
 **  dirname - name of the proposed directory.
 ** Returns:
 **  true    - Path ok.
 **  false   - Path is bad.
 */
 bool
 EventLogMain::dirOk(string dirname) const
 {
   struct stat statinfo;
   int  s = stat(dirname.c_str(), &statinfo);
   if (s) return false;		// If we can't even stat it that's bad.

   mode_t mode = statinfo.st_mode;
   if (!S_ISDIR(mode)) return false; // Must be a directory.

   return !access(dirname.c_str(), W_OK | X_OK);
 }
 /**
  * dataTimeout
  *
  *  Determine if there's a data timeout.  A data timeout occurs when no data comes
  * from the ring for RING_TIMEOUT seconds.  This is used to detect missing
  * end segments in the ring (e.g. run ending because a source died).
  */
 bool
 EventLogMain::dataTimeout()
 {
   noData predicate;

   m_pRing->blockWhile(predicate, RING_TIMEOUT);
   return (m_pRing->availableData() == 0);
 }
/**
 * writeItem
 *   Write a ring item.
 *
 *   @param fd - File descriptor open on the output file.
 *   @param item - Reference to the ring item.
 *
 * @throw  uses CBufferedOutput::put which throws errs.
 *         The errors are caught described and we exit :-(
 */
void
EventLogMain::writeItem(int fd, CRingItem& item)
{
    try {
      void*    pItem = item.getItemPointer();
      uint32_t nBytes= itemSize(item);

      // If necessary create the checksum context
      // If checksumming add the ring item to the sum.

      if (m_fChecksum) {
        if (!m_pChecksumContext) {
          m_pChecksumContext = EVP_MD_CTX_create();
          if (!m_pChecksumContext) throw errno;
          if(EVP_DigestInit_ex(
              reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext), EVP_sha512(), NULL) != 1) {
            EVP_MD_CTX_destroy(reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext));
            m_pChecksumContext = 0;
            throw std::string("Unable to initialize the checksum digest");
          }
      
        }
        EVP_DigestUpdate(
               reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext), pItem, nBytes);
      }
      io::writeData(fd, pItem, nBytes);
      //io::writeData(fd, pItem, nBytes);
    }
    catch(int err) {
      if(err) {
        cerr << "Unable to output a ringbuffer item : "  << strerror(err) << endl;
      }  else {
        cerr << "Output file closed out from underneath us\n";
      }
      exit(EXIT_FAILURE);
    }
    catch (std::string e) {
      std::cerr << e << std::endl;
      exit(EXIT_FAILURE);
    }
}
/**
* itemSize
*    Return the number of bytes in a ring item.
* @param item - reference to a ring item.
*
* @return size_t -size of the item.
*/
size_t
EventLogMain::itemSize(CRingItem& item) const
{
    return ::itemSize(reinterpret_cast<pRingItem>(item.getItemPointer()));
}
/**
 * shaFile
 *    Compute the filename for the checksum for a run.
 *
 * @param run - Run number
 * 
 * @return std::string - the filename.
 */
std::string
EventLogMain::shaFile(int run) const
{
  char runNumber[100];
  sprintf(runNumber, "%04d", run);

  std::string fileName = m_eventDirectory;
  fileName+= ("/" + m_prefix + "-");
  fileName+= runNumber;
  fileName += ".sha512";

  return fileName;
}




/**
 * writeInterior
 *   Writes the interior of a run file.   The run file prefix
 *   (potentially format item(s) and the first begin run item) have already
 *   been written to an open event segment.  We're going to do a copy free
 *   write of the remainder of the run.  We'll do this by getting the get
 *   pointer from the ring buffer and accumulating a contiguous chunk of
 *   data to write that's terminated by one of the following:
 *   - A BEGIN or END run element.
 *   - An abnormal End element
 *   - an item that's wrapped across the ring item top/bottom.
 *   - An item that terminates exactly on the top of the ring buffer.
 *   We'll do the write of that chunk of data in one full swat.  This write
 *   is therefore copy free.  The ending cases are then handled as follows:
 *   -  BEGIN/END - write what we have, through the item.  Tally the number of
 *      begins and ends we've seen so far.  When we hit zero we're done with the
 *      run.
 *   -  Abnormal end - write through the item and we're done.
 *   -  Wrap - write up to the wrapped item, get the wrapped item and write it.
 *   -  End of ring buffer - write through the item.
 *
 *   In this way, we should be able to essentially do a zero copy write of
 *   the event data.  The only exception begin the wrapped items which,
 *   for sipmlicity are copied int a contiguous item and then written from the
 *   copy.
 *
 *   Furthermore, at high rates our writes should be pretty big which is good
 *   for reducing overhead.
 *
 * @param fd - file descriptor open on the first run file segment.
 * @param runNumber - number of the run being recorded.
 * @param bytesSoFar - number of bytes written to the segment so far.
 * 
 */
void
EventLogMain::writeInterior(int fd, uint32_t runNumber, uint64_t bytesSoFar)
{
  m_nRunNumber = runNumber;
  m_pChunker->setRunNumber(runNumber);
  
  int endsSeen(0);
  Chunk nextChunk;
  int segno = 0;
  while(1) {
    waitForLotsOfData();                  // Wait for a lot of data or timeout.
    
    // Get a contigous chunk.
    
    m_pChunker->getChunk( nextChunk);
    m_nBeginsSeen += nextChunk.s_nBegins;
    endsSeen   += nextChunk.s_nEnds;
    if(nextChunk.s_nBytes)  {
      writeData(fd, nextChunk.s_pStart, nextChunk.s_nBytes);
      m_pRing->skip(nextChunk.s_nBytes);
      bytesSoFar += nextChunk.s_nBytes;
      if (bytesSoFar >= m_segmentSize) {
        m_pChunker->closeEventSegment();
        fd = openEventSegment(runNumber, ++segno);
        bytesSoFar  = 0;
      }
    }


    // See if we've got a balanced set of begins/ends:
    
    if(endsSeen >= m_nBeginsSeen) {
      m_pChunker->closeEventSegment();
      return;                      // The run is recorded.
    } else if (endsSeen && dataTimeout()) {
      
      // If we time out on data, then end abnormally:
      
      m_pChunker->closeEventSegment();
      std::cerr << " Timed out with " << m_nBeginsSeen - endsSeen
        << " ends still not seen\n";
      return;
    }
    if (m_pChunker->nextItemWraps()) {                               // Note we just let the next chunk
      bytesSoFar += writeWrappedItem(fd, endsSeen);     // see if we need to open a new segment
    }
  }
}
/**
 *  WaitForLotsOfData
 *      Wait for lots of data to be available on a ring buffer.
 *      - Lots of data is defined as a 2*M bytes.
 *      - We wait at most one second.
 *      - We set the poll interval down to 1ms from its default.
 */
void
EventLogMain::waitForLotsOfData()
{
  class DataAvailPredicate : public CRingBuffer::CRingBufferPredicate {
    size_t nBytesRequired;
  public:
    DataAvailPredicate(size_t n) : nBytesRequired(n) {}
    bool operator()(CRingBuffer& ring) {
      return nBytesRequired > ring.availableData();
    }
  };
  
  DataAvailPredicate p(2*M);
  unsigned long priorPollInterval = m_pRing->setPollInterval(0);
  m_pRing->blockWhile(p, 1);
  m_pRing->setPollInterval(priorPollInterval);
}


/**
 * writeWrappedItem
 *    Assuming the next item in the ring buffer wraps, we write it.
 *    This is done by first waiting to be sure we have a uin32_t we then
 *    peek that to get the ring item size.  Then we create a block of data
 *    for the full ring item, get it from the ring and
 *    write it to the output.  We update m_nBeginsSeen and the ends seen counter.
 *
 *  @param fd - file descriptor to which the write is done.
 *  @param[inout] nEnds - references the end counter - incremented if the item is an end.
 *  @note m_nBeginsSeen is incremented if the item is a begin.
 *  @return  number of bytes of data written.
 */

size_t
EventLogMain::writeWrappedItem(int fd, int& ends)
{
  m_pChunker->waitForData(sizeof(uint32_t));
  uint32_t nSize;
  m_pRing->peek(&nSize, sizeof(uint32_t));
  
  uint8_t buffer[nSize];
  pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(buffer);
  
  m_pRing->get(buffer, nSize, nSize);
  if (pH->s_type == BEGIN_RUN) {
    m_nBeginsSeen++;
    if(badBegin(pH)) {
        m_pChunker->closeEventSegment();
        std::cerr << " Begin run changed run number without --combine-runs "
          << " or too many begin runs for the data source count\n";
        exit(EXIT_FAILURE);
    }
  }
  if (pH->s_type == END_RUN)   ends++;
  
  writeData(fd, buffer, nSize);
  return nSize;
}
/**
 * writeData
 *    Writes data to the file.  If checksumming is enabled the checksum
 *    is updated.
 *
 *  @param pData - pointer to the data to write.
 *  @param nBytes - Number of bytes of data to write.
 */
void
EventLogMain::writeData(int fd, void* pData, size_t nBytes)
{
  uint8_t* p = static_cast<uint8_t*>(pData);
  size_t  nLeft = nBytes;
  while (nLeft > BUFFERSIZE) {
    io::writeData(fd, p, BUFFERSIZE);
    nLeft -= BUFFERSIZE;
    p     += BUFFERSIZE;
  }
  
  // Last partial buffer write:
  
  if (nLeft) {
    io::writeData(fd, p, nLeft);
  }
  
  if (m_fChecksum) {
    checksumData(pData,nBytes);
  }
}
/**
 * checksumData
 *    update the sha512 hash of the data:
 *
 * @param pData -pointer to the new data.
 * @param nBytes - number of bytes of new data.
 */
void
EventLogMain::checksumData(void* pData, size_t nBytes)
{
  // If we need to make the checksum context:
  
  if(!m_pChecksumContext) {
    m_pChecksumContext = EVP_MD_CTX_create();
    if (!m_pChecksumContext) {
      throw std::system_error(
        errno, std::generic_category(), "Allocating sha512 context"
      );
    }
    // Make the sha512 digest in the context:
    
    if(EVP_DigestInit_ex(
	      reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext), EVP_sha512(), NULL) != 1) {
	    EVP_MD_CTX_destroy(reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext));
	    m_pChecksumContext = 0;
      throw std::string("Failed to initialize the sha512 digest");
    }
    
  }
  // Now we can compute the checksum to update the chunk we have:
  
  EVP_DigestUpdate(
    reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext),
    pData, nBytes
  );
    
}

/**
 * badBegin
 *    @param p - pointer to a state transition item with type BEGIN_RUN
 *    @return bool - true if the begin_run item indicates a  problem.
 */
bool
EventLogMain::badBegin(void* p)
{
  pStateChangeItem pItem = static_cast<pStateChangeItem>(p);
  
  // If run changes are allowed or we're not exiting on end of run
  // this is always ok:
  
  if (m_fChangeRunOk || (!m_exitOnEndRun)) {
    return false;
  }

  // If we've got too many begins that's a problem.  The caller has
  // already incremented the begin run counter.
  
  if (m_nBeginsSeen > m_nSourceCount) return true;
  
  // If the run number changed that's also bod:
  
  pStateChangeItemBody pBody =
    reinterpret_cast<pStateChangeItemBody>(m_pChunker->getBody(pItem));
  
  return (pBody->s_runNumber != m_nRunNumber);
  
  
}
