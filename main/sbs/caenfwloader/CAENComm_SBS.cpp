/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/*
** @file CAENCOMM_NSCL.cpp
** @author Ron Fox
**
** This file implements much of the CAENComm library but for the 
** SBS/GE 612 fiber optic VME bus bridge.
**
*/

#define BT1003 1		// I think this is needed to get the SBS include file action.

#include "CAENComm.h"
#include <vector>
#include <btapi.h>
#include <stdint.h>
#include <string.h>
#include <linux/limits.h>	// not portable.


//////////////////////////////////////////////////////////////////////////
//
// File scoped definitions.
//



// The following struct holds the actual handle information:
//
typedef struct _NSCLHandle {
  bt_desc_t*    s_pSbsHandle;	// handle to the SBS board.
  uint32_t     s_nBaseAddress;  // Map base address for documentation sake.
  void*        s_pMappedMemory; // Pointer to memory map.
  bool         s_fValid;	// True if this is an open device.
} NSCLHandle, *pNSCLHandle;

// Since CAEN insists that handles be integers the vector below is used to
// hold handles.  At present we don't try to re-use old handles.
// since this is just to get their firmware loader to work.
//
static std::vector<NSCLHandle> gHandles;



// This is specific to the CAEN 1720 which has a 64k address window:

static const size_t gWindowSize=0x10000;

// Alignment masks:

static const uint32_t LONG_ALIGN_MASK(0xfffffffc);
static const uint32_t WORD_ALIGN_MASK(0xfffffffe);


///////////////////////////////////////////////////////////////////////////////
//
// Static utility functions (these don't require C bindings
//

/**
 * Return an index to a new handle available for use
 * @return int
 * @retval gHandles[return-value] is a new initialized handle
 * @note the handle is initialized with s_fValid as false.
 * @note no attempt is made to re-use closed handles.
 */
static int
newHandle()
{
  NSCLHandle templateHandle = {NULL, 0, NULL, false};
  gHandles.push_back(templateHandle);
  return gHandles.size() -1;
}

/**
 * Returns a pointer to the handle associated with a 
 * handle index:
 * @param idx  [in] Handle index.
 * @return pNSCLHandle
 * @retval Pointer to the associated handle
 * @throw int error code if the index is invalid.
 */
static pNSCLHandle
getHandle(int idx)
{
  if (idx < gHandles.size()) {
    pNSCLHandle result = &(gHandles[idx]);
    if (result->s_fValid) {
      return result;
    }
    else {
      throw CAENComm_InvalidHandler; // sic handle.
    }
  }
  else {
    throw CAENComm_InvalidHandler; // sic handle.
  }
}

/**
 * Validate a VME offset.
 * The address offset must be in the window and must be aligned as per the mask passed
 * in or a bad parameter error is thrown.
 * @param Address - candidate addresss.
 * @param mask    - Mask of bits that can be set in the address.
 *                  e.g. Address & mask != Adress is an error.
 * @throw int - bad parameter error if validity fails.
 */
static void
validateAddress(uint32_t Address, uint32_t mask)
{
  if ((Address >= gWindowSize) || ((Address &mask) != Address)) {
    throw CAENComm_InvalidParam;
  }
}


/**
 * Offset To Virtual Address:
 * Given a handle and an address offset produces a pointer to the corresponding
 * virtual address:
 * @param pHandle - the handle pointer (already validated)
 * @param offset  - the byte offset to apply.
 * @return void*
 * @retval virtual address specified by the offset/handle.
 */
static void*
virtualAddress(pNSCLHandle pHandle, uint32_t offset)
{
    uint8_t* pByteBase = reinterpret_cast<uint8_t*>(pHandle->s_pMappedMemory);
    return               reinterpret_cast<void*>(pByteBase + offset);

}

/**
 * Do a poke (everything is validated)
 * @param pHandle - pointer to handle struct.
 * @param Address - Byte offset.
 * @param value   - T value.
 */
template<class T>
void
poke(pNSCLHandle pHandle, uint32_t Address, T value)
{
    T* p = reinterpret_cast<T*>(virtualAddress(pHandle, Address));
    *p          = value;
  
}
/**
 * Do a peek from the VME
 * @param pHandle - Pointer to validated address struct.
 * @param Address - Validated byte offset.
 */
template<class T>
T 
peek(pNSCLHandle pHandle, uint32_t Address)
{
    T* p = reinterpret_cast<T*>(virtualAddress(pHandle, Address));

    return *p;
}
/////////////////////////////////////////////////////////////////////////////////
//
// Public interface.  These functions all have C external calling sequence.
//    Why bother with C++? Because excpetion handling makes error management so 
//    much simpler.
//

extern "C" {


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_OpenDevice(CAENComm_DeviceDescr dd, int *handle);
* \brief   Open the device
*
* \param	[IN] LinkType: the link used by the device(CAENComm_USB|CAENComm_PCI_OpticalLink|CAENComm_PCIE_OpticalLink|CAENComm_PCIE)
* \param	[IN] LinkNum: The link number
* \param	[IN] ConetNode: The board number in the link.
* \param	[IN] VMEBaseAddress: The VME base address of the board
* \param	[OUT] handle: device handler
* \return  0 = Success; negative numbers are error codes
* \note  We ignore the link type -- it's always going to be SBS
* \note  The LinkNum will be the SBS interface board.
* \note  The ConetNode will be ignored,.
* \note  We are always going to open BT_DEV_A32
* \note  SBS errors are converted to a generic error.
*
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_OpenDevice(CAENComm_ConnectionType LinkType,
					       int LinkNum, int ConetNode, 
					       uint32_t VMEBaseAddress, 
					       int *handle)
{
  CAENComm_ErrorCode status =CAENComm_Success;	// optimistic completion code.

  try {
    char deviceName[PATH_MAX+1];
    bt_desc_t*  btHandle = new bt_desc_t;;
    void*      pMap;
    
    // Generate our device name and attempt to open it:
    

    const char*  pName = bt_gen_name(LinkNum, BT_DEV_A32, deviceName, sizeof(deviceName));
    
    if (pName != deviceName) throw CAENComm_GenericError;

    bt_error_t ok = bt_open(btHandle, pName, BT_RDWR);
    if (ok != BT_SUCCESS) throw CAENComm_GenericError;

    // Attempt to map the address space:

    ok = bt_mmap(*btHandle, &pMap, 
		 VMEBaseAddress, gWindowSize, 
		 BT_RDWR, BT_SWAP_NONE);

    if(ok != BT_SUCCESS) throw CAENComm_GenericError;

    // Create the handle, fill it in, validate it and return its index to the caller.

    int iHandle              = newHandle();
    pNSCLHandle pHandle      = &(gHandles[iHandle]);
    pHandle->s_pSbsHandle    = btHandle;
    pHandle->s_nBaseAddress  = VMEBaseAddress;
    pHandle->s_pMappedMemory = pMap;
    pHandle->s_fValid        = true;
    *handle                  = iHandle;

  }
  catch(CAENComm_ErrorCode code) {
    status = code;
  }
  return status;
    

}

/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_CloseDevice(int handle);
* \brief   Close the device
*
* \param   [IN]  handle: device handler
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_CloseDevice(int handle)
{
  CAENComm_ErrorCode returnValue = CAENComm_Success; // Optimistic initialization.
  try {
    // Get the handle:

    pNSCLHandle pHandle = getHandle(handle);
    
    // Unmap memory and close the device.
    // ignoring errors for now.

    bt_unmmap(*(pHandle->s_pSbsHandle),
	      pHandle->s_pMappedMemory,
	      gWindowSize);
    bt_close(*(pHandle->s_pSbsHandle));

    // Mark the handle as invalid:

    pHandle->s_fValid = false;
    delete pHandle->s_pSbsHandle;
  }
  catch(CAENComm_ErrorCode code) {
    returnValue = code;
  }
  return returnValue;
}

/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_Write32(int handle, uint32_t Address, uint32_t *Data)
* \brief   Write a 32 bit register of the device
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offset
* \param   [IN]  Data: new register content to write into the device
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_Write32(int handle, uint32_t Address, uint32_t Data)
{
  CAENComm_ErrorCode result = CAENComm_Success;

  try {

    // Validations 

    pNSCLHandle pHandle = getHandle(handle);
    validateAddress(Address, LONG_ALIGN_MASK);
    
    poke<uint32_t>(pHandle, Address, Data);


  }
  catch (CAENComm_ErrorCode code) {
    result = code;
  }
  return result;
}
/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_Write16(int handle, uint32_t Address, uint16_t *Data)
* \brief   Write a 16 bit register of the device
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offset
* \param   [IN]  Data: new register content to write into the device
* \return  0 = Success; negative numbers are error codes
* \note If it weren't for the alignment validation I could have factored this out into
*       a fully templated poke function...could have pased the mask in I suppose.
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_Write16(int handle, uint32_t Address, uint16_t Data)
{
  CAENComm_ErrorCode result = CAENComm_Success;

  try {

    // Validations 

    pNSCLHandle pHandle = getHandle(handle);
    validateAddress(Address, WORD_ALIGN_MASK);
    
    poke<uint16_t>(pHandle, Address, Data);

  }
  catch (CAENComm_ErrorCode code) {
    result = code;
  }
  return result;

}
/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_Read32(int handle, uint32_t Address, uint32_t *Data)
* \brief   Read a 32 bit data from the specified register.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offset
* \param   [OUT] Data: The data read from the device
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_Read32(int handle, uint32_t Address, uint32_t *Data)
{
  CAENComm_ErrorCode result = CAENComm_Success;
  try {
    // Validations 

    pNSCLHandle pHandle = getHandle(handle);
    validateAddress(Address, LONG_ALIGN_MASK);
 
    *Data = peek<uint32_t>(pHandle, Address);

  }
  catch (CAENComm_ErrorCode code) {
    result = code;
  }
  return result;
}
/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_Read16(int handle, uint32_t Address, uint16_t *Data)
* \brief   Read a 16 bit data from the specified register.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offset
* \param   [OUT]  Data: The data read from the device
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_Read16(int handle, uint32_t Address, uint16_t *Data)
{
  CAENComm_ErrorCode result = CAENComm_Success;
  try {
    // Validations 

    pNSCLHandle pHandle = getHandle(handle);
    validateAddress(Address, LONG_ALIGN_MASK);
 
    *Data = peek<uint16_t>(pHandle, Address);

  }
  catch (CAENComm_ErrorCode code) {
    result = code;
  }
  return result;
}

/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_MultiRead32(int handle, uint32_t Address, int nCycles, uint32_t *data, CAENComm_ErrorCode *ErrorCode)
* \brief   The function performs a block of single 32bit ReadRegister.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offsets
* \param   [IN]  nCycles: the number of read to perform
* \param   [OUT] data: The datas read from the device
* \param   [OUT] ErrorCode: The error codes relaive to each cycle.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_MultiRead32(int handle, 
						uint32_t *Address, int nCycles, 
						uint32_t *data, 
						CAENComm_ErrorCode *ErrorCode)
{
  // going to do this the long and slow way:

  CAENComm_ErrorCode retval = CAENComm_Success;
  try {
    for (int i=0; i < nCycles; i++) {
      ErrorCode[i] = CAENComm_Read32(handle,
				     Address[i],
				     &(data[i]));
    }
  }
  catch (CAENComm_ErrorCode code) {
    retval = code;
  }
  return retval;
}
/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_MultiRead16(int handle, uint32_t *Address, int nCycles, uint32_t *data, CAENComm_ErrorCode *ErrorCode)
* \brief   The function performs a block of single 16bit ReadRegister.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offsets
* \param   [IN]  nCycles: the number of read to perform
* \param   [OUT] data: The datas read from the device
* \param   [OUT] ErrorCode: The error codes relaive to each cycle.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_MultiRead16(int handle, uint32_t *Address, int nCycles,
						uint16_t *data, CAENComm_ErrorCode *ErrorCode)
{
  // going to do this the long and slow way:

  CAENComm_ErrorCode retval = CAENComm_Success;
  try {
    for (int i=0; i < nCycles; i++) {
      ErrorCode[i] = CAENComm_Read16(handle,
				     Address[i],
				     &(data[i]));

    }
  }
  catch (CAENComm_ErrorCode code) {
    retval = code;
  }
  return retval;
}
/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_MultiWrite32(int handle, uint32_t *Address, int nCycles, uint32_t *data, CAENComm_ErrorCode *ErrorCode)
* \brief   The function performs a block of single 16bit WriteRegister.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offsets
* \param   [IN]  nCycles: the number of read to perform
* \param   [IN]  data: The datas to write to the device
* \param   [OUT] ErrorCode: The error codes relaive to each cycle.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_MultiWrite32(int handle, uint32_t *Address, int nCycles, 
					  uint32_t *data, CAENComm_ErrorCode *ErrorCode)
{
  CAENComm_ErrorCode retval = CAENComm_Success;

  try {
    for(int i=0; i < nCycles; i++) {
      ErrorCode[i] = CAENComm_Write32(handle, 
				      Address[i], data[i]);
    }
  }
  catch (CAENComm_ErrorCode code) {
    retval = code;
  }
  return retval;
}

/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_MultiWrite16(int handle, uint32_t *Address, int nCycles, uint32_t *data, CAENComm_ErrorCode *ErrorCode)
* \brief   The function performs a block of single 16bit WriteRegister.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offsets
* \param   [IN]  nCycles: the number of read to perform
* \param   [IN]  data: The datas to write to the device
* \param   [OUT] ErrorCode: The error codes relaive to each cycle.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_MultiWrite16(int handle, uint32_t *Address, 
						 int nCycles, uint16_t *data, 
						 CAENComm_ErrorCode *ErrorCode)
{
  CAENComm_ErrorCode retval = CAENComm_Success;

  try {
    for(int i=0; i < nCycles; i++) {
      ErrorCode[i] = CAENComm_Write16(handle, 
				      Address[i], data[i]);
    }
  }
  catch (CAENComm_ErrorCode code) {
    retval = code;
  }
  return retval;
}

//////////////////////////////////////////////////////////////////////////
// The remaining functions are unimplemented and therefore return
// CAENComm_NotSupported.
//

/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_BLTRead(int handle, uint32_t Address, uint32_t *Buff, int BltSize, int *nw)
* \brief   Read a block of data from the device using a BLT (32 bit) cycle
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: data space starting address
* \param   [IN]  BltSize: size of the Block Read Cycle (in bytes)
* \param   [OUT] buff: pointer to the data buffer
* \param   [OUT] nw: number of longwords (32 bit) actually read from the device
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_BLTRead(int handle, uint32_t Address, 
					    uint32_t *Buff, int BltSize, int *nw)
{
  return CAENComm_NotSupported;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_MBLTRead(int handle, uint32_t Address, uint32_t *Buff, int BltSize, int *nw)
* \brief   Read a block of data from the device using an MBLT (64 bit) cycle
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: data space starting address
* \param   [IN]  BltSize: size of the Block Read Cycle (in bytes)
* \param   [OUT] buff: pointer to the data buffer
* \param   [OUT] nw: number of longwords (32 bit) actually read from the device
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_MBLTRead(int handle, uint32_t Address, 
					     uint32_t *Buff, int BltSize, int *nw)
{
  return CAENComm_NotSupported;
}



/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_VMEIRQCheck(int handle, uint8_t *Mask)
* \brief   The function returns a bit mask indicating the active VME IRQ lines.

* \param   [IN]  handle: device handler
* \param   [OUT]  Mask: A bit-mask indicating the VME IRQ lines.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_VMEIRQCheck(int VMEhandle, uint8_t *Mask)
{
  return CAENComm_NotSupported;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_IRQDisable(int handle)
* \brief   The function disables the IRQ lines.

* \param   [IN]  handle: device handler
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_IRQDisable(int handle)
{
  return CAENComm_NotSupported;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_IRQEnable(int handle)
* \brief   The function enaables the IRQ lines.

* \param   [IN]  handle: device handler
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_IRQEnable(int handle)
{
  return CAENComm_NotSupported;
}



/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_VMEIACKCycle16(int VMEhandle, IRQLevels Level, int *BoardID)
* \brief   The function performs a 16 bit VME interrupt acknowledge cycle .

* \param   [IN]  handle: device handler
* \param   [IN]  Level: The VME IRQ level to acknowledge (see IRQLevels enum).
* \param   [OUT] BoardID: the Id of the Board that reased the interrupt.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_VMEIACKCycle16(int VMEhandle, IRQLevels Level, int *BoardID)
{
  return CAENComm_NotSupported;
}



/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_VMEIACKCycle32(int VMEhandle, IRQLevels Level, int *BoardID)
* \brief   The function performs a 32 bit VME interrupt acknowledge cycle.

* \param   [IN]  handle: device handler
* \param   [IN]  Level: The VME IRQ level to acknowledge (see IRQLevels enum).
* \param   [OUT] BoardID: the Id of the Board that reased the interrupt.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_VMEIACKCycle32(int VMEhandle, IRQLevels Level, int *BoardID)
{
  return CAENComm_NotSupported;
}



/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_IACKCycle(int handle, IRQLevels Level, int *BoardID)
* \brief   The function performs a 16 bit VME interrupt acknowledge cycle .

* \param   [IN]  handle: device handler
* \param   [IN]  Level: The VME IRQ level to acknowledge (see IRQLevels enum).
* \param   [OUT] BoardID: the Id of the Board that reased the interrupt.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_IACKCycle(int handle, IRQLevels Level, int *BoardID)
{
  return CAENComm_NotSupported;
}

/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_IRQWait(int handle, uint32_t Timeout)
* \brief   The function wait the IRQ until one of them raise or timeout expires.
		   This function can be used ONLY on board NOT controlled by CAEN VME Bridges 

* \param   [IN]  handle: device handler
* \param   [IN]  Timeout: Timeout in milliseconds.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_IRQWait(int handle, uint32_t Timeout)
{
  return CAENComm_NotSupported;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_VMEIRQWait(int LinkType, int LinkNum, int ConetNode, uint8_t IRQMask, uint32_t Timeout, int *VMEHandle)
* \brief   The function wait the IRQ until one of them raise or timeout expires.

* \param   [IN] LinkType: the link used to connect to the CAEN VME Bridge(CAENComm_USB|CAENComm_PCI_OpticalLink|CAENComm_PCIE_OpticalLink|CAENComm_PCIE)
* \param   [IN] LinkNum: The link number
* \param   [IN] ConetNode: The CAEN VME Bridge number in the link.
* \param   [IN] Timeout: Timeout in milliseconds.
* \param   [IN] IRQMask: A bit-mask indicating the IRQ lines
* \param   [OUT] VMEHandle: The CAEN Bridhe handle to use in VMEIRQCheck and VMEIACKCycle
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_VMEIRQWait(CAENComm_ConnectionType LinkType, 
					       int LinkNum, int ConetNode, uint8_t IRQMask, 
					       uint32_t Timeout,int *VMEHandle)
{
  return CAENComm_NotSupported;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_Info(int handle, CAENCOMM_INFO info, char *data)
* \brief   The function returns information about serial number or firmware release of the device.

* \param   [IN]  handle: device handler
* \param   [IN]  info: The interested info (see CAENCOMM_INFO enum).
* \param   [OUT]  data: an array (user defined to 30 byte) with the requested info
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_Info(int handle, CAENCOMM_INFO info, char *data)
{
  return CAENComm_NotSupported;
}



/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_SWRelease(char *SwRel)
* \brief   Returns the Software Release of the library
*
* \param   [OUT] SwRel: the Software Release of the library
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_SWRelease(char *SwRel)
{
  return CAENComm_NotSupported;
}


}


