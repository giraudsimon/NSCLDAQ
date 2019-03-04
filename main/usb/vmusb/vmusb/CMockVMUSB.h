
#ifndef CMOCKVMUSB_H
#define CMOCKVMUSB_H


#include <map>
#include <vector>
#include <string>
#include <stdint.h>
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include "CLoggingReadoutList.h"

class CMockVMUSB : public CVMUSB
{
    private:
    std::vector<std::string> m_opRecord;
    std::map<uint32_t, uint32_t> m_registers;
    std::map<uint32_t, std::string> m_registerNames;
    std::map<uint32_t, uint32_t> m_addressData;
    std::vector<std::pair<int,std::vector<uint16_t> > > m_returnData;

    public:
    CMockVMUSB();

    virtual CLoggingReadoutList* createReadoutList() const; 

    virtual void writeActionRegister(uint16_t data);

    /** \brief Execute a list
     * 
     *  This actually just dispatches to a private helper method. If the list 
     *  passed is truly a CVMUSBReadoutList, then executVMUSBRdoList is called.
     *  Otherwise, if an upcast to a CLoggingReadoutList succeeds, the 
     *  executeLoggingRdoList method is called.
     */
    virtual int executeList(CVMUSBReadoutList& list, void* pReadBuffer, 
                            size_t readBufferSize, size_t* bytesRead);
    virtual int loadList(uint8_t listNumber, CVMUSBReadoutList& list, 
                         off_t listOffset=0);
    virtual int usbRead(void* data, size_t bufferSize, size_t* transferCount, 
                        int timeout=2000);

    std::vector<std::string> getOperationRecord() const { return m_opRecord;}

    std::pair<int,std::vector<uint16_t> >& createReturnDataStructure() 
    {
      m_returnData.push_back( std::make_pair(0,std::vector<uint16_t>()) );
      return m_returnData.back();
    }

    std::vector<std::pair<int,std::vector<uint16_t> > >& getReturnData() 
    {
      return m_returnData;
    };

    void addReturnDatum(uint16_t datum, int status=0);
    void addReturnData(std::vector<uint16_t> datum, int status);

    private:
     void setUpRegisterMap(); 
     void setUpRegisterNameMap(); 

     virtual void writeRegister(uint32_t regAddr, uint32_t value);
     virtual uint32_t readRegister(uint32_t regAddr);

    template<class T>
     void recordVMEOperation(std::string opname, uint32_t address, 
                             uint8_t addrMod, T data);
    template<class T>
     void recordOperation(std::string opname, T data);

    
    int executeLoggingRdoList(CLoggingReadoutList& list, void* pReadBuffer, 
                              size_t readBufferSize, size_t* bytesRead);
    int executeVMUSBRdoList(CVMUSBReadoutList& list, void* pReadBuffer, 
                            size_t readBufferSize, size_t* bytesRead);
    void fillReturnData(void* pReadBuffer, size_t bufSize, size_t* nBytesRead);
};


inline
std::string vecstring_at(std::vector<std::string> list, unsigned int index) {
  return list.at(index);
}

inline
void vecstring_pushback(std::vector<std::string> list, std::string message) {
  return list.push_back(message);
}

inline
unsigned int vecstring_size(std::vector<std::string> list) {
  return list.size();
}

#endif
