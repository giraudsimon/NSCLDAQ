
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include "CConfiguration.h"
#include "CamacSlotLimits.h"


template<class Controller, class RdoList> 
bool CULMTrigger<Controller,RdoList>::isBooted=false;

template<class Controller, class RdoList>
CULMTrigger<Controller,RdoList>::CULMTrigger() 
  :
    m_pConfig(0)
{
}


template<class Controller, class RdoList>
CULMTrigger<Controller,RdoList>::CULMTrigger(const CULMTrigger& rhs)
    : CReadoutHardwareT<Controller,RdoList>(rhs)
{
//    if (rhs.m_pConfig) {
//        m_pConfig = new CReadoutModule(*(rhs.m_pConfig));
//    }

}

template<class Controller, class RdoList>
CULMTrigger<Controller,RdoList>& 
CULMTrigger<Controller,RdoList>::operator=(const CULMTrigger& rhs)
{
    return *this;
}

template<class Controller, class RdoList>
CULMTrigger<Controller,RdoList>::~CULMTrigger()
{
//    if(m_pConfig) delete m_pConfig;
}


template<class Controller, class RdoList>
void  CULMTrigger<Controller,RdoList>::onAttach(CReadoutModule& config)
{
//    std::cout << "Attaching CULMTrigger" << std::endl;
    m_pConfig = &config;
    m_pConfig->addParameter("-slot",
                            XXUSB::CConfigurableObject::isInteger,
                            &SlotLimits,
                            "1");

    m_pConfig->addParameter("-firmware",
            validFirmwareFile, NULL, "");

    m_pConfig->addIntegerParameter("-pcDelay",0);
    m_pConfig->addIntegerParameter("-pcWidth",0);
    m_pConfig->addIntegerParameter("-scDelay",0);
    m_pConfig->addIntegerParameter("-scWidth",0);
    m_pConfig->addIntegerParameter("-psDelay",0);
    m_pConfig->addIntegerParameter("-ccWidth",0);
    m_pConfig->addIntegerParameter("-ssDelay",0);

    m_pConfig->addIntegerParameter("-bypasses",0);

    m_pConfig->addIntegerParameter("-pdFactor",0);
    m_pConfig->addIntegerParameter("-sdFactor",0);
    m_pConfig->addIntegerParameter("-triggerBox",0);

    m_pConfig->addIntegerParameter("-inspect1",0);
    m_pConfig->addIntegerParameter("-inspect2",0);
    m_pConfig->addIntegerParameter("-inspect3",0);
    m_pConfig->addIntegerParameter("-inspect4",0);

    m_pConfig->addIntegerParameter("-adcWidth",0);
    m_pConfig->addIntegerParameter("-qdcWidth",0);
    m_pConfig->addIntegerParameter("-tdcWidth",0);

    m_pConfig->addIntegerParameter("-coincWidth",0);

    m_pConfig->addIntegerParameter("-configuration",0);


    m_pConfig->addBooleanParameter("-forceFirmwareLoad",true);
    m_pConfig->addBooleanParameter("-readRegister",true);
    m_pConfig->addBooleanParameter("-eventwiseClear",true);

}

template<class Controller, class RdoList>
void CULMTrigger<Controller,RdoList>::Initialize(Controller& controller)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    std::string firmwareName= m_pConfig->cget("-firmware"); 

    bool forceFirmwareLoad = m_pConfig->getBoolParameter("-forceFirmwareLoad");

    uint16_t qx=0;
    int status = 0;
    if (forceFirmwareLoad || ! isConfigured(controller)) {
        //    status = loadFirmware2(controller, firmwareName); 
        status = loadFirmware(controller, firmwareName);

        if (status<0) {
            std::ostringstream msg;
            msg << "CULMTrigger loading failed...returned error code=" 
                << status;
            msg << std::endl;
            throw msg.str();
        }
        isBooted = isConfigured(controller);

    }

    // Disable by vetoing the trigger and the tstamp clock signals
    setGo(controller,0);
    doClear(controller);
    setEnable(controller,0);

    int val;
    val = m_pConfig->getIntegerParameter("-pcDelay");
    controller.simpleWrite16(slot,0,16,uint16_t(val),qx);
    handleQX(qx,slot,0,16);

    val = m_pConfig->getIntegerParameter("-pcWidth");
    controller.simpleWrite16(slot,1,16,uint16_t(val),qx);
    handleQX(qx,slot,1,16);

    val = m_pConfig->getIntegerParameter("-scDelay");
    controller.simpleWrite16(slot,2,16,uint16_t(val),qx);
    handleQX(qx,slot,2,16);

    val = m_pConfig->getIntegerParameter("-scWidth");
    controller.simpleWrite16(slot,3,16,uint16_t(val),qx);
    handleQX(qx,slot,3,16);

    val = m_pConfig->getIntegerParameter("-psDelay");
    controller.simpleWrite16(slot,4,16,uint16_t(val),qx);
    handleQX(qx,slot,4,16);

    val = m_pConfig->getIntegerParameter("-ccWidth");
    controller.simpleWrite16(slot,5,16,uint16_t(val),qx);
    handleQX(qx,slot,5,16);

    val = m_pConfig->getIntegerParameter("-ssDelay");
    controller.simpleWrite16(slot,6,16,uint16_t(val),qx);
    handleQX(qx,slot,6,16);

    val = m_pConfig->getIntegerParameter("-bypasses");
    controller.simpleWrite16(slot,7,16,uint16_t(val),qx);
    handleQX(qx,slot,7,16);

    val = m_pConfig->getIntegerParameter("-pdFactor");
    controller.simpleWrite16(slot,8,16,uint16_t(val),qx);
    handleQX(qx,slot,8,16);

    val = m_pConfig->getIntegerParameter("-sdFactor");
    controller.simpleWrite16(slot,9,16,uint16_t(val),qx);
    handleQX(qx,slot,9,16);

    val = m_pConfig->getIntegerParameter("-triggerBox");
    controller.simpleWrite16(slot,10,16,uint16_t(val),qx);
    handleQX(qx,slot,10,16);

    val = m_pConfig->getIntegerParameter("-inspect1");
    controller.simpleWrite16(slot,0,17,uint16_t(val),qx);
    handleQX(qx,slot,0,17);

    val = m_pConfig->getIntegerParameter("-inspect2");
    controller.simpleWrite16(slot,1,17,uint16_t(val),qx);
    handleQX(qx,slot,1,17);

    val = m_pConfig->getIntegerParameter("-inspect3");
    controller.simpleWrite16(slot,2,17,uint16_t(val),qx);
    handleQX(qx,slot,2,17);

    val = m_pConfig->getIntegerParameter("-inspect4");
    controller.simpleWrite16(slot,3,17,uint16_t(val),qx);
    handleQX(qx,slot,3,17);

    val = m_pConfig->getIntegerParameter("-adcWidth");
    controller.simpleWrite16(slot,0,18,uint16_t(val),qx);
    handleQX(qx,slot,0,18);

    val = m_pConfig->getIntegerParameter("-qdcWidth");
    controller.simpleWrite16(slot,1,18,uint16_t(val),qx);
    handleQX(qx,slot,1,18);

    val = m_pConfig->getIntegerParameter("-tdcWidth");
    controller.simpleWrite16(slot,2,18,uint16_t(val),qx);
    handleQX(qx,slot,2,18);

    val = m_pConfig->getIntegerParameter("-coincWidth");
    controller.simpleWrite16(slot,3,18,uint16_t(val),qx);
    handleQX(qx,slot,3,18);

    setTStampMode(controller, EXTERNALLATCH);

    doClear(controller);
    setGo(controller,1);

}

template<class Controller, class RdoList>
void CULMTrigger<Controller,RdoList>::addReadoutList(RdoList& list)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    // read trigger register
    if (m_pConfig->getBoolParameter("-readRegister")) {
      addRegisterRead(list);
    }

    // clear the register
    if (m_pConfig->getBoolParameter("-eventwiseClear")) {
      addRegisterClear(list);
    }
}



template<class Controller, class RdoList>
void CULMTrigger<Controller,RdoList>::onEndRun(Controller& controller)
{
  setGo(controller,0);
  doClear(controller);
}

template<class Controller, class RdoList>
int 
CULMTrigger<Controller,RdoList>::loadFirmware2(Controller& controller, 
                                              const std::string& filename)
{

    int slot = m_pConfig->getIntegerParameter("-slot"); 

    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        std::cout << "!!! CULMTrigger::loadFirmware(Controller&,const std::string&)";
        std::cout << "\n    Cannot open firmware file " << filename << std::endl;

        throw CBadFirmwareFile(filename);  
    }

    std::cout << "    Configuration file: " << filename << std::endl;

    unsigned char c;
    // Find first 0xff in the file
    // because c is signed, 0xff is -1
    int index=0;
    do {
       c = file.get();
       ++index;
    } while( c!=0xff && file.good() ); 
    if (c!=0xff) {
        std::cout << "Ended with file.good()= " << file.good();
        std::cout << "\nEnded at tellg = " << file.tellg();
        std::cout << std::endl;    
    }

    uint16_t qx=0;
    // set in general programming mode
    controller.simpleControl(slot,0,30,qx);
    // select configuration mode
    controller.simpleControl(slot,0,28,qx);
    // program xilinx chip
    controller.simpleControl(slot,0,25,qx);

    // test for XilinX chip readiness
    int timeout=100;
    int nAttempts=0;
    do {
      controller.simpleControl(slot,0,14,qx);
      ++nAttempts;
    } while ((qx&0x1)==0 && nAttempts==timeout); 

    if (nAttempts==timeout) {
        std::cerr << "CULMTrigger failed to become ready" << std::endl; 
        return -10;
    } 

    // ............................................................
    // Begin reading true data... this is the start of the header
   // at this point the ULM should be ready
    unsigned int nBytes=0, nCount=0;

    nBytes = determineNBytes(controller,file);
    c = file.get();

    std::cout.precision(2);
    std::cout.flags(std::ios::fixed);

    // load body of firmware into rdolist
    RdoList* body = controller.createReadoutList();
    uint16_t dummy;
    size_t nbytes;
    int status = -100;
    
    uint16_t data = c;  
	do {
		body->addWrite16(slot,0,16, c);
		body->addControl(slot,0,13);
		//usleep(1);
		c = file.get();
        data = c;
		nCount++;
		//printf("%d bytes loaded\n", nCount);

    if (nCount%(nBytes/10)==0) {
      std::cout << "\r    Completed " << std::setw(6) << (100.0*nCount)/nBytes << "% " << nCount << std::flush;
    }

		if (nCount > nBytes) {
            std::cerr << "CULMTrigger is reading more bytes than the file contains!";
            std::cout << std::endl;
        }
        
        // can only handle 100 camac commands per list...we play it safe with 100
        if (nCount%100==0) {
            // execute rdolist
            status = controller.executeList(*body,(void*)&dummy, sizeof(dummy),&nbytes);
            if (status<0) {
                std::cerr << "loadfirmware2 failed at body" << std::endl;
                return status;
            }
//            if ((nBytes-nCount) < 5000) {
//                controller.simpleControl(1,0,9,qx);
//            }
            body->clear();
        }
    } while (file.good() && nCount!=30992);

    if (body->size()>0) {
        // execute rdolist
        status = controller.executeList(*body,(void*)&dummy, sizeof(dummy),&nbytes);
        if (status<0) {
            std::cerr << "loadFirmware2 failed at body" << std::endl;
            return status;
        }
    }
    delete body;
    //.............................................................

    std::cout << "\nphew ... take a breather" << std::endl;
    sleep(3);   
 
    // check done
    timeout = 100;
	do {
		controller.simpleControl(slot,0,13,qx);
		usleep(1);
	} while ((qx&0x1)==0 && (timeout-- > 0));
	if (timeout == -1) {
		std::cerr << "timeout while CULMTrigger checking done" << std::endl;
 //       return -12;
	}
    
    // exit
	usleep(10);
	controller.simpleControl(slot,0,9,qx);  // exit programming mode
	usleep(10);
    file.close();

    return 0;

}

template<class Controller, class RdoList>
int 
CULMTrigger<Controller,RdoList>::loadFirmware(Controller& controller, 
                                              const std::string& filename) 
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 

    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        std::cout << "!!! CULMTrigger::loadFirmware(Controller&,const std::string&)";
        std::cout << "\n    Cannot open firmware file " << filename << std::endl;

        throw CBadFirmwareFile(filename);  
    }

    std::cout << "--- Configuration file : " << filename << std::endl;

    unsigned char c;
    // Find first 0xff in the file
    int index=0;
    do {
       c = file.get();
       ++index;
    } while( c!=0xff && file.good() );
    if (c!=0xff) {
        std::cout << "Iterated " << index << " times" << std::endl;    
        std::cout << "Ended with file.good()= " << file.good() << std::endl;    
        std::cout << "Ended at tellg = " << file.tellg() << std::endl;    
    }
    uint16_t qx=0;
    // set in general programming mode
    controller.simpleControl(slot,0,30,qx);
    // select configuration mode
    controller.simpleControl(slot,0,28,qx);
    // program xilinx chip
    controller.simpleControl(slot,0,25,qx);

    // test for XilinX chip readiness
    int timeout=100;
    do {
      controller.simpleControl(slot,0,14,qx);
      --timeout;
    } while ((qx&0x1)==0 && timeout>0); 
    

    if (timeout==0) {
        std::cerr << "CULMTrigger failed to become ready" << std::endl; 
        return -10;
    } 

    // at this point the ULM should be ready
    unsigned int actualBytes=0, nBytes=0, nCount=0;

    // Figure out the number of bytes...which actually serves little purpose.
    // Regardless, this extracts 3 bytes from the file and then peek at fourth.
//    nBytes = determineNBytes(controller,file);
    std::streampos curpos = file.tellg();
    file.seekg(0,std::ios::end);
    actualBytes = (file.tellg() - curpos); 
    nBytes = ((file.tellg() - curpos)/10)*10; // make it a nice integral of 10
    file.seekg(curpos);
//    c = file.get(); 
 
    std::cout << "Bytes left to read = " << nBytes << std::endl;
    std::cout.precision(2);
    std::cout.flags(std::ios::fixed);
   
    uint16_t data = c;
    do {
      controller.simpleWrite16(slot,0,16,data,qx);
      //		SendChar(controller, c);
      controller.simpleControl(slot,0,13,qx);
      if (qx&0x01) {
        // it is normal to be done b4 eof so dont print anything. 
        // in fact, if nCount==30991, at this point we ultimately take the exit.
        //            std::cerr << "CULMULMTrigger configuration done before end of file";
        //            std::cerr << std::endl;
        break;
      }
      c = file.get();

      data = c;
      nCount++;

      if (nCount%(nBytes/10)==0) {
        std::cout << "\r    Loading configuration " 
          << std::setw(6) << (100.0*nCount)/nBytes
          << "% complete  (Total bytes read = " << nCount << ")" 
          << std::flush;
      }

      if (nCount > actualBytes) {
        std::cerr << "\nCULMTrigger is reading more bytes than expected!";
        std::cerr << " (expected=" << actualBytes << ", currently at " << nCount << ")";
        std::cout << std::endl;
      }
    } while (file.good());

    std::cout << std::endl;
    usleep(1000);
    // test Xilinx DONE line until ready
    timeout = 1000; 
    int nAttempts = 0; 
    do {
      controller.simpleControl(slot,0,13,qx);
      handleQX(qx,slot,0,13);
      usleep(1);
    } while ((qx&0x1)==0 && (++nAttempts < timeout));
    if (nAttempts == timeout) {
      std::cerr << "timeout while CULMTrigger checking done" << std::endl;
      return -12;
    } 


    usleep(10);
    controller.simpleControl(slot,0,9,qx);  // exit programming mode
    usleep(10);
    file.close();

    return 0;
}


template<class Controller, class RdoList>
  unsigned int
CULMTrigger<Controller,RdoList>::determineNBytes(Controller& controller, 
    std::ifstream& file)
{	
  unsigned char c;
  unsigned int nBytes = 0;

    c = 0xff;

	SendChar(controller,c);
	c = file.get();
	SendChar(controller,c);
	nBytes = c & 0xf;
	c = file.get();
	SendChar(controller,c);
	nBytes = nBytes * 0x100 + c;
	c = file.get();
	SendChar(controller,c);
	nBytes = nBytes * 0x100 + c;

    // don't extract then next element but look at it.
	c = file.peek();
	nBytes = nBytes * 0x10 + (c & 0xf0) / 0x10;
	nBytes /=8;

//	std::cout << "ULMTrigger configuration contains " << nBytes << " bytes";
//    std::cout << std::endl;

    return nBytes;
}

template<class Controller, class RdoList>
void CULMTrigger<Controller,RdoList>::SendChar(Controller& ctlr, unsigned char c)
{	
  int slot = m_pConfig->getIntegerParameter("-slot"); 
  int timeout = 100;	

  uint16_t data = 0;
  uint16_t qx=0;
  do {
    ctlr.simpleControl(slot,0,12,qx);
    //usleep(1);
  } while ((qx&0x1)==0 && (timeout-- > 0));
  if (timeout == 0) {
    std::cerr << "timeout while CULMTrigger waiting to write";
    std::cerr << std::endl;
  } else {
    data = 0;
    data  |= (c & 0xff);
    ctlr.simpleWrite16(slot,0,16,data,qx);
    //usleep(1);
  }
}


template<class Controller, class RdoList>
bool
CULMTrigger<Controller,RdoList>::validFirmwareFile(std::string name, 
                                                   std::string value, 
                                                   void* arg)
{	
    int status = ::access(value.c_str(),R_OK);
    
    return (status==0);
}

template<class Controller, class RdoList>
bool CULMTrigger<Controller,RdoList>::isConfigured(Controller& controller)
{
    bool retflag=false;

    std::cout << "    Validating ULMTrigger configuration ... " << std::flush; 

    int slot = m_pConfig->getIntegerParameter("-slot"); 
    int configID = m_pConfig->getIntegerParameter("-configuration"); 

    uint16_t modID, config, qx;
    
    int nAttempts=0;
    int timeout=100;
    do {
    
        int status = controller.simpleRead16(slot,15,0, modID, qx);
        if (status<0) {
            std::cerr << "CULMTrigger::checkConfiguration(Controller&)";
            std::cerr << " read(" << slot <<",15,0) returned with code " << status;
            std::cerr << std::endl;
        }
    } while ((qx&0x1)==0 && (++nAttempts < timeout));

    if ((qx&0x1)==0) {
      std::cerr << "Timed out while reading module ID (" << slot << ",15,0)";
      std::cerr << " : q=" << (0x1&qx) << " x=" << (qx>>1) << std::endl;
    }
    
    nAttempts=0;
    do {

        int status = controller.simpleRead16(slot,14,0, config, qx);
        if (status<0) {
            std::cerr << "CULMTrigger::checkConfiguration(Controller&)";
            std::cerr << " read(" << slot << ",14,0) returned with code " << status;
            std::cerr << std::endl;
        }
    } while ((qx&0x1)==0 && (++nAttempts < timeout));

    if ((qx&0x1)==0) {
      std::cerr << "Timed out while reading configuration (" << slot << ",14,0)";
      std::cerr << " : q=" << (0x1&qx) << " x=" << (qx>>1) << std::endl;
    }


    if (modID==2367 && config==configID) {
        std::cout << "SUCCESS" << std::endl; 
        retflag=true;
    } else {
        std::cout << "FAILURE" << std::endl; 
        retflag=false;
        std::cout << "    - User specified -configuration (" << configID << ")"
            << " was not found. Instead found " << config;
        std::cout << "\n    - modID read was (" << modID << ")";
        std::cout <<  std::endl; 
    }

    return retflag;
}

template<class Controller, class RdoList>
int CULMTrigger<Controller,RdoList>::setGo(Controller& controller, bool onoff) 
{
    uint16_t qx=0;
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    return controller.simpleWrite16(slot,11,16,static_cast<uint16_t>(onoff),qx);
}

template<class Controller, class RdoList>
int CULMTrigger<Controller,RdoList>::setEnable(Controller& controller, uint16_t bits) 
{
    uint16_t qx=0;
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    return controller.simpleWrite16(slot,13,16, bits,qx);
}

template<class Controller, class RdoList>
int CULMTrigger<Controller,RdoList>::setTStampMode(Controller& controller, TStampMode mode) 
{
    uint16_t qx=0;
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    return controller.simpleWrite16(slot,12,16, static_cast<uint16_t>(mode), qx );
}

template<class Controller, class RdoList>
int CULMTrigger<Controller,RdoList>::getGo(Controller& controller, bool& onoff) 
{
    uint16_t qx=0;
    uint16_t ionoff = static_cast<uint16_t>(onoff);

    int slot = m_pConfig->getIntegerParameter("-slot"); 
    return controller.simpleRead16(slot,11,0,ionoff,qx);
}

template<class Controller, class RdoList>
int CULMTrigger<Controller,RdoList>::getTStampMode(Controller& controller, TStampMode& mode) 
{
    uint16_t qx=0;
    uint16_t modeval;
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    int status = controller.simpleRead16(slot,12,0, modeval, qx );

    if (status==0) {
        switch (modeval) {
            case EXTERNALCLOCK: 
                mode = EXTERNALCLOCK;
                break;

            case EXTERNALLATCH:
                mode = EXTERNALLATCH;
                break;
    
            default:
                std::cout << "CULMTrigger::getSelect(...) returned unknown timestamp ";
                std::cout << "mode (mode=" << static_cast<int>(modeval) << ")" << std::endl;
                break;
        }    

    }

    return status;
    
}

//////////////////////////////////////////////////////////
//  Register read
//

// Add a read of the register to the readoutlist 
template<class Controller, class RdoList>
void CULMTrigger<Controller,RdoList>::addRegisterRead(RdoList& list)
{
  int slot = m_pConfig->getIntegerParameter("-slot"); 
  list.addRead16(slot,0,3);
}

// Add a read of the register to the readoutlist 
template<class Controller, class RdoList>
int CULMTrigger<Controller,RdoList>::doRegisterRead(Controller& controller,
                                                    uint16_t& data )
{
  uint16_t qx = 0;
  int slot = m_pConfig->getIntegerParameter("-slot"); 
  int status = controller.simpleRead16(slot,0,3,data,qx);
  return 0;
}

////////////////////////////////////////////////////////////
// Module clear
// Does the following:
//  1. Clears latches
//  2. Trigger register
//  3. Timestamp clock

// Add a clear to the readoutlist 
template<class Controller, class RdoList>
void CULMTrigger<Controller,RdoList>::addClear(RdoList& list)
{
  int slot = m_pConfig->getIntegerParameter("-slot"); 
  list.addControl(slot,0,9);
}

template<class Controller, class RdoList>
int CULMTrigger<Controller,RdoList>::doClear(Controller& controller) 
{
    uint16_t qx=0;
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    return controller.simpleControl(slot,0,9,qx);
}

////////////////////////////////////////////////////////////
// Register clear
// Add a register clear to the readoutlist 
// The main difference between register clear and clear
template<class Controller, class RdoList>
void CULMTrigger<Controller,RdoList>::addRegisterClear(RdoList& list)
{
  int slot = m_pConfig->getIntegerParameter("-slot"); 
  list.addControl(slot,0,10);
}

// Add a register clear to the readoutlist 
// The main difference between register clear and clear
template<class Controller, class RdoList>
int CULMTrigger<Controller,RdoList>::doRegisterClear(Controller& controller)
{
  uint16_t qx=0;
  int slot = m_pConfig->getIntegerParameter("-slot"); 
  return controller.simpleControl(slot,0,10,qx);
}

template<class Controller, class RdoList>
void CULMTrigger<Controller,RdoList>::handleQX(uint16_t qx, int n, int a, int f) const
{
  if ((qx&0x1)==0 || (qx>>1)==0) {
    std::cerr << "Command (" << n << "," << a << "," << f << ")";
    std::cerr << " produced (Q,X) = (" << (0x1&qx) << "," << (qx>>1) << ")";
    std::cerr << std::endl;
  }
}
