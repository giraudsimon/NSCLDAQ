#ifndef CCAENCHAIN_H
#define CCAENCHAIN_H

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



// Headers required:

#include <vector>
#include <string>
#include <RangeError.h>
#include <stdint.h>


// Forward class definitions:

class CAENcard;



/*!
   This class allows the user to compose a set of modules into a 
   chain for CBLT readout.  The CBLT readout support allows a set of 
   crate contiguous modules to be readout in a unit.  As each module
   finishes presenting the data associated with an event, it uses the
   IRQ/IACK daisy chain to pass control on to the next module.  This makes
   large systems amenable to block mode readout (and that's what this class
   sets up).
   
   Chains consist of a start card (left most module in the chain), an
   end card (right most module in the chain) and a set of intermediate cards
   which must not skip any slots, and must live to the right of the
   start card and to the left of the end card.

   Sample good chain for a crate:
   \verbatim

   +-+-+-+-+-+-+-+-+-+
   |S I I I E x x x x|
   +-----------------+
 
   S - Start card
   I - Intermediate card
   E - End card
   x - Don't care
   - - Empty slot.

   \endverbatim

   Sample invalid chains:

   \verbatim

   +-+-+-+-+-+-+-+-+-+
   |S I - I E x x x x|
   +-----------------+

   Empty slot in the middle of a chain.

   +-+-+-+-+-+-+-+-+
   |I S I I E x x x| 
   +---------------+
 
   Intermediate card not in between Start and End cards.

   \endverbatim

   In general, if you have a system of geographical addressing, you won't
   need to worry about this, just plug the cards in and designate to the
   constructor the range of slots you'll be using.

   If you don't have slot addressable cards, you'll have to be sure that the
   std::vector of base addresses you supply really corresponds to a valid chain
   or errors will occur.
*/
class CCAENChain {
  // Private data.
private:
  std::vector<CAENcard*> m_vCards;	    //!< Card driver objects for the chain.
  uint32_t     m_nCBLTAddress; //!< CBLT address assigned to the chain.
  int               m_nCrate;	    //!< VME crate containing the chain. 
  void*             m_pHandle;   //!< Handle to he VME crate for Read.
  uint16_t    m_nMaxBytes;    //!< Calculated maximum transfer size.
  void*             m_pBase;	//!< Ptr to base address.

  // Constructors and other canonical operations.
public:
  CCAENChain(int nFirstSlot, int nLastSlot,
	     std::vector<uint32_t>& vBases,
	     int nCrate = 0, bool geo=true);
  ~CCAENChain();
  
  // Canonicals that are not allowed:
private:
  CCAENChain(const CCAENChain& rhs);             //!< forbidden!!
  CCAENChain& operator= (const CCAENChain& rhs); //!< forbidden!!
  int         operator==(const CCAENChain& rhs); //!< Forbidden!!
  int         operator!=(const CCAENChain& rhs); //!< Forbidden!!!
public:


  // Selectors:
public:
  std::vector<CAENcard*> getCards() {      //!< Get std::vector with ptrs to all cards. 
    return m_vCards;
  }
  uint32_t getCBLTAddress() const { //!< Get CBLT transfer address.
    return m_nCBLTAddress;
  }
  int getCrate() const {		//!< Get crate # the chain lives in.
    return m_nCrate;
  }
  void* getVmeHandle() {	//!< Get handle to VME device open on cblt.
    return m_pHandle;
  }
  uint16_t getMaxBytes() const { //!< Get max# of bytes this chain produces.
    return m_nMaxBytes;
  }


  // Mutators: - intended mostly for derived classes that might need to 
  //             set these things during construction.

protected:
  void setCards(const std::vector<CAENcard*> vCards) { //!< Set std::vector of card ptrs
    m_vCards = vCards;
  }
  void setCBLTAddress(uint32_t nCBLTAddress) { //!< set new xfer address.
    m_nCBLTAddress = nCBLTAddress;
  }
  void setCrate(int nCrate) {	//!< Set the crate # the chain is in.
    m_nCrate = nCrate;
  }
  void setHandle(void* pHandle) { //!< Set new vme handle.
    m_pHandle = pHandle;
  }
  void setMaxBytes(uint16_t nBytes) { //!< Set new max bytes value.
    m_nMaxBytes = nBytes;
  }

  // Class Functions:
public:

  CAENcard* operator[](int index) ;
  int ReadEvent(void* pBuffer);                //!< Read an event from cards.
  void ClearData();		               //!< Clear constituent cards.
  void lastModuleEmptyEnable();     ;
  void lastModuleEmptyDisable();

  // Utilities:

private:
  void FreeModules();		//!< Kill all  modules driver objects
  CAENcard* lastCard();        // Return the last card in the chain.
};


#endif



