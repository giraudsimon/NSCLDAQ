/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

// Implementation of the CVMUSB class.

#include <config.h>
#include "CVMUSBEthernet.h"
#include "CVMUSBReadoutList.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TCLList.h>
#include "CSocket.h"

#include <errno.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

// Constants:

// Identifying marks for the VM-usb:

static const short USB_WIENER_VENDOR_ID(0x16dc);
static const short USB_VMUSB_PRODUCT_ID(0xb);

// Bulk transfer endpoints

static const int ENDPOINT_OUT(2);
static const int ENDPOINT_IN(0x86);
// Timeouts:

static const int DEFAULT_TIMEOUT(2000);	// ms.



// Bits in the list target address word:

static const uint16_t TAVcsID0(1); // Bit mask of Stack id bit 0.
static const uint16_t TAVcsSel(2); // Bit mask to select list dnload
static const uint16_t TAVcsWrite(4); // Write bitmask.
static const uint16_t TAVcsIMMED(8); // Target the VCS immediately.
static const uint16_t TAVcsID1(0x10);
static const uint16_t TAVcsID2(0x20);
static const uint16_t TAVcsID12MASK(0x30); // Mask for top 2 id bits
static const uint16_t TAVcsID12SHIFT(4);

//   The following flag determines if enumerate needs to init the libusb:

static bool usbInitialized(false);

/////////////////////////////////////////////////////////////////////

static int ReadBlock(CSocket& sock, void* pData, size_t nBytes)
{
  char *d = reinterpret_cast<char*>(pData);
  int bytesRead = 0;

  while (nBytes) {
    int nRead = sock.Read(d, nBytes);
    if (nRead > 0) {
      nBytes    -= nRead;
      bytesRead += nRead;
      d         += nRead;
    }
    else if (nRead == 0) {
      break;			// EOF
    } else {
      if ((errno != EINTR) && (errno != EAGAIN) && (errno != EWOULDBLOCK)) {
	// return the data we have already:

	if (bytesRead > 0) return bytesRead;
	return -1;		// Error!!
      }
    }
  }
  return bytesRead;
}

////////////////////////////////////////////////////////////////////
/*!
  Construct the CVMUSBEthernet object.  This involves storing the
  device descriptor we are given, opening the device and
  claiming it.  Any errors are signalled via const char* exceptions.
  @param deviceName - Name of the VMUSB Module in the controlconfig.tcl file.
                      defaults to vmusb
  @param host   - Name of the host to which we are connecting.  This
                  defaults to localhost.
  @param port   - port port on which the tcl server is running.  This
                  defaults to 27000

  In a single machine environment you should not need to modify these
  settings... in a multisystem environment where you are remote from the
  Readout, you should typically only need to modify host.

  @note - A Tcl server must be running else an exception is thrown.

*/
CVMUSBEthernet::CVMUSBEthernet(string deviceName, string host, unsigned int port) :
  m_deviceName(deviceName),
  m_pSocket(0),
  m_pInterp(0),
  m_host(host),
  m_port(port)

{
  openServer();
}

////////////////////////////////////////////////////////////////
/*!
    Destruction of the interface involves shutting down the socket 
    to the server

*/
CVMUSBEthernet::~CVMUSBEthernet()
{
  if (m_pSocket) {
    m_pSocket->Shutdown();
    delete m_pSocket;
  }
  delete m_pInterp;
}

/**
 * reconnect
 *
 * Teardown and rebuild the connection to the server.
 * If I can read the firmware register, I don't need to do this:
 *   @return bool -true if a reconnection was needed, false if not.
 */
bool
CVMUSBEthernet::reconnect()
{
  try {
    int fwid = readFirmwareID();
    return false;                      // No need.
  }
  catch (...) {                        // Can't read fwreg - need to reconnect.
    m_pSocket->Shutdown();
    delete m_pSocket;
    delete m_pInterp;
    m_pSocket = 0;
    m_pInterp = 0;
  
    openServer();
    return true;
  }
}

/**
 * write the action register remotely:
 * 
 * @param value - the value to write to the action register.
 * 
 * @note - the action regiseter is writeonly.
 *
 */
void
CVMUSBEthernet::writeActionRegister(uint16_t value)
{

  // Build up the command as a list then an \n terminated command.

  CTCLObject commandObj;
  commandObj.Bind(m_pInterp);
  commandObj = "vmusb";
  commandObj += "writeactionregister";
  commandObj += static_cast<int>(value);

  string request = static_cast<string>(commandObj);
  request += "\n";

  // Send the command and get the response.


  m_pSocket->Write(request.c_str(), request.size());
  std::string replyLine;
  char replyChar;
  bool done = false;
  while (!done) {
    int nRead = m_pSocket->Read(&replyChar, 1);
    if (nRead != 0) {
      replyLine += replyChar;
    } else {
      throw std::string("Server closed channel in writeActionRegister");
    }
    if (replyChar == '\n') done=true;

  }
  if (replyLine.substr(0,3) != "OK:") {
    throw std::string("Server reply not OK in writeActionRegister");
  }

	
  
}

////////////////////////////////////////////////////////////////////
//////////////////////// Register operations ///////////////////////
////////////////////////////////////////////////////////////////////
//////////////////////
/*!
   Read the firmware id register.  This is register 0.
   Doing this is a matter of building a CVMUSBReadoutList to do the
   job and then submitting it for immediate execution.

   \return int
   \retval The value of the firmware Id register.

*/
int
CVMUSBEthernet::readFirmwareID()
{
    return readRegister(FIDRegister);
}

////////////////////////////////////////////////////////////////////////

/*!
   Write the global mode register (GMODERegister).
  \param value :uint16_t 
     The 16 bit global mode register value.
*/
void
CVMUSBEthernet::writeGlobalMode(uint16_t value)
{
    writeRegister(GMODERegister, static_cast<uint32_t>(value));
}

/*!
    Read the global mode register (GMODERegistesr).
    \return int
    \retval the value of the register.
*/
int
CVMUSBEthernet::readGlobalMode()
{
    return static_cast<uint16_t>(readRegister(GMODERegister));
}

/////////////////////////////////////////////////////////////////////////

/*!
   Write the data acquisition settings register.
  (DAQSetRegister)
  \param value : uint32_t
     The value to write to the register
*/
void
CVMUSBEthernet::writeDAQSettings(uint32_t value)
{
    writeRegister(DAQSetRegister, value);
}
/*!
  Read the data acquisition settings register.
  \return uint32_t
  \retval The value read from the register.
*/
int
CVMUSBEthernet::readDAQSettings()
{
    return readRegister(DAQSetRegister);
}
//////////////////////////////////////////////////////////////////////////
/*!
  Write the LED source register.  All of the LEDs on the VMUSB
  are essentially user programmable.   The LED Source register determines
  which LEDS display what.  The register is made up of a bunch of
  3 bit fields, invert and latch bits.  These fields are defined by the
  constants in the CVMUSBEthernet::LedSourceRegister class.  For example
  CVMUSBEthernet::LedSourceRegister::topYellowInFifoFull ored in to this register
  will make the top yellow led light when the input fifo is full.


  \param value : uint32_t
     The value to write to the LED Src register.
*/
void
CVMUSBEthernet::writeLEDSource(uint32_t value)
{
    writeRegister(LEDSrcRegister, value);
}
/*!
   Read the LED Source register.  
   \return int
   \retval The current value of the LED source register.
*/
int
CVMUSBEthernet::readLEDSource()
{
    return readRegister(LEDSrcRegister);
}
/////////////////////////////////////////////////////////////////////////
/*!
   Write the device source register.  This register determines the
   source of the start for the gate and delay generators. What makes
   scalers increment and which signals are routed to the NIM out
   lemo connectors.
   \param value :uint32_t
      The new value to write to this register.   This is a bitmask that
      consists of code fields along with latch and invert bits.
      These bits are defined in the CVMUSBEthernet::DeviceSourceRegister
      class.  e.g. CVMUSBEthernet::DeviceSourceRegister::nimO2UsbTrigger1 
      ored into value will cause the O2 to pulse when a USB initiated
      trigger is produced.
*/
void
CVMUSBEthernet::writeDeviceSource(uint32_t value)
{
    writeRegister(DEVSrcRegister, value);
}
/*!
   Read the device source register.
   \return int
   \retval current value of the register.
*/
int
CVMUSBEthernet::readDeviceSource()
{
    return readRegister(DEVSrcRegister);
}
/////////////////////////////////////////////////////////////////////////
/*!
     Write the gate width and delay for delay and gate generator A
     \param value : uint32_t 
       The gate and delay generator value.  This value is split into 
       two fields, the gate width and delay.  Note that the width is
       further modified by the value written to the DGGExtended register.
*/
void
CVMUSBEthernet::writeDGG_A(uint32_t value)
{
    writeRegister(DGGARegister, value);
}
/*!
   Read the register controlling the delay and fine width of 
   Delay and Gate register A.
   \return uint32_t
   \retval  register value.
*/
uint32_t
CVMUSBEthernet::readDGG_A()
{
    return readRegister(DGGARegister);
}
/*!
  Write the gate with and delay for the B dgg. See writeDGG_A for
  details.
*/
void
CVMUSBEthernet::writeDGG_B(uint32_t value)
{
    writeRegister(DGGBRegister, value);
}
/*!
   Reads the control register for the B channel DGG.  See readDGG_A
   for details.
*/
 uint32_t
 CVMUSBEthernet::readDGG_B()
{
    return readRegister(DGGBRegister);
}

/*!
   Write the DGG Extension register.  This register was added
   to provide longer gate widths.  It contains the high order
   bits of the widths for both the A and B channels of DGG.
*/
void
CVMUSBEthernet::writeDGG_Extended(uint32_t value)
{
    writeRegister(DGGExtended, value);
}
/*!
   Read the value of the DGG extension register.
*/
uint32_t
CVMUSBEthernet::readDGG_Extended()
{
    return readRegister(DGGExtended);
}

//////////////////////////////////////////////////////////////////////////
/*!
   Read the A scaler channel
   \return uint32_t
   \retval the value read from the A channel.
*/
uint32_t
CVMUSBEthernet::readScalerA()
{
    return readRegister(ScalerA);
}
/*!
   Read the B scaler channel
*/
uint32_t
CVMUSBEthernet::readScalerB()
{
    return readRegister(ScalerB);
}


/////////////////////////////////////////////////////////////////////

/*!
    Write an interrupt service vector register.  The
    ISV's allow the application to specific a list to be associated
    with specific VME interrupts.  Each ISV register
    contains a pair of ISV specifications. See the
    CVMSUSB::ISVregister class for bit/mask/shift definitions in this
    register.
    \param which : int
      Specifies which of the ISV registers to write.  This is a value
      between 1 and 4.  1 is ISV12, 2 ISV34 ... 4 ISV78.
    \param value : uint32_t
       The new value to write to the register.

    \throw string  - if the which value is illegal.
*/
void
CVMUSBEthernet::writeVector(int which, uint32_t value)
{
    unsigned int regno = whichToISV(which);
    writeRegister(regno, value);
    
}


/*!
  Read an interrupt service vector register.  See writeVector for
  a discussion that describes these registers.
  \param which : int
      Specifies the ISV register to to read.
  \return int
  \retval the register contents.
 
  \throw string  - if thwhich is illegal.
*/
int
CVMUSBEthernet::readVector(int which)
{
    unsigned int regno = whichToISV(which);
    return readRegister(regno);
}

/*!
   Write the interrupt mask.  This 8 bit register has bits that are set for
   each interrupt level that should be ignored by the VM-USB.
   @param mask Interrupt mask to write.
*/
void
CVMUSBEthernet::writeIrqMask(uint8_t mask)
{
  // Well this is a typical VM-USB abomination:
  // - First save the global (mode?) register as we're going to wipe it out
  // - Next write the irqmask | 0x8000 to the glboal mode register.
  // - Write a USB trigger to the action register.
  // - Clear the action register(?)
  // - Write 0 to the global mode register.
  // - restore the saved global mode register.

  uint16_t oldGlobalMode = readGlobalMode();
  uint16_t gblmask      = mask;
  gblmask              |= 0x8000;
  writeGlobalMode(gblmask);
  writeActionRegister(2);	// Hopefully bit 1 numbered from zero not one.
  uint32_t maskValue         = readFirmwareID(); // should have the mask.
  writeGlobalMode(0);			    // Turn off this nonesense

  writeGlobalMode(oldGlobalMode); // restor the old globalmode.

  m_irqMask = mask;
}
/*!
   Read the interrupt mask.  This 8 bit register has bits set for each
   interrupt level that should be ignoered by the VM-USB.
   @return uint8_t
   @retval contents of the mask register.
*/;
int
CVMUSBEthernet::readIrqMask()
{
  // Since the interrupt mask register appears cleverly crafted so that you
  // can't actually read it without destroying it, we're just going to use
  // a copy of the value:

  return m_irqMask;
}

///////////////////////////////////////////////////////////////////////
/*!
    write the bluk transfer setup register.  This register
    sets up a few of the late breaking data taking parameters
    that are built to allow data to flow through the USB more
    effectively.  For bit/mask/shift-count definitions of this
    register see CVMUSBEthernet::TransferSetup.
    \param value : uint32_t
      The value to write to the register.
*/
void
CVMUSBEthernet::writeBulkXferSetup(uint32_t value)
{
    writeRegister(USBSetup, value);
}
/*!
   Read the bulk transfer setup register.
*/
int
CVMUSBEthernet::readBulkXferSetup()
{
    return readRegister(USBSetup);
}
/////////////////////////////////////////////////////////////////////////
/////////////////////////// VME Transfer Ops ////////////////////////////
/////////////////////////////////////////////////////////////////////////

/*!
   Write a 32 bit word to the VME for some specific address modifier.
   This is done by creating a list, inserting a single
   32bit write into it and executing the list.
   \param address  : uint32_t 
      Address to which the write is done.
   \param aModifier : uint8_t
      Address modifier for the operation.  This should not be a block transfer
      address modifier... those may not work for single shots and in any
      event would not pay.
    \param data   : uint32_t
      The datum to write.
 
   \return int
   \retval 0   - Success.
   \retval -1  - USB write failed.
   \retval -2  - USB read failed.
   \retval -3  - VME Bus error.

    \note In case of failure, errno, contains the reason.
*/
int
CVMUSBEthernet::vmeWrite32(uint32_t address, uint8_t aModifier, uint32_t data)
{
  CVMUSBReadoutList list;

  list.addWrite32(address, aModifier, data);
  return doVMEWrite(list);

}
/*!
   Write a 16 bit word to the VME.  This is identical to vmeWrite32,
   however the data is 16 bits wide.
*/
int
CVMUSBEthernet::vmeWrite16(uint32_t address, uint8_t aModifier, uint16_t data)
{
  CVMUSBReadoutList list;
  list.addWrite16(address, aModifier, data);
  return doVMEWrite(list);
}
/*!
  Do an 8 bit write to the VME bus.
*/
int
CVMUSBEthernet::vmeWrite8(uint32_t address, uint8_t aModifier, uint8_t data)
{
  CVMUSBReadoutList list;
  list.addWrite8(address, aModifier, data);
  return doVMEWrite(list);
}

/*!
   Read a 32 bit word from the VME.  This is done by creating a list,
   inserting a single VME read operation in it and executing the list in 
   immediate mode.
   \param address : uint32_t
      The address to read.
   \param amod    : uint8_t
      The address modifier that selects the address space used for the
      transfer.  You should not use a block transfer modifier for this.
   \param data    : uint32_t*
      Pointer to the location in which the data will be placed.

   \return int
    \retval  0    - All went well.
    \retval -1    - The usb_bulk_write failed.
    \retval -2    - The usb_bulk_read failed.
*/
int
CVMUSBEthernet::vmeRead32(uint32_t address, uint8_t aModifier, uint32_t* data)
{
  CVMUSBReadoutList list;
  list.addRead32(address, aModifier);
  uint32_t      lData;
  int status = doVMERead(list, &lData);
  *data      = lData;
  return status;
}

/*!
    Read a 16 bit word from the VME.  This is just like the previous
    function but the data transfer width is 16 bits.
*/
int
CVMUSBEthernet::vmeRead16(uint32_t address, uint8_t aModifier, uint16_t* data)
{
  CVMUSBReadoutList list;
  list.addRead16(address, aModifier);
  uint32_t lData;
  int      status = doVMERead(list, &lData);
  *data = static_cast<uint16_t>(lData);

  return status;
}
/*!
   Read an 8 bit byte from the VME... see vmeRead32 for information about
   this.
*/
int
CVMUSBEthernet::vmeRead8(uint32_t address, uint8_t aModifier, uint8_t* data)
{
  CVMUSBReadoutList list;
  list.addRead8(address, aModifier);
  uint32_t lData;
  int      status = doVMERead(list, &lData);
  *data  = static_cast<uint8_t>(lData);
  return status;
}

//////////////////////////////////////////////////////////////////////////
/////////////////////////// Block read operations ////////////////////////
/////////////////////////////////////////////////////////////////////////

/*!
    Read a block of longwords from the VME.  It is the caller's responsibility
    to:
    - Ensure that the starting address of the transfer is block aligned
      (that is a multiple of 0x100).
    - That the address modifier is  block transfer modifier such as
      CVMUSBReadoutList::a32UserBlock.
    The list construction takes care of the case where the count spans block
    boundaries or requires a trailing partial block transfer.  Note that
    transfers are done in 32 bit width.

    \param baseAddress : uint32_t
      Fisrt transfer address of the transfer.
    \param aModifier   : uint8_t
      Address modifier of the transfer.  See above for restrictions.
    \param data        : void*
       Buffer to hold the data read.
    \param transferCount : size_t
       Number of \em Longwords to transfer.
    \param countTransferred : size_t*
        Pointer to a size_t into which will be written the number of longwords
        actually transferred.  This can be used to determine if the list
        exited prematurely (e.g. due to a bus error).

     \return int
     \retval 0  - Successful completion.  Note that a BERR in the middle
                  of the transfer is successful completion.  The countTransferred
                  will be less than transferCount in that case.
     \retval -1 - Failure on usb_bulk_write, the actual cause of the error is
                  in errno, countTransferred will be 0.
     \retval -2 - Failure on the usb_bulk_write, The actual error will be in 
                  errno.  countTransferred will be 0.

*/
int
CVMUSBEthernet::vmeBlockRead(uint32_t baseAddress, uint8_t aModifier,
		     void*    data,        size_t transferCount, 
		     size_t*  countTransferred)
{
  // Create the list:

  CVMUSBReadoutList list;
  list.addBlockRead32(baseAddress, aModifier, transferCount);


  *countTransferred = 0;
  int status = executeList(list, data, transferCount*sizeof(uint32_t),
			   countTransferred);
  *countTransferred = *countTransferred/sizeof(uint32_t); // bytes -> transfers.
  return status;
}
/*!
   Do a 32 bit wide block read from a FIFO.  The only difference between
   this and vmeBlockRead is that the address read from is the same for the
   entire block transfer.
*/
int 
CVMUSBEthernet::vmeFifoRead(uint32_t address, uint8_t aModifier,
		    void*    data,    size_t transferCount, size_t* countTransferred)
{
  CVMUSBReadoutList list;
  list.addFifoRead32(address, aModifier, transferCount);

  *countTransferred = 0;
  int status =  executeList(list, data, transferCount*sizeof(uint32_t), 
			    countTransferred);
  *countTransferred = *countTransferred/sizeof(uint32_t); // bytes -> transferrs.
  return status;
}


//////////////////////////////////////////////////////////////////////////
/////////////////////////// List operations  ////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
/*!
    Execute a list immediately.  It is the caller's responsibility
    to ensure that no data taking is in progress and that data taking
    has run down (the last buffer was received).  
    The list is transformed into an out packet to the VMUSB and
    transaction is called to execute it and to get the response back.
    \param list  : CVMUSBReadoutList&
       A reference to the list of operations to execute.
    \param pReadBuffer : void*
       A pointer to the buffer that will receive the reply data.
    \param readBufferSize : size_t
       number of bytes of data available to the pReadBuffer.
    \param bytesRead : size_t*
       Return value to hold the number of bytes actually read.
 
    \return int
    \retval  0    - All went well.
    \retval -1    - The Set to the server failed.
    \retval -2    - The Receive of data from the server indicated an error.
    \retval -3    - The server returned an error, use getLastError to retrieve it.

    In case of failure, the reason for failure is stored in the
    errno global variable.


*/
int
CVMUSBEthernet::executeList(CVMUSBReadoutList&     list,
		   void*                  pReadoutBuffer,
		   size_t                 readBufferSize,
		   size_t*                bytesRead)
{

  m_lastError = "";

  string vmeList      = marshallList(list);
  CTCLObject datalist;
  datalist.Bind(m_pInterp);
  datalist += static_cast<int>(readBufferSize);
  datalist += vmeList;


  // Send the list to the remote with a "\n" on the back end of it:

  CTCLObject command;		// A command is the string rep of a list after all!
  command.Bind(m_pInterp);
  command = "vmusb";
  command += "immediatelist";
  command += static_cast<int>(readBufferSize);
  command += vmeList;



  string request = static_cast<string>(command);
  request       += "\n";
  try {
    m_pSocket->Write(request.c_str(), request.size());
  }
  catch(...) {
    return -1;
  }

  // Get the reply  data back.. each  of the read buffer can be at most
  // {127 } So allow 8 chars just for fun for each byte in allocating the receive buffer:
  // In getting the reply we keep reading until we get a \n in the input buffer.
  //
  char* response = new char[readBufferSize*8+256];
  size_t remaining = readBufferSize*8 + 200; // Safety buffer.
  int   offset   = 0;
  bool  done     = false;
  int   nread;
  while(!done && (remaining > 0)  ) {
    try {
       nread = m_pSocket->Read(&(response[offset]), readBufferSize*8);
    }
    catch (...) {
      delete []response;
      return -2;
    }
    response[offset+nread] = 0;	// Null terminate the data...
    if (strchr( response, '\n')) {
      done = true;
    } 
    else {
      offset    += nread;
      remaining -= nread;
    }
  }
  // The entire response is here... marshall the respones into the output buffer

  try {
    *bytesRead = marshallOutputData(pReadoutBuffer, response, readBufferSize);
  }
  catch (...) {
    delete []response;
    return -3;
  }
  delete[]response;
  return 0;
}

/**
 * loadList
 * 
 *  Load a list of VME operations for use online mode.  Lists get
 *  associated with triggers which in turn can make those lists execute
 *  autonomousl in DAQ modem, adding data to the data buffers.
 *
 * @param listNumber - Number of the list to load {[0-7]}.
 * @param list       - The CVMUSReaout list to marshall and load.
 * @param offset     - The offset in VMUSB list memory at which the
 *                     list will be loaded.
 * @return int  - 0 for success, -n for failure.
 */
int
CVMUSBEthernet::loadList(uint8_t listNumber, CVMUSBReadoutList& list, off_t offset)
{
  string vmeList = marshallList(list); // Stringized list.
  m_lastError    = "";

  // Build the command:

  CTCLObject command;
  command.Bind(m_pInterp);
  command += "vmusb";
  command += "load";
  command +=  static_cast<int>(listNumber);
  command +=  static_cast<int>(offset);
  command += vmeList;

  string request = static_cast<string>(command);
  request += "\n";

  try {
    m_pSocket->Write(request.c_str(), request.size());
  }
  catch(...) {
    return -1;
  }

  // Reply data should just be a line that is "OK" or "ERROR - the errormessage"

  std::string response;
  while(1) {
    try {
      char c;
      int nread = m_pSocket->Read(&c, sizeof(char));
      if (nread == 1) {
	response += c;
	if (c == '\n') break;
      }
      
    }
    catch(...) {
      return -2;
    }
  }
  // response is the entire reply msg, including \n>

  if (response.substr(0,3) == "OK:") {
    return 0;
  } else {
    m_lastError = response;
    return -3;
  }
}
/**
 *  usbRead
 *
 * Do a bulk read.
 * This requires a subcommand of "bulkread" which has parameters
 * size of the read and the timeout.
 * The return is a line containing the transfer count if success.
 * and a block of data.
 * On failure the thing after ERROR is the errno value.
 *
 * @param data[out] - Buffer in which the data will be put.
 * @param buffersize[in] - Amount of data being requested.
 * @param transfercount[out] - Number of bytes tranferred.
 * @param timeout[in]  - ms of timeout for the read.
 *
 * @return int
 * @retval 0 - success.
 * @retval -1 - Failed with errno containing the reason.
 */
int
CVMUSBEthernet::usbRead(void* pData, size_t bufferSize, size_t* transferCount,
			int timeout)
{
  CTCLObject command;
  command.Bind(m_pInterp);
  command += "vmusb";
  command += "bulkread";
  command += static_cast<int>(bufferSize);
  command += static_cast<int>(timeout);

  std::string request = command;
  request += "\n";
 
  try {
    m_pSocket->Write(request.c_str(), request.size());
  }
  catch(...) {
    return -1;
  }
  // Reply data should just be a line that is "OK" or "ERROR - the errormessage"

  std::string response;
  while(1) {
    try {
      char c;
      int nread = m_pSocket->Read(&c, sizeof(char));
      if (nread == 1) {
	response += c;
	if (c == '\n') break;
      }
      
    }
    catch(...) {
      return -2;
    }
  }
  // response is the entire reply msg, including \n>

  if (response.substr(0,3) == "OK:") {
    // The remainder of the line after "OK" is the transfer count:

    std::string xferCount = response.substr(3);
    int readBytes = atoi(xferCount.c_str());
    *transferCount = readBytes;
    int nRead = ReadBlock(*m_pSocket, pData, readBytes);
    if (nRead < 0) {
      return -1;
    }


    
    return 0;

  } else if (response.substr(0,5) == "FAIL:") {
    // Following "FAIL: " is what we should put in errno.
    
    errno = atoi(response.substr(5).c_str());
    return -2;
  } else {
    errno = EPROTO;
    return -2;
  }

  
}

////////////////////////////////////////////////////////////////////////
/////////////////////////////// Utility methods ////////////////////////
////////////////////////////////////////////////////////////////////////

/*
   Reading a register is just creating the appropriate CVMUSBReadoutList
   and executing it immediatly.
*/
uint32_t
CVMUSBEthernet::readRegister(unsigned int address)
{
    CVMUSBReadoutList list;
    uint32_t          data;
    size_t            count;
    list.addRegisterRead(address);

    int status = executeList(list,
			     &data,
			     sizeof(data),
			     &count);
    if (status == -1) {
	char message[100];
	sprintf(message, "CVMUSBEthernet::readRegister USB write failed: %s",
		strerror(errno));
	throw string(message);
    }
    if (status == -2) {
	char message[100];
	sprintf(message, "CVMUSBEthernet::readRegister USB read failed %s",
		strerror(errno));
	throw string(message);

    }


    return data;
			     

    
}
/*
  
   Writing a register is just creating the appropriate list and
   executing it immediately:
*/
void
CVMUSBEthernet::writeRegister(unsigned int address, uint32_t data)
{
    CVMUSBReadoutList list;
    uint32_t          rdstatus;
    size_t            rdcount;
    list.addRegisterWrite(address, data);

    int status = executeList(list,
			     &rdstatus, 
			     sizeof(rdstatus),
			     &rdcount);

    if (status == -1) {
	char message[100];
	sprintf(message, "CVMUSBEthernet::writeRegister USB write failed: %s",
		strerror(errno));
	throw string(message);
    }
    if (status == -2) {
	char message[100];
	sprintf(message, "CVMUSBEthernet::writeRegister USB read failed %s",
		strerror(errno));
	throw string(message);

    }
    
}


/**
 *  Marshall a CVMEReadoutList into a string that is a Tcl list of integers representing the
 *  contents of the list.
 *  @param list  - The CVMEReadoutList object.
 *  @return string
 *  @retval the Tcl list constructed from the vme readout list.
 */
string
CVMUSBEthernet::marshallList(CVMUSBReadoutList& list)
{
  vector<uint32_t> listVect = list.get();
  CTCLObject       TclList;
  TclList.Bind(m_pInterp);
  for (int i =0; i < listVect.size(); i++) {
    char item[100];
    sprintf(item, "0x%x", listVect[i]);
    TclList += item;
  }
  return (string)TclList;
}
/**
 *  Marshall a reply buffer from the server into the user's buffer.
 *  @param pOutputBuffer - Pointer to the  user's output buffer.
 *  @param reply         - Pointer to the server's reply, a Tcl list.
 *  @param maxOutputSize - Size of the output buffer... no more than this number of bytes will be
 *                         written to the output buffer.
 * @return size_t
 * @retval Actual number of bytes written to the output buffer.
 */
size_t
CVMUSBEthernet::marshallOutputData(void* pOutputBuffer, const char* reply, size_t maxOutputSize)
{
  uint8_t* o = reinterpret_cast<uint8_t*>(pOutputBuffer);

  //  throw the reply data into a list and split it:

  CTCLList tclList(m_pInterp, reply);
  StringArray list;
  tclList.Split(list);

  // The first element is the status.
  // if it is OK no problems. if it is ERROR set the error informatino.
  //

  if (list[0] == "ERROR") {
    m_lastError = reply;
    throw m_lastError;		// Error in reply.
  }

  // So the result is just the reply list:


  // Figure out how many bytes the user gets...

  size_t actualSize = list.size() - 1;
  if (actualSize > maxOutputSize) actualSize = maxOutputSize;

  // Each list element is an ascii encoded byte:

  for (int i = 1; i < actualSize; i++) {
    unsigned long aByte;
    aByte = strtoul(list[i].c_str(), NULL, 0);
    *o++ = static_cast<uint8_t>(aByte & 0xff);
  }

  return actualSize;
}


/**
 * openServer
 *    Given that the host and port members are set,
 *  open the server connection and 
 *  create the local interpreter.
 */
void
CVMUSBEthernet::openServer()
{
  try {
    char portNumber[100];
    sprintf(portNumber, "%d", m_port);
    m_pSocket = new CSocket;
    m_pSocket->Connect(m_host, string(portNumber));
    m_pInterp = new CTCLInterpreter();
  }
  catch (...) {			// Exception catch prevents memory leaks and...
    delete m_pSocket;
    m_pSocket = 0;
    delete m_pInterp;
    m_pInterp = 0;
    throw;			// lets the caller deal with the error.
  }
}

