/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2018.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Giordano Cerizza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include <CDataSource.h>           
#include <CDataSourceFactory.h>    
#include <CRingItem.h>             
#include <DataFormat.h>            
#include <Exception.h>             
#include <CRingItemFactory.h>      
#include <CRingScalerItem.h>       
#include <CRingStateChangeItem.h>  
#include <CRingTextItem.h>          
#include <CPhysicsEventItem.h>       
#include <CRingPhysicsEventCountItem.h>
#include <CDataFormatItem.h>         
#include <CGlomParameters.h>         
#include <CDataFormatItem.h>         

#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <cstdint>
#include <map>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include <unistd.h>

#include <ctime>

#include "scalerReaderMain.h"

bool debug = false;

///////////////////////////////////////////////////////////////////////////
//
// Constructors and other canonicals we bothered to implement:
//

/*!
  Construct the object.  This just initializes stuff to reasonable defaults.
  The real preparation is done in operator() since we need the command
  arguments to know what to do.
*/
scalerReaderMain::scalerReaderMain()
{}

/*!
   Destruction just kills off the dynamic elements:

*/
scalerReaderMain::~scalerReaderMain()
{}
  
//////////////////////////////////////////////////////////////////////////////
//
// Public interface
//

int
scalerReaderMain::operator()(int argc, char** argv)
{
  std::string sourceName;
  std::string run;
  int nbuff=0;
  
  if (argc < 2)
    {
      std::cout << "Formatted dump of acquired scaler items" << std::endl;
      std::cout << "The instantaneous scalers are dumped into a csv file while the summary is printed on screen" << std::endl << std::endl;
      std::cout << "Usage: scalerReader source number_of_buffers" << std::endl << std::endl;
      std::cout << "source=STRING\t URL of source, ring buffer or file" << std::endl;
      std::cout << "number_of_buffers=INT\t if added, prints to screen the partial scaler summary every N scaler buffers" << std::endl;       
      exit(0);
    }
  else
    {
      sourceName = argv[1];
      if (argv[2] != NULL)
	nbuff = std::atoi(argv[2]);
      unsigned first = sourceName.find("-")+1;
      unsigned last = sourceName.find("-00.evt");
      run = sourceName.substr (first,last-first);
    }
  
  // Output file
  std::string ofile = "scaler_run"+run+".csv";
  outscalers.open(ofile);
  
  outscalers << "SourceID,Time";
  for (int i=0; i<32; i++){
    scalernumbers[i] = 0;
    outscalers << ",Channel" << i;
  }
  outscalers << std::endl;
  
  scalerBuffer = 0;
  
  // Make the exclusion list and sample (none) list:
  // then connect to the ring:

  std::vector<uint16_t>  sample;
  std::vector<uint16_t>  exclude;

  exclude.push_back(PACKET_TYPES);
  exclude.push_back(MONITORED_VARIABLES);
  exclude.push_back(PHYSICS_EVENT);
  exclude.push_back(PHYSICS_EVENT_COUNT);
  exclude.push_back(EVB_FRAGMENT);
  exclude.push_back(EVB_UNKNOWN_PAYLOAD);

  CDataSource* pDataSource;
  try {
    pDataSource = CDataSourceFactory::makeSource(sourceName, sample, exclude);
    if (debug)
      std::cout << "Source attached!" << std::endl;
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
  
  // Process items from the ring.
  CRingItem*  pItem;
    
  while ((pItem = pDataSource->getItem() )) {
    std::unique_ptr<CRingItem> item(pItem);
    processRingItem(*item);
    
    if (scalerBuffer > 0 && nbuff != 0 && (scalerBuffer%nbuff) == 0) {
        // Partial summary
        header("Partial summary", run);
        dumpScalers();
      
    }

    // Summary
    
    header("Summary", run);
    dumpScalers();
  }  
  
  return EXIT_SUCCESS;	
    
}

/////////////////////////////////////////////////////////////////////////////////
//
// Private utilities.
//
void
scalerReaderMain::processRingItem(CRingItem& item)
{
  CRingItem* castableItem = CRingItemFactory::createRingItem(item);
  std::unique_ptr<CRingItem> autoDeletedItem(castableItem); 

  uint32_t type  = castableItem->type();
  switch (type) {
  case PERIODIC_SCALERS:
    {    
      scalerBuffer++;
      CRingScalerItem& scaler(dynamic_cast<CRingScalerItem&>(*castableItem));
      processScalerItem(scaler);
      break;
    }
  case BEGIN_RUN:             
  case END_RUN:
  case PAUSE_RUN:
  case RESUME_RUN:
    {
      CRingStateChangeItem& statechange(dynamic_cast<CRingStateChangeItem&>(*castableItem));
      processStateChangeItem(statechange);
      break;
    }
  case PACKET_TYPES:        
  case MONITORED_VARIABLES:
    {
      CRingTextItem& text(dynamic_cast<CRingTextItem&>(*castableItem));
      processTextItem(text);
      break;
    }
  case PHYSICS_EVENT:
    {
      CPhysicsEventItem& event(dynamic_cast<CPhysicsEventItem&>(*castableItem));
      processEvent(event);
      break;
    }
  case PHYSICS_EVENT_COUNT:
    {
      CRingPhysicsEventCountItem& eventcount(dynamic_cast<CRingPhysicsEventCountItem&>(*castableItem));
      processEventCount(eventcount);
      break;
    }
  case RING_FORMAT:
    {
      CDataFormatItem& format(dynamic_cast<CDataFormatItem&>(*castableItem));
      processFormat(format);
      break;
    }
  case EVB_GLOM_INFO:
    {
      CGlomParameters& glomparams(dynamic_cast<CGlomParameters&>(*castableItem));
      processGlomParams(glomparams);
      break;
    }
  default:
    {
      processUnknownItemType(item);
      break;
    }
  }

  
}

void
scalerReaderMain::processScalerItem(const CRingScalerItem& item)
{
  if (debug)
    std::cout << "#########################" << std::endl;
  int sourceID;
  if (item.hasBodyHeader()) {
    sourceID = item.getSourceId();
  }  else {
    if (debug)
      std::cout << "No body header for scaler!" << std::endl;
    sourceID = -1;
  }

  time_t ts = item.getTimestamp();
  if (debug)
    std::cout << "Scaler item recorded on: " << ctime(&ts) << std::endl;
  outscalers << sourceID << "," << ctime(&ts);
  
  for (int i = 0; i < item.getScalerCount(); i++) {
    if (debug)
      std::cout << "Channel: " << i << " " << item.getScaler(i) << std::endl;
    outscalers << "," << item.getScaler(i);
    scalernumbers[i]+=item.getScaler(i);
  }
  outscalers << std::endl;
  
}

void
scalerReaderMain::processStateChangeItem(CRingStateChangeItem& item)
{}

void
scalerReaderMain::processTextItem(CRingTextItem& item)
{}

void
scalerReaderMain::processEvent(CPhysicsEventItem& item)
{}

void
scalerReaderMain::processEventCount(CRingPhysicsEventCountItem& item)
{}

void
scalerReaderMain::processFormat(CDataFormatItem& item)
{}

void
scalerReaderMain::processGlomParams(CGlomParameters& item)
{}

void
scalerReaderMain::processUnknownItemType(CRingItem& item)
{}
/**
 * dumpScalers
 *    Ouptut the scaler information under the header to stdout
 */
void
scalerReaderMain::dumpScalers()
{
  for (int i = 0; i < 32; i++){
    std::cout << "Channel " << i << " " << scalernumbers[i] << "  Ave Rate = " << (float)scalernumbers[i]/(float)scalerBuffer << std::endl;
  }
    
}
/**
 * header
 *    Output a header.
 * @param title - type of summary header.
 * @param run   - run number.
 */
void
scalerReaderMain::header(const char* title, std::string run)
{
    std::cout << "#########################################" << std::endl;
    std::cout << "# " << title << " = " << run << std::endl;
    std::cout << "Number of scaler buffers read = " << scalerBuffer << std::endl;
    std::cout << "#########################################" << std::endl;
      
}
