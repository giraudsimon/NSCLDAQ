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
#ifndef CSIS3600_H
#define CSIS3600_H
#include <CModule32.h>
#include <string>

#include <VmeModule.h>

/*!
   This class provides software support for the SIS 3600 latch
   module.  Currently, support is limited to latch mode (not 
   coincidence mode).
*/
class CSIS3600 : public CModule32
{
private:
  int m_nFastClearWindow;	//!< Shadows fast clear window value.

  // Constructors and other canonicals:

public:
  CSIS3600(uint32_t base, int crate =0) ;
  ~CSIS3600();

private:
  CSIS3600(const CSIS3600& rhs); //!< Copy constructor.
  CSIS3600& operator=(const CSIS3600& rhs); //!< Assignment.
  int      operator==(const CSIS3600& rhs) const;
  int      operator!=(const CSIS3600& rhs) const;
public:
  // Operations:
  void Reset();			//!< Reset the module.

  // Operations on the user light and output bit (mode 2).

  void LightLed() const;	//!< Light the led.
  void ClearLed() const;	//!< Clear the led.
  bool isLedLit() const;	//!< true if led is lit.

  // General operational mode.  Coincidence and latch
  // mode are mutually incompatible!!
  
  void SetLatchMode() const;	//!< Use latch (strobed) mode.
  void SetCoincidenceMode() const; //!< Use coincidence mode instead.
  bool isLatchMode() const;	//!< Check current operating mode.

  // Clear functions
  //
  void EnableExternalClear() const;  //!< External clear input works.
  void DisableExternalClear() const; //!< External clear disable.
  bool ExternalClearEnabled() const; //!< State of clear enable.

  void EnableExternalNext() const;
  void DisableExternalNext() const;
  bool ExternalNextEnabled() const;

  void SetFastClearWindow(int ns); //!< Set fast clear window.
  int  GetFastClearWindow() const; //!< Inquire fast clear window.

  // Next clock logic control:

  void Enable()  const;		//!< Enable receipt of data.
  void Disable() const;         //!< Disable receipt of data.
  void Clock()   const;		//!< Software clock data.

  // Latching data in software:

  void StartLatch() const;	//!< Begin a software latch.
  void EndLatch()   const;	//!< End the software latch period.

  // Data transfer and availability checking:

  bool DataReady()  const;	//!< True if FIFO not empty
  void ClearData()  const;	//!< Clear the FIFO.
  uint32_t Read() const ;
  size_t  Read(void* pBuffer, 
	       int nLongs) const;	//!< Read block of events.


public:

  // Utilities:

protected:


};


#endif
