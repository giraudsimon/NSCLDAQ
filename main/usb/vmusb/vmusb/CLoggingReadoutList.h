

#ifndef CMOCKREADOUTLIST_H
#define CMOCKREADOUTLIST_H

#include <CVMUSBReadoutList.h>
#include <vector>
#include <string>

class CLoggingReadoutList : public CVMUSBReadoutList 
{
  private:
    std::vector<std::string> m_log;

  public:

    std::vector<std::string> getLog() const { return m_log;}
    void clearLog() { m_log.clear();}
    size_t logSize() const { return m_log.size();}

    void clear();
    void append(const CLoggingReadoutList& list);

    void addRegisterRead(unsigned int address);
    void addWrite32(uint32_t address, uint8_t amod, uint32_t datum);
    void addWrite16(uint32_t address, uint8_t amod, uint16_t datum);
    void addWrite8(uint32_t address, uint8_t amod, uint8_t datum);

    void addRead32(uint32_t address, uint8_t amod);
    void addRead16(uint32_t address, uint8_t amod);
    void addRead8(uint32_t address, uint8_t amod);

    void addBlockRead32(uint32_t baseAddress, uint8_t amod, size_t transfers);
    void addFifoRead32(uint32_t baseAddress, uint8_t amod, size_t transfers);
    void addFifoRead16(uint32_t baseAddress, uint8_t amod, size_t transfers);
    void addBlockWrite32(uint32_t baseAddress, uint8_t amod, void* data, size_t transfers);

    void addBlockCountRead8(uint32_t address, uint32_t mask, uint8_t amod);
    void addBlockCountRead16(uint32_t address, uint32_t mask, uint8_t amod);
    void addBlockCountRead32(uint32_t address, uint32_t mask, uint8_t amod);

    void addMaskedCountBlockRead32(uint32_t address, uint8_t amod);
    void addMaskedCountFifoRead32(uint32_t address, uint8_t amod);

    void addDelay(uint8_t clocks);
    void addMarker(uint16_t value);

};

#endif
