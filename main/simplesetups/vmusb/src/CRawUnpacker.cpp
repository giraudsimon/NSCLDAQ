
#include "CRawUnpacker.h"
#include <BufferDecoder.h>
#include <TCLAnalyzer.h>
#include <iostream>

using namespace std;

CRawUnpacker::CRawUnpacker()
  : m_unpacker(),
    m_values("t",4096,0.0,4095.0,"channels",64,0)
{
}

CRawUnpacker::~CRawUnpacker()
{
}


Bool_t
CRawUnpacker::operator()(const Address_t pEvent, 
                        CEvent& rEvent, 
                        CAnalyzer& rAnalyzer, 
                        CBufferDecoder& rDecoder)
{
  TranslatorPointer<uint16_t> p(*rDecoder.getBufferTranslator(), pEvent);
  CTclAnalyzer& a(dynamic_cast<CTclAnalyzer&>(rAnalyzer));

  size_t  size = (*p++ & 0x0fff);

  // the event header is exclusive so the actual size of the full event
  // is actually the 
  a.SetEventSize((size+1)*sizeof(uint16_t));

  size_t nLongWords = size*sizeof(uint16_t)/sizeof(uint32_t);

  return unpack(p, nLongWords);
}


Bool_t 
CRawUnpacker::unpack(TranslatorPointer<uint32_t> begin,
                     size_t nLongWords)
{
  auto end = begin+nLongWords;

  try {
    vector<ParsedADCEvent> events = m_unpacker.parseAll(begin, end);
    int offset = 0;
    for (auto& event : events) {
      offset = (event.s_geo-10)*32;
      for (auto& chanData : event.s_data) {
        m_values[chanData.first+offset] = chanData.second;
      }
    }
  } catch (exception& exc) {
    cout << "Parsing Failed! Reason=" << exc.what() << endl;
    return kfFALSE;
  }
                                    
  return kfTRUE;
}

