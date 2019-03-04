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
#include "BufdumpMain.h"


#include <URL.h>
#include <CRingItemFactory.h>
#include <CRingItem.h>
#include <CRingBuffer.h>
#include <StringsToIntegers.h>
#include <CDataSource.h>
#include <CDataSourceFactory.h>
#include <CRingScalerItem.h>

#include "dumperargs.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdio.h>


using namespace std;

///////////////////////////////////////////////////////////////////////////////////
//
// Constructor and destructor
//

/*!
   Construct the object.  No real action occurs until the
   operator() is called, as all of the interseting data must be
   determined by parsing the command line arguments.
*/
BufdumpMain::BufdumpMain() :
  //  m_ringSource(true),
  //  m_pDataSource(0),
  m_skipCount(0),
  m_itemCount(0)
{
}

/*!
  Destroy the object:
*/
BufdumpMain::~BufdumpMain()
{
  //  delete m_pDataSource;

}
///////////////////////////////////////////////////////////////////////////////
//
// Public interface:
//

/*!
   Entry point for the dumper.
   - Parse the arguments.
   - Open the data source.
   - Accept items from the ring and dump them to stdout.

   \param argc   Number of command line arguments.
   \param argv   Array of pointers to command line arguments.

   \return int
   \retval EXIT_SUCCESS - Successful execution
   \retval EXIT_FAILURE - Some problem.. or may just exit.
*/
int
BufdumpMain::operator()(int argc, char** argv)
{
  // parse the arguments:

  gengetopt_args_info parse;
  cmdline_parser(argc, argv, &parse);

  // there should be no unnamed arguments. if there are,
  // print the help text and exit:

  if (parse.inputs_num > 0) {
    std::cerr << "Only switches are allowed, not command line parameters\n";
    cmdline_parser_print_help();
    return EXIT_FAILURE;
  }

  // Set the scaler format mask... note 32 is a bit pathalogical in a 
  // for uint32_t values...note also the max is 32 bits:

  if ((parse.scaler_width_arg > 32) ||  (parse.scaler_width_arg <= 0)) {
    std::cerr << "The --scaler-width flag value must be greater than zero and 32 or less, was: " 
	      << parse.scaler_width_arg << std::endl;
    return EXIT_FAILURE;
  } else if (parse.scaler_width_arg == 32) {
    CRingScalerItem::m_ScalerFormatMask = 0xffffffff;
  } else {
    CRingScalerItem::m_ScalerFormatMask = (1 << parse.scaler_width_arg) - 1;
  }

  // figure out the sample/exclusion vectors:

  vector<uint16_t> sample;
  vector<uint16_t> exclude;
  vector<int>      s;
  if (parse.sample_given) {
    try {
      s = stringListToIntegers(string(parse.sample_arg));
    }
    catch (...) {
      cerr << "Invalid value for --sample, must be a list of item types was: "
	   << string(parse.sample_arg) << endl;
      return EXIT_FAILURE;
    }
    for(int i=0; i < s.size(); i++) {
      sample.push_back(s[i]);
    }
  }
  
  vector<int> e;
  if (parse.exclude_given) {
    try {
      e = stringListToIntegers(string(parse.exclude_arg));
    }
    catch (...) {
      cerr << "Invalid value for --exclude, must be a list of item types was: "
	   << string(parse.sample_arg) << endl;
      return EXIT_FAILURE;
      
    }
    for (int i = 0; i < e.size(); i++) {
      exclude.push_back(e[i]);
    }
  }
    

  // figure out what sort of data source we have:

  string sourceName = defaultSource();
  if (parse.source_given) {
    sourceName = parse.source_arg;
  }

  
  CDataSource* pSource;
  try {
    pSource = CDataSourceFactory::makeSource(sourceName, sample, exclude);

  }
  catch (CException& e) {
    std::cerr << "Failed to open data source " << sourceName << std::endl;
    std::cerr << e.ReasonText() << std::endl;
    throw EXIT_FAILURE;
  }
  catch (std::string msg) {
    std::cerr << "Failed to open data source " << sourceName << std::endl;
    std::cerr << msg << std::endl;
    throw EXIT_FAILURE;
  }
  catch (const char* msg) {
    std::cerr << "Failed to open data source " << sourceName << std::endl;
    std::cerr << msg << std::endl;
    throw EXIT_FAILURE;
  }
  catch (...) {
    std::cerr << "Unanticipated exception attempting to create data source: " << std::endl;
    throw;
  }


  // We can now actually get stuff from the ring..but first we need to set the
  // skip and item count:

  if (parse.skip_given) {
    if (parse.skip_arg < 0) {
      cerr << "--skip value must be >= 0 but is "
	   << parse.skip_arg << endl;
      return EXIT_FAILURE;
    }
    m_skipCount = parse.skip_arg;

  }
  if (parse.count_given) {
    if (parse.count_arg < 0) {
      cerr << "--count value must be >= 0 but is "
	   << parse.count_arg << endl;
      return EXIT_FAILURE;
    }

    m_itemCount = parse.count_arg;
  }

  // Skip any items that need to be skipped:

  while (m_skipCount) {
    CRingItem* pSkip = pSource->getItem();
    delete pSkip;
    m_skipCount--;
  }

  size_t numToDo = m_itemCount;
  bool done = false;
  while (!done) {
    CRingItem *pItem = pSource->getItem();
    if (pItem) {
      processItem(*pItem);
      delete pItem;
      
      numToDo--;
      
      if ((m_itemCount != 0) && (numToDo == 0)) done = true;
    } else {
      done = true;
    }
  }

  return EXIT_SUCCESS;

}

//////////////////////////////////////////////////////////////////////////////////////
//
//   Utilities.

/*
** Process an item recieved from the ring.
** At this level, we just need to figure out what type of item we have
** cast construct a stand in for it, if necessary, and 
** dispatch to the appropriate dumper member function:
**
** Parameters:
**    item - Reference to the item to dump.
*/
void
BufdumpMain::processItem(const CRingItem& item)
{

  cout << "-----------------------------------------------------------\n";
  CRingItem* pActualItem = CRingItemFactory::createRingItem(item);
  cout << pActualItem->toString();
  delete pActualItem;

}

/*
** Return the default source URL which is 
** tcp://localhost/username
**
*/
string
BufdumpMain::defaultSource() const
{
  return CRingBuffer::defaultRingUrl();
			
}

/*
** Returns the time string associated with some time_t
*/
string
BufdumpMain::timeString(time_t theTime) const
{

  string result(ctime(&theTime));
  
  // For whatever reason, ctime appends a '\n' on the end.
  // We need to remove that.

  result.erase(result.size()-1);

  return result;
}
