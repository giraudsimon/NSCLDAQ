
#include "CCBD8210CrateController.h"
#include "CCBD8210CamacBranchDriver.h"
#include "CVMUSB.h"
#include "CNAF.h"
#include "CVMUSBReadoutList.h"

CCBD8210CrateController::CCBD8210CrateController(int branch, int crate)
 : CCrateController(),
   m_branch(branch),
   m_crate(crate),
   m_ctlr(0)
{
}

CCBD8210ReadoutList* CCBD8210CrateController::createReadoutList() const
{
    return new CCBD8210ReadoutList(m_branch,m_crate);
}

int CCBD8210CrateController::simpleWrite16(int n, int a, int f, uint16_t data, uint16_t& qx)
{
    int status = -10;
    qx=0;
    if (m_ctlr) {

        // This saddens me because I cant write
        // auto func = CCBD8210CamacBranchDriver::convertToAddress;
    
        uint32_t addr = CCBD8210CamacBranchDriver::convertToAddress(m_branch,m_crate,n,a,f);
        uint32_t word16bit = CCBD8210CamacBranchDriver::BIT16;

        addr = word16bit | addr; 
        
        CVMUSBReadoutList list;
        status = m_ctlr->vmeWrite16(addr, CVMUSBReadoutList::a24UserData, data);
//        list.addWrite16(addr, CVMUSBReadoutList::a24UserData, data);
//        list.addDelay(1); // add 1 clock = 12.5us
//
//        uint32_t dummy;        
//        size_t nbytes;        
//        status = m_ctlr->executeList(list,&dummy,sizeof(dummy),&nbytes);
        if (status<0) {
            std::cerr << "CCBD8210CrateController::simpleWrite16(int,int,int,uint16_t,uint16_t&)";
            std::cerr << " failed vmeWrite16 with error = " << status;
            std::cerr << std::endl;
            std::cerr << "b = " << m_branch << " c = " << m_crate
                      << " n = " << n << " a = " << a << " f = " << f
                      << " d = " << data << std::endl;
        }
        bool q = qTest();
        bool x = xTest();
        qx = formXQ(x,q);
    }
    return status;
}


int CCBD8210CrateController::simpleWrite24(int n, int a, int f, uint32_t data, uint16_t& qx)
{
    int status = -10;
    qx=0;
    if (m_ctlr) {

        // This saddens me because I cant write
        // auto func = CCBD8210CamacBranchDriver::convertToAddress;

        uint32_t addr = CCBD8210CamacBranchDriver::convertToAddress(m_branch,m_crate,n,a,f);
        uint32_t word16bit = CCBD8210CamacBranchDriver::BIT16;
        uint32_t word24bit = CCBD8210CamacBranchDriver::BIT24;


        uint16_t lodata = (data & 0xffff);
        uint16_t hidata = ( (data>>16) & 0xff );

        status = m_ctlr->vmeWrite16(addr|word24bit, CVMUSBReadoutList::a24UserData, hidata);
        if (status<0) {
            std::cerr << "CCBD8210CrateController::simpleWrite32(int,int,int,uint32_t,uint16_t&)";
            std::cerr << " failed vmeWrite32 with error = " << status;
            std::cerr << std::endl;
            std::cerr << "b = " << m_branch << " c = " << m_crate
                      << " n = " << n << " a = " << a << " f = " << f
                      << " d = " << data << std::endl;
        }

        status = m_ctlr->vmeWrite16(addr|word16bit, CVMUSBReadoutList::a24UserData, lodata);
        if (status<0) {
            std::cerr << "CCBD8210CrateController::simpleWrite32(int,int,int,uint32_t,uint16_t&)";
            std::cerr << " failed vmeWrite32 with error = " << status;
            std::cerr << std::endl;
            std::cerr << "b = " << m_branch << " c = " << m_crate
                      << " n = " << n << " a = " << a << " f = " << f
                      << " d = " << data << std::endl;

        }

        bool q = qTest();
        bool x = xTest();
        qx = formXQ(x,q);
    }   
    return status;
}

int CCBD8210CrateController::simpleRead16( int n, int a, int f, uint16_t& data, uint16_t& qx)
{
    int status = -10;
    qx=0;
    if (m_ctlr) {

        // This saddens me because I cant write
        // auto func = CCBD8210CamacBranchDriver::convertToAddress;
    
        uint32_t addr = CCBD8210CamacBranchDriver::convertToAddress(m_branch,m_crate,n,a,f);
        uint32_t word16bit = CCBD8210CamacBranchDriver::BIT16;

        addr = word16bit | addr; 
        CVMUSBReadoutList list;
        status = m_ctlr->vmeRead16(addr, CVMUSBReadoutList::a24UserData, &data);
//        list.addRead16(addr, CVMUSBReadoutList::a24UserData);
//        list.addDelay(1); // add 1 clock = 12.5us

//        size_t nbytes;        
//        status = m_ctlr->executeList(list,&data,sizeof(data),&nbytes);
        if (status<0) {
            std::cerr << "CCBD8210CrateController::simpleRead16(int,int,int,uint16_t&,uint16_t&)";
            std::cerr << " failed vmeRead16 with error = " << status;
            std::cerr << std::endl;
            std::cerr << "b = " << m_branch << " c = " << m_crate
                      << " n = " << n << " a = " << a << " f = " << f
                      << std::endl;
        }

        bool q = qTest();
        bool x = xTest();
        qx = formXQ(x,q);
    }
    return status;
}

int CCBD8210CrateController::simpleRead24( int n, int a, int f, uint32_t& data, uint16_t& qx)
{
    int status = -10;
    qx=0;
    if (m_ctlr) {

        uint32_t addr = CCBD8210CamacBranchDriver::convertToAddress(m_branch,m_crate,n,a,f);
        uint32_t word16bit = CCBD8210CamacBranchDriver::BIT16;
        uint32_t word24bit = CCBD8210CamacBranchDriver::BIT24;

        uint16_t lodata=0, hidata=0;

        status = m_ctlr->vmeRead16(addr|word24bit, CVMUSBReadoutList::a24UserData,&hidata);
        if (status<0) {
            std::cerr << "CCBD8210CrateController::simpleRead32(int,int,int,uint32_t&,uint16_t&)";
            std::cerr << " failed vmeRead32 with error = " << status;
            std::cerr << std::endl;
            std::cerr << "b = " << m_branch << " c = " << m_crate
                      << " n = " << n << " a = " << a << " f = " << f
                      << std::endl;
        }

        status = m_ctlr->vmeRead16(addr|word16bit, CVMUSBReadoutList::a24UserData,&lodata);
        if (status<0) {
            std::cerr << "CCBD8210CrateController::simpleRead32(int,int,int,uint32_t&,uint16_t&)";
            std::cerr << " failed vmeRead32 with error = " << status;
            std::cerr << std::endl;
            std::cerr << "b = " << m_branch << " c = " << m_crate
                      << " n = " << n << " a = " << a << " f = " << f
                      << std::endl;
        }

        data  = (  lodata & 0xffff );
        data |= ( (hidata & 0xff)<<16 );

        bool q = qTest();
        bool x = xTest();
        qx = formXQ(x,q);
    }
    return status;
}

int CCBD8210CrateController::simpleControl(int n, int a, int f, uint16_t& qx)
{
    uint32_t dummy;
    return simpleRead24(n,a,f,dummy,qx);
}


bool CCBD8210CrateController::qTest() 
{
    bool flag = false;
    if (m_ctlr) {

        uint32_t addr = CCBD8210CamacBranchDriver::convertToAddress(m_branch,0,29,0,0);
        uint32_t word16bit = CCBD8210CamacBranchDriver::BIT16;
        addr = word16bit | addr;

        uint16_t res = 0;
        int status = m_ctlr->vmeRead16(addr, CVMUSBReadoutList::a24UserData, &res);
        if (status<0) {
            std::cerr << "CCBD8210CrateController::qTest()";
            std::cerr << " failed vmeRead16 with error = " << status;
            std::cerr << std::endl;
        }
        res &= CCBD8210CamacBranchDriver::QMask;
        flag = (res > 0);
    }
    return flag;
}

bool CCBD8210CrateController::xTest() 
{
    bool flag = false;
    if (m_ctlr) {

        uint32_t addr = CCBD8210CamacBranchDriver::convertToAddress(m_branch,0,29,0,0);
        uint32_t word16bit = CCBD8210CamacBranchDriver::BIT16;
        addr = word16bit | addr;

        uint16_t res = 0;
        int status = m_ctlr->vmeRead16(addr, CVMUSBReadoutList::a24UserData, &res);
        if (status<0) {
            std::cerr << "CCBD8210CrateController::qTest()";
            std::cerr << " failed vmeRead16 with error = " << status;
            std::cerr << std::endl;
        }
        res &= CCBD8210CamacBranchDriver::XMask;
        flag = (res > 0);
    }
    return flag;
}

int CCBD8210CrateController::executeList(CCamacReadoutList& list, void* pbuffer, 
                                            size_t bufsize, size_t *nbytes)
{
    CCBD8210ReadoutList& cbdList = dynamic_cast<CCBD8210ReadoutList&>(list);

    int status=-10;
    if (m_ctlr) {
        CVMUSBReadoutList* vmeList = cbdList.getReadoutList();
    
        if (vmeList) {
            status = m_ctlr->executeList(*vmeList,pbuffer,bufsize,nbytes);
        } else {
            std::cerr << "CCBD8210CrateController::executeList(...) ";
            std::cerr << "list does not wrap around an existing CVMUSBReadoutList"; 
            std::cerr << std::endl;  
        }

    } 
    return status;
}

uint16_t CCBD8210CrateController::formXQ(uint16_t x, uint16_t q) const
{
  return ((x<<1) | q);
}
