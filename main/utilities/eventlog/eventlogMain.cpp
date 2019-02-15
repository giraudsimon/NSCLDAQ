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



#include <CRingBuffer.h>


#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <CRemoteAccess.h>
#include <DataFormat.h>
#include <CAllButPredicate.h>
#include <CRingItemFactory.h>
#include <io.h>
#include "CBufferedRingItemConsumer.h"

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


using std::string;
using std::cerr;
using std::endl;
using std::cout;

// constant defintitions.

static const uint64_t K(1024);
static const uint64_t M(K*K);
static const uint64_t G(K*M);

static const int RING_TIMEOUT(5);	// seconds in timeout for end of run segments...need no data in that time.

static const size_t BUFFERSIZE(10*M);
static const int MAXDATASLEEP(1000*50);   // max microseconds in waitForData.

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
   m_pOutputter(nullptr),
   m_pItem(nullptr),
   m_nItemSize(0)
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
   recordData();

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

   int fd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 
		 S_IWUSR | S_IRUSR | S_IRGRP);
   if (fd == -1) {
     perror("Open failed for event file segment"); 
     exit(EXIT_FAILURE);
   }
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
	   m_nBeginsSeen = 1;
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
    m_pOutputter = new io::CBufferedOutput(fd, BUFFERSIZE);
    m_pOutputter->setTimeout(2);       // Flush after 2 seconds and write.

    // If there is a format item, write it out to file:
    // Note there won't be if the run number has been overridden.
    
    if (pFormatItem) {
      bytesInSegment += itemSize(*pFormatItem);
      writeItem(fd, *pFormatItem);
 
    }

    copyRingItem(*pItem);
    delete pItem;
    // There's a small memory leak here;   copyRingItem will
    // allocated m_pItem to hold it.  But from then on we use the
    // dataSource to just get pointers into its buffer without
    // ever freeing that dynamic storage.
    // At the end of the run we just set m_pItem to nullptr
    // Problem is that there's not really a good way to know how to
    // delete that storage.
    
    CBufferedRingItemConsumer dataSource(*m_pRing);
    
    while(1) {

      size_t size    = m_pItem->s_size;
      itemType       = m_pItem->s_type;;
 
      // If necessary, close this segment and open a new one:
 
      if ( (bytesInSegment + size) > m_segmentSize) {
       m_pOutputter->flush();
       delete m_pOutputter;
       m_pOutputter = nullptr;
       close(fd);
       segment++;
       bytesInSegment = 0;
 
       fd = openEventSegment(runNumber, segment);
       m_pOutputter = new io::CBufferedOutput(fd, BUFFERSIZE);
       m_pOutputter->setTimeout(2);
      }

      writeItem();
 
      bytesInSegment  += size;
 
 
      if(itemType == END_RUN) {
        endsRemaining--;
        if (endsRemaining == 0) {
          break;
        }
      }
      if (itemType == ABNORMAL_ENDRUN) {
         endsRemaining = 0;             // In case we're not --one-shot
         break;                         // unconditionally ends the run.
      }

      // If we've seen an end of run, need to support timing out
      // if we dont see them all.
 
      if ((endsRemaining != m_nSourceCount) && dataTimeout()) {
          cerr << "Timed out waiting for end of runs. Need " << endsRemaining 
            << " out of " << m_nSourceCount << " sources still\n";
          cerr << "Closing the run\n";
 
        break;
      }
      m_pItem = static_cast<pRingItemHeader>(dataSource.get());   // the first 
      if(isBadItem(runNumber)) {
        std::cerr << "Eventlog: Data indicates probably the run ended in error exiting\n";
        exit(EXIT_FAILURE);
      }
    } 
    //
    //  If checksumming, finalize the checksum and write out the checksum file as well.
    //  by  now m_pChecksumContext is only set if m_fChecksum was true when the run
    //  files were opened.
    //
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
    m_pOutputter->flush();
    delete m_pOutputter;
    m_pOutputter =nullptr;
    close(fd);
    m_pItem = nullptr;    

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
   }
   catch (...) {
     cerr << "Could not open the data source: " << ringUrl << endl;
     exit(EXIT_FAILURE);
   }
   // Checksum flag:

   m_fChecksum = (parsed.checksum_flag != 0);
   m_fChangeRunOk = (parsed.combine_runs_flag != 0);

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
      m_pOutputter->put(pItem, nBytes);
      // io::writeData(fd, pItem, nBytes);
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
 * isBadItem
 *     This method is called to determine if we've gotten a ring item that
 *      might indicate we need to exit in --one-shot mode:
 *      -  If --combine-runs is set, or --one-shot not set, return false.
 *      -  If the run number changed from the one we are recording,
 *         true.
 *      -  If we had more begin runs than m_nSourceCount, true
 *      -  None of these, return false.
 * @note  The item is pointed to by m_pItem.
 * @param runNumber - the current run number.
 *
 * @return bool
 * @retval true  - there's something fishy about this -- probably we should exit.
 * @retval false - as near as we can tell everything is ok.
 */
bool
EventLogMain::isBadItem(int runNumber)
{

  // For some states of program options we just don't care about the
  // data

  if (m_fChangeRunOk || (!m_exitOnEndRun)) {
    return false;
  }
  // For the rest we only care about state changes -- begins in fact

  if (m_pItem->s_type == BEGIN_RUN) {
    m_nBeginsSeen++;
    if (m_nBeginsSeen > m_nSourceCount) {
      return true;
    }
    CRingStateChangeItem* begin =
      reinterpret_cast<CRingStateChangeItem*>(CRingItemFactory::createRingItem(m_pItem));
    if (begin->getRunNumber() != runNumber) {
      delete begin;
      return true;
    }
    delete begin;
  }
  return false;

}
/**
 * getFromRing
 *    Get the next raw ring item from the ringbuffer.
 *    - Wait until there's at least a unit32_t.
 *    - Peek the size of the item.
 *    - ensure the buffer is big enough
 *    - Wait unilt there's at least the full ring item.
 *    - get the data from the ring.
 */
void
EventLogMain::getFromRing()
{
  waitForData(sizeof(uint32_t));
  uint32_t size;
  m_pRing->peek(&size, sizeof(uint32_t));
  checkSize(size);
  waitForData(size);
  
  m_pRing->get(m_pItem, size, size);   // Fetch the ring item.
}
/**
 * copyRingItem
 *   Copy a ring item from a CRingItem into our buffer.
 *
 * @param rItem - the item to copy.
 */
void
EventLogMain::copyRingItem(CRingItem& rItem)
{
  size_t nBytes = itemSize(rItem);
  checkSize(nBytes);
  
  memcpy(m_pItem, rItem.getItemPointer(), nBytes);
}
/**
 * checkSize
 *    If necessary expand the m_pItem block.
 * @param nBytes -size of the new block.
 */
void
EventLogMain::checkSize(size_t nBytes)
{
  if (m_nItemSize < nBytes) {
    m_pItem = static_cast<pRingItemHeader>(realloc(m_pItem, nBytes));
    m_nItemSize = nBytes;
  }
}

/**
 * waitForData
 *    Wait until the ring item has at least a required number of bytes
 *    for us.  The blocking part of the wait increases with each failure
 *    in order to adapt to actual data rates.
 *
 *  @param nBytes - desired minimum number of bytes in the ring.
 */
void
EventLogMain::waitForData(size_t nBytes)
{
  int sleepUs = 1;
  while (m_pRing->availableData() < nBytes) {
    usleep(sleepUs);
    if (sleepUs < MAXDATASLEEP) sleepUs = sleepUs * 2; // wait longer each time.
  }
}
/**
 * writeItem
 *    write a raw ring item.
 *    - The item is in m_pItem
 *    - The destination is m_pOutputter.
 */
void
EventLogMain::writeItem()
{
    m_pOutputter->put(m_pItem, m_pItem->s_size);
}
