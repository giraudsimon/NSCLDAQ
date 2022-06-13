
#include <CMockCCUSB.h>

#include <iostream>
#include <iomanip>
#include <typeinfo>

using namespace std;


bool CMockCCUSB::reconnect() {
  m_record.push_back("reconnect");
  return true;
}


void CMockCCUSB::writeActionRegister(uint16_t val)
{
  m_formatter.str(""); m_formatter.clear();
  m_formatter << hex << setfill('0');
  m_formatter << "writeActionRegister(0x" << setw(8) << val << ")";

  m_record.push_back(m_formatter.str());
}

int CMockCCUSB::executeList(CCCUSBReadoutList& list,
    void* pReadBuffer,
    size_t readBufferSize,
    size_t* bytesRead)
{
  int status = 0;
  try {
    // try to upcast reference. this throws if not possible.
    CLoggingReadoutList& logList = dynamic_cast<CLoggingReadoutList&>(list);
    status = executeLoggingRdoList(logList, pReadBuffer, readBufferSize, bytesRead);
  } catch (std::bad_cast&) {
    status = executeCCUSBRdoList(list,pReadBuffer, readBufferSize, bytesRead);
  }

  // fill the return data
  fillReturnData(pReadBuffer, readBufferSize, bytesRead);

  // pop off the front
  if (m_returnData.size()>0) {
    m_returnData.erase(m_returnData.begin());
  }

  return status; 
}

int CMockCCUSB::executeCCUSBRdoList(CCCUSBReadoutList& list,
    void* pReadBuffer,
    size_t readBufferSize,
    size_t* bytesRead)
{
  m_formatter << hex << setfill('0');

  std::vector<uint16_t> ops = list.get();
  unsigned int size = ops.size();

  // 
  m_record.reserve(m_record.size() + size + 2);
  m_record.push_back("executeList::begin");

  // add all of the operations
  for (unsigned int index=0; index < size; ++index) {

    m_formatter.str(""); m_formatter.clear();

    m_formatter << setw(4) << ops.at(index);
    m_record.push_back(m_formatter.str());
  }

  m_record.push_back("executeList::end");

  return 0;
}

int CMockCCUSB::executeLoggingRdoList(CLoggingReadoutList& list,
    void* pReadBuffer,
    size_t readBufferSize,
    size_t* bytesRead)
{
  m_record.push_back("executeList::begin");
  auto log = list.getLog();
  m_record.insert(m_record.end(), log.begin(), log.end());
  m_record.push_back("executeList::end");

  return 0;
}

int CMockCCUSB::loadList(uint8_t listNumber, CCCUSBReadoutList& list) 
{

  m_formatter << hex << setfill('0');

  std::vector<uint16_t> ops = list.get();
  unsigned int size = ops.size();

  //  make sure we have enough space in one potential reallocation
  m_record.reserve(m_record.size() + size + 2);

  m_formatter.str(""); m_formatter.clear();
  m_formatter << "loadlist-begin_" << listNumber;
  m_record.push_back(m_formatter.str());

  // add all of the operations
  for (unsigned int index=0; index < size; ++index) {

    m_formatter.str(""); m_formatter.clear();

    m_formatter << setw(8) << ops.at(index);
    m_record.push_back(m_formatter.str());
  }

  m_record.push_back("loadlist-end");
  
  return 0;             // ??
}

void CMockCCUSB::fillReturnData(void* pBuffer, size_t bufSize, size_t* nbytes) 
{
  //  figure out how many uint16_ts to copy into buffer
  //  - we copy the maximum number possible
  size_t bufferSize = bufSize/sizeof(uint16_t);

  if (m_returnData.size()==0) {
    *nbytes = 0;
  } else {

    auto& returnData = m_returnData.front();
    size_t nToReturn = std::min(bufferSize, returnData.size());
    (*nbytes) = nToReturn*sizeof(uint16_t);

    // copy
    uint16_t* buffer = reinterpret_cast<uint16_t*>(pBuffer);

    for (size_t index=0; index<nToReturn; ++index) {
      *(buffer+index) = returnData[index];
    }

    auto begin = returnData.begin();
    auto end = begin + nToReturn;

    returnData.erase(begin, end);
  }
}


int CMockCCUSB::usbRead(void* data, size_t bufferSize, size_t* transferCount, 
    int timeout) 
{
  m_formatter.str(""); m_formatter.clear();
  m_formatter << hex << setfill('0');

  m_formatter << "usbRead( 0x" << setw(8) << data << ", " << dec << bufferSize;
  m_formatter << ", 0x" << hex << setw(8) << transferCount << ", ";
  m_formatter << dec << timeout << ")";

  m_record.push_back(m_formatter.str());
<<<<<<< HEAD
  return 0;
=======
  return 0;                  // ??
>>>>>>> origin/12.0-pre5
}


void CMockCCUSB::addReturnDatum(uint16_t datum) 
{
  vector<uint16_t> returnData = {datum};
  vector<uint16_t>& buffer = createReturnDataStructure();
  buffer.insert(buffer.end(), returnData.begin(), returnData.end());
}
void CMockCCUSB::addReturnData(std::vector<uint16_t> data) 
{
  vector<uint16_t>& buffer = createReturnDataStructure();
  buffer.insert(buffer.end(), data.begin(), data.end());
}

