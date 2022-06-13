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
#include "RingSelectorMain.h"
#include <CRingBuffer.h>
#include <CRingSelectionPredicate.h>
#include <CAllButPredicate.h>
#include <CDesiredTypesPredicate.h>
#include <StringsToIntegers.h>
#include <CRemoteAccess.h>
#include <CRingItem.h>
#include <DataFormat.h>
#include <ErrnoException.h>
#include <io.h>

#include <os.h>

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdint.h>

#include "parser.h"
#include "COutputter.h"


/////////////////////////////////////////////////////////////////////////////////////////////
//
// Constructor and canonical implementations:

RingSelectorMain::RingSelectorMain() :
  m_pRing(0),
  m_pPredicate(0),
  m_queues(1000)		// # of ring items that can be in transit.
{}

RingSelectorMain::~RingSelectorMain()
{
  delete m_pRing;
  delete m_pPredicate;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Public members:


/*!
   Entry point:
   \param argc - Number of command line words.
   \param argv - Array of pointers to them.

   \return int
   \retval 0 - success
   \retval other - failure.
*/
int
RingSelectorMain::operator()(int argc, char** argv) 
{
  struct gengetopt_args_info parsedArgs;
  cmdline_parser(argc, argv, &parsedArgs);

  m_formatted  = parsedArgs.formatted_given;
  m_exitOnEnd  = parsedArgs.exitonend_given;
  m_nonBlocking= parsedArgs.non_blocking_given;

  m_pPredicate = createPredicate(&parsedArgs);

  m_pRing      = selectRing(&parsedArgs);
  processData();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Functional chunks of the program:

/*
**  Given the parsed arguments, create the appropriate predicate or error out.
**  Parameters:
**      parse - the arguments parsed by gengetopt's parser.
** Returns:
**   Pointer to a predicate that can be used to select the appropriate data from the
**   ring buffer.
** Side effects:
**   If both the --exclude and --select switches are presnt, that's an error we report and exit.
**
*/
CRingSelectionPredicate*
RingSelectorMain::createPredicate(struct gengetopt_args_info* parse)
{
  // Check for illegal switch combos:

  if (parse->exclude_given && parse->accept_given) {
    std::cerr << "Error, the --exclude and --select switches cannot both be given\n";
    exit(EXIT_FAILURE);
  }

  // Which is given determines the actual predicate type:
  // The default predicate type is an all but predicate...where the sampled switch
  // provides the only exceptions.

  CRingSelectionPredicate* returnValue;

  if (parse->accept_given) {
    // desired....

    CDesiredTypesPredicate* p = new CDesiredTypesPredicate();

    std::string types = parse->accept_arg;
    std::vector<int> acceptTypes = stringListToIntegers(types);
    addTypes(*p, acceptTypes);
    
    // Any sampling?

    if (parse->sample_given) {
      types = parse->sample_arg;
      std::vector<int> sampleTypes = stringListToIntegers(types);
      addTypes(*p, sampleTypes, true);
    }
    returnValue = p;

  }
  else {
    
    // All but predicate:

    CAllButPredicate* p = new CAllButPredicate();

    // If there excluded ones remove them.
    
    std::string types;
    
    if (parse->exclude_given) {
      types = parse->exclude_arg;
      std::vector<int> excludeTypes = stringListToIntegers(types);
      addExceptions(*p, excludeTypes);
    }
  

    // If there are sampled ones add them as sampled:

    if (parse->sample_given) {
      types = parse->sample_arg;
      std::vector<int> sampleTypes = stringListToIntegers(types);
      addExceptions(*p, sampleTypes, true);
      
    }
    returnValue = p;
  }

  return returnValue;
}
/*
** Determine the URL of the ring to attach to.
** Parameters:
**    parse  - The argument parse structure from gengetopt's parser.
** Returns:
**   Pointer to the ring.
** Side effects:
**  Can throw an exception if the ring does not exist.
*/
CRingBuffer*
RingSelectorMain::selectRing(struct gengetopt_args_info* parse)
{
  std::string url;
  if (parse->source_given) {
    url = parse->source_arg;
  }
  else {
    url = CRingBuffer::defaultRingUrl();
  }
  
  CRingBuffer* pRing = CRingAccess::daqConsumeFrom(url);

  if (!pRing) {
    throw std::string("Could not access the --source ring buffer");
  }
  return pRing;
}

/*
** Process the data from the ring.  In this case we just accept ring items
** from the ring and pound them out to stdout unchanged.
*/
void
RingSelectorMain::processData()
{
  COutputter outputThread(m_queues, m_exitOnEnd);
  outputThread.start();
  while(1) {
    CRingItem* pItem = CRingItem::getFromRing(*m_pRing, *m_pPredicate);
    RingItem*  pData = pItem->getItemPointer();
    size_t     size  = itemSize(pData);
    if (m_nonBlocking) {
      CRingItem* pQueueElement = m_queues.getFree();
      if(pQueueElement) {
        pQueueElement = pItem;
        m_queues.send(pQueueElement);
      } else {
        delete pItem;		// no free elements and non-blocking
      }
    } else {
      CRingItem* pQueueElement = m_queues.getFreeW();
      pQueueElement = pItem;
      m_queues.send(pQueueElement);
    }

  }
}

////////////////////////////////////////////////////////////////////////////
//
// Utilities


/*
** Write a chunk of data to an output file...looping as needed until either
** it's all been written or there's an error.
** The CErrnoException is thrown on errors.
** Parameters:
**   fd      - file desriptor on which to write the data.
**   pData   - Pointer to the block of data to write.
**   size    - Number of bytes of data to write.
*/
void
RingSelectorMain::writeBlock(int fd, void* pData, size_t size)
{

  try {
    io::writeData(fd, pData, size);
  }
  catch(int e) {
    if (e) {
      errno = e;
      throw CErrnoException("Writing data to output file");
    } else {
      throw std::string("Output file closed");
    }
  }

}
/**
 * addTypes
 *    Add desired0 types to an all but predicate
 *    (daqdev/NSCLDAQ#700).
 * @param p - refernences the predicate.
 * @param types - types to add.
 * @param sample - true if added with sampling.
 */
void
RingSelectorMain::addTypes(
  CDesiredTypesPredicate&  p, std::vector<int>& types, bool sample
)
{
  for (int i = 0; i < types.size(); i++) {
    p.addDesiredType(types[i], sample);
  }
  
}
/**
 * addExcedptions
 *    Add exception types to an all but predicate.
 *
 * @param p - refernences the predicate.
 * @param types - types to add.
 * @param sample - true if added with sampling.
 */
void
RingSelectorMain::addExceptions(
  CAllButPredicate&  p, std::vector<int>& types, bool sample
)
{
  for (int i =0; i < types.size(); i++) {
    p.addExceptionType(types[i], sample);
  }
}