


#ifndef CMOCKCCUSB_H
#define CMOCKCCUSB_H

#include <string>
#include <vector>
#include <sstream>

#include <memory>
#include <CLoggingReadoutList.h>
#include <CCCUSB.h>

class CMockCCUSB : public CCCUSB {

  private:
    std::vector<std::string> m_record;
    std::ostringstream       m_formatter;
    std::vector<std::vector<uint16_t> > m_returnData;
  
  public:
    CLoggingReadoutList* createReadoutList() const 
    {return new CLoggingReadoutList;}

    bool reconnect();
    void writeActionRegister(uint16_t val);

    int executeList(CCCUSBReadoutList& list,
                    void* pReadBuffer,
                    size_t readBufferSize,
                    size_t* bytesRead);

    int loadList(uint8_t listNumber, CCCUSBReadoutList& list);
    int usbRead(void* data, size_t bufferSize, size_t* transferCount, 
                int timeout);


    std::vector<std::string> getOperationsRecord() {return m_record;}

    std::vector<uint16_t>& createReturnDataStructure() 
    {
      m_returnData.push_back(std::vector<uint16_t>());
      return m_returnData.back();
    }

    std::vector<std::vector<uint16_t> >& getReturnData() 
    {
      return m_returnData;
    };

    void addReturnDatum(uint16_t datum);
    void addReturnData(std::vector<uint16_t> datum);


  private:
    int executeCCUSBRdoList(CCCUSBReadoutList& list,
                    void* pReadBuffer,
                    size_t readBufferSize,
                    size_t* bytesRead);
    int executeLoggingRdoList(CLoggingReadoutList& list,
                    void* pReadBuffer,
                    size_t readBufferSize,
                    size_t* bytesRead);
    void fillReturnData(void* pReadBuffer,
                    size_t readBufferSize,
                    size_t* bytesRead);
                           
};

#endif 
