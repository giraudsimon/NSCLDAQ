
#ifndef CRAWUNPACKER_H
#define CRAWUNPACKER_H

#include <config.h>
#include "CRawADCUnpacker.h"
#include <EventProcessor.h>
#include <TreeParameter.h>
#include <cstdint>
#include <cstddef>

class CRawUnpacker : public CEventProcessor
{
  private:
    CRawADCUnpacker      m_unpacker;
    CTreeParameterArray  m_values;

  public:
    CRawUnpacker();
    virtual ~CRawUnpacker();

    virtual Bool_t operator()(const Address_t pEvent,
                              CEvent& rEvent,
                              CAnalyzer& rAnalyzer,
                              CBufferDecoder& rDecoder);

    Bool_t unpack(TranslatorPointer<std::uint32_t> begin, 
                  std::size_t nLongWords);
};

#endif
