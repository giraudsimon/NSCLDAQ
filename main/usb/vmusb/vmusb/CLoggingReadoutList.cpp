
#include <CLoggingReadoutList.h>
#include <sstream>
#include <iomanip>

using namespace std;

void CLoggingReadoutList::clear() { 
  CVMUSBReadoutList::clear();
  clearLog();
}

void CLoggingReadoutList::append(const CLoggingReadoutList& list) {
  CVMUSBReadoutList::append(list);
  vector<std::string> record = list.getLog(); 
  m_log.insert(m_log.end(), record.begin(), record.end());
}  


void CLoggingReadoutList::addRegisterRead(unsigned int address)
{
  CVMUSBReadoutList::addRegisterRead(address);

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addRegisterRead " << setw(8) << address;
  m_log.push_back(ss.str());
}


void CLoggingReadoutList::addWrite32(uint32_t address, uint8_t amod, uint32_t datum)
{
  CVMUSBReadoutList::addWrite32(address, amod, datum);

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addWrite32 " << setw(8) << address 
     << " " << setw(2) << static_cast<uint16_t>(amod) 
     << " " << dec << datum;
  m_log.push_back(ss.str());
}

void CLoggingReadoutList::addWrite16(uint32_t address, uint8_t amod, uint16_t datum)
{
  CVMUSBReadoutList::addWrite16(address, amod, datum);

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addWrite16" 
     << " " << setw(8) << address 
     << " " << setw(2) << static_cast<uint16_t>(amod) 
     << " " << dec << datum;
  m_log.push_back(ss.str());
}

void CLoggingReadoutList::addWrite8(uint32_t address, uint8_t amod, uint8_t datum)
{
  CVMUSBReadoutList::addWrite8(address, amod, datum);

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addWrite8" 
     << " " << setw(8) << address 
     << " " << setw(2) << static_cast<uint16_t>(amod) 
     << " " << dec << static_cast<uint16_t>(datum);
  m_log.push_back(ss.str());
}

void CLoggingReadoutList::addRead32(uint32_t address, uint8_t amod)
{
  CVMUSBReadoutList::addRead32(address, amod);

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addRead32" 
     << " " << setw(8) << address 
     << " " << setw(2) << static_cast<uint16_t>(amod);
  m_log.push_back(ss.str());
}
void CLoggingReadoutList::addRead16(uint32_t address, uint8_t amod)
{
  CVMUSBReadoutList::addRead16(address, amod);

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addRead16" 
     << " " << setw(8) << address 
     << " " << setw(2) << static_cast<uint16_t>(amod);
  m_log.push_back(ss.str());
}

void CLoggingReadoutList::addRead8(uint32_t address, uint8_t amod)
{
  CVMUSBReadoutList::addRead8(address, amod);

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addRead8" 
     << " " << setw(8) << address 
     << " " << setw(2) << static_cast<uint16_t>(amod);
  m_log.push_back(ss.str());
}

void CLoggingReadoutList::addBlockRead32(uint32_t address, uint8_t amod, size_t transfers)
{
  CVMUSBReadoutList::addBlockRead32(address, amod, transfers);

  stringstream ss;
  ss.fill('0');
  ss << "addBlockRead32" 
     << hex 
     << " " << setw(8) << address 
     << " " << setw(2) << static_cast<uint16_t>(amod)
     << dec
     << " " << transfers;
  m_log.push_back(ss.str());
}

void CLoggingReadoutList::addFifoRead32(uint32_t address, uint8_t amod, size_t transfers)
{
  CVMUSBReadoutList::addFifoRead32(address, amod, transfers);

  stringstream ss;
  ss.fill('0');
  ss << "addFifoRead32" 
     << hex 
     << " " << setw(8) << address 
     << " " << setw(2) << static_cast<uint16_t>(amod)
     << dec
     << " " << transfers;
  m_log.push_back(ss.str());
}

void CLoggingReadoutList::addFifoRead16(uint32_t address, uint8_t amod, size_t transfers)
{
  CVMUSBReadoutList::addFifoRead16(address, amod, transfers);

  stringstream ss;
  ss.fill('0');
  ss << "addFifoRead16" 
     << hex 
     << " " << setw(8) << address 
     << " " << setw(2) << static_cast<uint16_t>(amod)
     << dec
     << " " << transfers;
  m_log.push_back(ss.str());
}

void CLoggingReadoutList::addBlockWrite32(uint32_t address, uint8_t amod, 
                                       void* data, size_t transfers)
{
  CVMUSBReadoutList::addBlockWrite32(address, amod, data, transfers);

  stringstream ss;
  ss << "addBlockWrite32" 
     << hex << setfill('0')
     << " " << setw(8) << address 
     << " " << setw(2) << static_cast<uint32_t>(amod)
     << dec 
     << " " << transfers;
  m_log.push_back(ss.str());

  m_log.reserve(m_log.size() + transfers);
  uint32_t* pData = reinterpret_cast<uint32_t*>(data);
  for (size_t it=0; it<transfers; ++it) {
    ss.str(""); ss.clear();
    ss << dec << setfill(' ') << setw(4) << it << " : " 
       << hex << setfill('0') << setw(8) << *(pData+it);
    m_log.push_back(ss.str());
  }
}

void 
CLoggingReadoutList::addBlockCountRead8(uint32_t address, uint32_t mask, 
                                     uint8_t amod)
{
  CVMUSBReadoutList::addBlockCountRead8(address,mask,amod);

  // addBlockCountRead8 is a two step operation that first calls addRead8.
  // The side effect of this is that an extra entry for addRead8 will be
  // appended into the log. We first must pop it off.
  //
  m_log.pop_back();

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addBlockCountRead8"
    << " " << setw(8) << address
    << " " << setw(8) << mask
    << " " << setw(2) << static_cast<uint16_t>(amod);
  m_log.push_back(ss.str()); 
}

void 
CLoggingReadoutList::addBlockCountRead16(uint32_t address, uint32_t mask, 
                                     uint8_t amod)
{
  CVMUSBReadoutList::addBlockCountRead16(address,mask,amod);

  // addBlockCountRead16 is a two step operation that first calls addRead16.
  // The side effect of this is that an extra entry for addRead16 will be
  // appended into the log. We first must pop it off.
  //
  m_log.pop_back();

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addBlockCountRead16"
    << " " << setw(8) << address
    << " " << setw(8) << mask
    << " " << setw(2) << static_cast<uint16_t>(amod);
  m_log.push_back(ss.str()); 
}

void 
CLoggingReadoutList::addBlockCountRead32(uint32_t address, uint32_t mask, 
                                     uint8_t amod)
{
  CVMUSBReadoutList::addBlockCountRead32(address,mask,amod);

  // addBlockCountRead32 is a two step operation that first calls addRead32.
  // The side effect of this is that an extra entry for addRead32 will be
  // appended into the log. We first must pop it off.
  //
  m_log.pop_back();

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addBlockCountRead32"
    << " " << setw(8) << address
    << " " << setw(8) << mask
    << " " << setw(2) << static_cast<uint16_t>(amod);
  m_log.push_back(ss.str()); 
}

void 
CLoggingReadoutList::addMaskedCountBlockRead32(uint32_t address, uint8_t amod)
{
  CVMUSBReadoutList::addMaskedCountBlockRead32(address,amod);

  // this is just a wrapper around the addBlockRead32 method so the above
  // line has the side effect of adding an entry to the log. We must
  // pop it off the back to keep this clean.
  m_log.pop_back();

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addMaskedCountBlockRead32"
    << " " << setw(8) << address
    << " " << setw(2) << static_cast<uint16_t>(amod);
  m_log.push_back(ss.str()); 

}

void 
CLoggingReadoutList::addMaskedCountFifoRead32(uint32_t address, uint8_t amod)
{
  CVMUSBReadoutList::addMaskedCountFifoRead32(address,amod);

  // this is just a wrapper around the addFifoRead32 method so the above
  // line has the side effect of adding an entry to the log. We must
  // pop it off the back to keep this clean.
  m_log.pop_back();

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addMaskedCountFifoRead32"
    << " " << setw(8) << address
    << " " << setw(2) << static_cast<uint16_t>(amod);
  m_log.push_back(ss.str()); 

}

void CLoggingReadoutList::addDelay(uint8_t clocks) 
{
  CVMUSBReadoutList::addDelay(clocks);

  stringstream ss;
  ss << "addDelay " << static_cast<uint16_t>(clocks);
  m_log.push_back(ss.str()); 
}

void CLoggingReadoutList::addMarker(uint16_t value) 
{
  CVMUSBReadoutList::addDelay(value);

  stringstream ss;
  ss.flags(ios::hex);
  ss.fill('0');
  ss << "addMarker " << setw(4) << value;
  m_log.push_back(ss.str()); 
}

