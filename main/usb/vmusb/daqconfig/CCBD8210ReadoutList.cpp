
#include "CCBD8210CamacBranchDriver.h"
#include "CCBD8210ReadoutList.h"
#include "CVMUSBReadoutList.h"

CCBD8210ReadoutList::CCBD8210ReadoutList()
: m_branch(-1),
    m_crate(-1),
    m_rdoList(new CVMUSBReadoutList),
    m_isListOwner(true)
{
}

CCBD8210ReadoutList::CCBD8210ReadoutList(int branch, int crate)
: m_branch(branch),
    m_crate(crate),
    m_rdoList(new CVMUSBReadoutList),
    m_isListOwner(true)
{
}

void CCBD8210ReadoutList::setReadoutList(CVMUSBReadoutList& rdoList)
{
    if (m_rdoList && m_isListOwner) {
        delete m_rdoList;
    }
    m_isListOwner=false;
    m_rdoList = &rdoList;
}

void CCBD8210ReadoutList::addRead16(int n, int a, int f) 
{

    if (m_rdoList) {

        // This saddens me because I cant write
        // auto func = CCBD8210CamacBranchDriver::convertToAddress;
    
        uint32_t addr = CCBD8210CamacBranchDriver::convertToAddress(m_branch,m_crate,n,a,f);
        uint32_t word16bit = CCBD8210CamacBranchDriver::BIT16;

        addr = word16bit | addr; 
        m_rdoList->addRead16(addr, CVMUSBReadoutList::a24UserData);
        // testing an added delay of 1 clock cycle
//        m_rdoList->addDelay(20);
    }
}

void CCBD8210ReadoutList::addRead24(int n, int a, int f) 
{
    if (m_rdoList) {
        uint32_t addr = CCBD8210CamacBranchDriver::convertToAddress(m_branch,m_crate,n,a,f);
        uint32_t word16bit = CCBD8210CamacBranchDriver::BIT16;
        uint32_t word24bit = CCBD8210CamacBranchDriver::BIT24;

        m_rdoList->addRead16(addr | word24bit, CVMUSBReadoutList::a24UserData);
        m_rdoList->addRead16(addr | word16bit, CVMUSBReadoutList::a24UserData);
//        m_rdoList->addRead32(addr, CVMUSBReadoutList::a24UserData);
        // testing an added delay of 1 clock cycle
        m_rdoList->addDelay(5);
    }
}

void CCBD8210ReadoutList::addWrite16(int n, int a, int f, uint16_t data) 
{
    if (m_rdoList) {

        uint32_t addr = CCBD8210CamacBranchDriver::convertToAddress(m_branch,m_crate,n,a,f);
        uint32_t word16bit = CCBD8210CamacBranchDriver::BIT16;

        addr = word16bit | addr; 
        m_rdoList->addWrite16(addr, CVMUSBReadoutList::a24UserData, data);
        // testing an added delay of 1 clock cycle
//        m_rdoList->addDelay(20);
    }
}

void CCBD8210ReadoutList::addWrite24(int n, int a, int f, uint32_t data)
{
    if (m_rdoList) {

        uint32_t addr = CCBD8210CamacBranchDriver::convertToAddress(m_branch,m_crate,n,a,f);
        uint32_t word16bit = CCBD8210CamacBranchDriver::BIT16;
        uint32_t word24bit = CCBD8210CamacBranchDriver::BIT24;

        uint16_t lodata = (data & 0xffff);
        uint16_t hidata = ( (data>>16) & 0xff );

        // vme does not do 24-bit transfers
        m_rdoList->addWrite16(addr | word24bit, CVMUSBReadoutList::a24UserData, hidata);
        m_rdoList->addWrite16(addr | word16bit, CVMUSBReadoutList::a24UserData, lodata);
        // testing an added delay of 1 clock cycle
//        m_rdoList->addDelay(20);

    }
}

void CCBD8210ReadoutList::addControl(int n, int a, int f)
{
    if (m_rdoList) {

        addRead24(n,a,f);
         
//        m_rdoList->addRead16(addr, CVMUSBReadoutList::a24UserData);
        // testing an added delay of 1 clock cycle
//        m_rdoList->addDelay(1);
    }
}

void CCBD8210ReadoutList::clear()
{
    if (m_rdoList) {
        m_rdoList->clear();
    }
}

size_t CCBD8210ReadoutList::size() const
{
    size_t s=0;

    if (m_rdoList) {
        s = m_rdoList->size();
    }

    return s;
}



void CCBD8210ReadoutList::addQStop24(int n, int a, int f, int max_reps, bool lamwait)
{
    for (int i=0; i<max_reps; ++i) {
        addRead24(n,a,f);
    }

} 
