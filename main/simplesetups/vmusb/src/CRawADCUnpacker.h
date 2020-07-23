
#ifndef CRAWADCUNPACKER_H
#define CRAWADCUNPACKER_H

#include <config.h>
#include <vector>
#include <utility>
#include <cstdint>
#include <TranslatorPointer.h>

struct ParsedADCEvent
{
  int s_geo;
  int s_crate;
  int s_count;
  int s_eventNumber;
  std::vector<std::pair<int, std::uint16_t> > s_data;
};

class CRawADCUnpacker 
{
  public:
    using Iter=TranslatorPointer<std::uint32_t>;

  private:
    enum ProcessingMode {HEADER_MODE, DATA_MODE, EOE_MODE, DONE_MODE};

  public:
    std::vector<ParsedADCEvent> 
      parseAll(const Iter& begin, const Iter& end);

    std::pair<Iter, ParsedADCEvent>
      parseSingle(const Iter& begin, const Iter& end);

  private:
    bool isHeader(std::uint32_t word);
    bool isData(std::uint32_t word);
    bool isEOE(std::uint32_t word);

    void unpackHeader(std::uint32_t word, ParsedADCEvent& event);
    void unpackDatum(std::uint32_t word, ParsedADCEvent& event);
    Iter 
      unpackData(const Iter& begin, const Iter& end, ParsedADCEvent& event);
    void unpackEOE(std::uint32_t word, ParsedADCEvent& event);
    void incompleteEvent(const char* whence);
};

#endif
