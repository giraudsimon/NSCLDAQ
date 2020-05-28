
#include "CRawADCUnpacker.h"
#include <string>
#include <stdexcept>
#include <iostream>

using namespace std;

static const uint32_t TYPE_MASK (0x07000000);
static const uint32_t TYPE_HDR  (0x02000000);
static const uint32_t TYPE_DATA (0x00000000);
static const uint32_t TYPE_TRAIL(0x04000000);

static const unsigned GEO_SHIFT(27);
static const uint32_t GEO_MASK (0xf8000000);

static const unsigned HDR_COUNT_SHIFT(8);      
static const uint32_t HDR_COUNT_MASK (0x00003700);
static const unsigned HDR_CRATE_SHIFT(16);
static const uint32_t HDR_CRATE_MASK (0x00ff0000);

static const unsigned DATA_CHANSHIFT(16);
static const uint32_t DATA_CHANMASK (0x001f0000);
static const uint32_t DATA_CONVMASK (0x00003fff);

static const uint32_t TRAIL_COUNT_MASK(0x00ffffff);

vector<ParsedADCEvent> 
CRawADCUnpacker::parseAll(const Iter& begin,
                          const Iter& end)
{
  vector<ParsedADCEvent> parsedData;

  auto iter = begin;
  while (iter != end) {

    if (*iter != 0xffffffff) {
      auto result = parseSingle(iter,end);

      parsedData.push_back(result.second);
      iter = result.first;
    } else {
//      cout << hex << "Found non header " << *iter << dec << endl;
      ++iter;
    }
    // no need to increment because parseSingle has done it already.
  } 

  return parsedData;
}


pair<CRawADCUnpacker::Iter, ParsedADCEvent> 
  CRawADCUnpacker::parseSingle(const Iter& begin, const Iter& end)
{
  
  auto iter = begin;
  ParsedADCEvent event;

  if (iter<end) {
    unpackHeader(*iter++, event);
  } else {
    incompleteEvent("CRawADCUnpacker::parseSingle() ");
    
  }

  int nWords = event.s_count;
  auto dataEnd = iter+nWords;

  if ((dataEnd > end) || (dataEnd == end)) {
    incompleteEvent("CRawADCUnpacker::parseSingle() ");
  } else {
    iter = unpackData(iter, dataEnd, event);
  }

  if (iter<end) {
    unpackEOE(*iter++,event);
  } else {
    incompleteEvent("CRawADCUnpacker::parseSingle() ");
  }

  return make_pair(iter,event);
}


bool CRawADCUnpacker::isHeader(uint32_t word)
{
  return ((word&TYPE_MASK)==TYPE_HDR);
}

bool CRawADCUnpacker::isData(uint32_t word)
{
  return ((word&TYPE_MASK)==TYPE_DATA);
}

bool CRawADCUnpacker::isEOE(uint32_t word)
{
  return ((word&TYPE_MASK)==TYPE_TRAIL);
}






void CRawADCUnpacker::unpackHeader(uint32_t word, ParsedADCEvent& event)
{
  if (! isHeader(word) ) {
    string errmsg = "CRawADCUnpacker::parseHeader() ";
    errmsg += "Found non-header word when expecting header. ";
    errmsg += "Word=";
    errmsg += to_string(word);
    throw runtime_error(errmsg);
  }
//  cout << hex << "Header = " << word << dec << endl;

  event.s_geo   = ((word & GEO_MASK)>>GEO_SHIFT);
  event.s_crate = ((word & HDR_CRATE_MASK)>>HDR_CRATE_SHIFT);
  event.s_count = ((word & HDR_COUNT_MASK)>>HDR_COUNT_SHIFT);
}




void CRawADCUnpacker::unpackDatum(uint32_t word, ParsedADCEvent& event)
{
  if (! isData(word) ) {
    string errmsg = "CRawADCUnpacker::unpackDatum() ";
    errmsg += "Found non-data word when expecting data.";
    throw runtime_error(errmsg);
  }
//  cout << hex << "Data = " << word << dec << endl;

  int channel   = ((word & DATA_CHANMASK)>>DATA_CHANSHIFT);
  uint16_t data = (word & DATA_CONVMASK);

  auto chanData = make_pair(channel,data);
  event.s_data.push_back(chanData);
}




CRawADCUnpacker::Iter 
CRawADCUnpacker::unpackData(const Iter& begin, 
                            const Iter& end,
                            ParsedADCEvent& event)
{
  // only allocate memory once because we know how much we need already
  event.s_data.reserve(event.s_count);

  auto iter = begin;
  while(iter != end) {

    unpackDatum(*iter, event); 

    ++iter;
  }

  return iter;
}





void CRawADCUnpacker::unpackEOE(uint32_t word, ParsedADCEvent& event)
{
  if (! isEOE(word) ) {
    string errmsg = "CRawADCUnpacker::unpackEOE() ";
    errmsg += "Found non-data word when expecting data.";
    throw runtime_error(errmsg);
  }
//  cout << hex << "EOE = " << word << dec << endl;
  
  event.s_eventNumber = (word & TRAIL_COUNT_MASK);
}

void CRawAdcUnpacker::incompleteEvent(const char* whence)
{
  string errmsg(whence);
  errmsg += "Incomplete event found in buffer.";
  throw runtime_error(errmsg);  
}