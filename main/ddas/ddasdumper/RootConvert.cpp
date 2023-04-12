// evt to root converter
//#include <string.h>
#include <stdint.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <CRingItem.h>
#include <CPhysicsEventItem.h>
#include <exception>

#include "ddaschannel.h"
#include "RootConvert.h"
#include "TTree.h"
#include <DataFormat.h>


RootConverter::RootConverter() : Converter(), dchan(new ddaschannel) {}

RootConverter::~RootConverter() 
{ 
    if (dchan!=0) {
        delete dchan; 
        dchan=0;
    } 
}

void RootConverter::Initialize(std::string in, std::string fout) 
{
    // Open files and create tree
    Converter::Initialize(in,fout);

    m_treeout->Branch("dchan",&dchan,32000,99);
}
/**
*  DumpData - dump the data from the item.. note the item is from the proper
*             format so we can get the body independent of the format.
*    @param item - reference to the item to dump to the root file.
*/
void RootConverter::DumpData(const CPhysicsEventItem& item){
  
  const uint32_t* body = reinterpret_cast<const uint32_t*>(item.getBodyPointer());   

  
  
  // take data from ring and pack it into a ddaschannel object in ROOT file
  dchan->Reset();
  try {
    dchan->UnpackChannelData(body);
  } catch (std::exception& exc) {
    std::cout << "Caught exception while parsing : " << exc.what() << std::endl;
    throw;
  }

  uint32_t nentries = m_treeout->GetEntries();
  if((nentries%1000) == 0) std::cout << "processed " << ((Int_t)(nentries/1000))*1000 << " events " << std::endl;
  m_treeout->Fill();
}

