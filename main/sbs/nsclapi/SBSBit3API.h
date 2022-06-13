/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
/*!
   Device specific extensions to CVMEInterface for SBS's driver for
   the SBS/Bit3 PCI - VME bus bridge.  Use of these functions will
   make your software device specific!!!!!!  

   Note as well that the 
   modifications you make will be global to all users of the same 
   device (address space), and will not be reset when you close/extit.
   So if you change a VMEA32/D32 device into a shortio device you've changed
   it for everyone!!!!
*/

#ifndef SBSBIT3API_H
#define SBSBIT3API_H

#ifndef BT1003
#define BT1003
#endif

extern "C" {



#include <btapi.h>

}
class CSBSBit3VmeInterface {
public:

  // Set parameters...

  static void SetDMABlockTransfer(void* pHandle,    bool enable);
  static void SetDMAPauseMode(void* pHandle,        bool enable);
  static void SetMaxTransferWidth(void* pHandle,    bt_width_t width);
  static void SetDMAAddressModifier(void* pHandle,  int Modifier);
  static void SetMMAPAddressModifier(void* pHandle, int Modifier);
  static void SetSwapMode(void* pHandle,            bt_swap_t SwapMode);
  static void SetDMAThreshold(void* pHandle,        int nTransfers);
  static void SetDMAPollCeiling(void* pHandle,      int nTransfers);
  static void SetTraceMask(void* pHandle,           int nMask);

  // Get current value of parameters:

  // Readonly parameters first:

  static int          GetLocalCardPartNumber(void* pHandle);
  static int          GetRemoteCardPartNumber(void* pHandle);
  static unsigned int GetLocalMemorySize(void* pHandle);

  static bool         IsDMABlockTransfer(void* pHandle);
  static bool         IsDMAPauseMode(void* pHandle);
  static bt_width_t   GetMaxTransferWidth(void* pHandle);
  static int          GetDMAAddressModifier(void* pHandle);
  static int          GetMMAPAddressModifier(void* pHandle);
  static int          GetSwapMode(void* pHandle);
  static int          GetDMAThreshold(void* pHandle);
  static int          GetDMAPollCeiling(void* pHandle);
  static int          GetTraceMask(void* pHandle);

  // Direct interfaces to the bt interface:

  static void         ResetVme(void* pHandle); //!< Reset VME bus.

  // Get/clear error status:

  static  bt_error_t CheckErrors(void* pHandle);
  static  bt_error_t ClearErrors(void* pHandle);

};


#endif
