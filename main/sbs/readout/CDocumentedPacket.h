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


//////////////////////////CDocumentedPacket.h file//////////////////////////////////

#ifndef CDOCUMENTEDPACKET_H  
#define CDOCUMENTEDPACKET_H
                               
#include <string>   



                               
/*!
   Encapsulates a documented packet for two purposes:
   - Generation of packet headers for the readout.
   - Generation of documentation of packets used by the system.
   Member data comes in two (overalpping) chunks therefore:
   - m_nTag, m_sName, m_sDescription m_sVersion m_sInstantiationDate
     are used to document the tag  in documentation buffers.
   - m_nTag, m_pHeaderPtr, m_fPacketInProgress are used during
      packet formatting as an event is being taken.
   
 */

class CDocumentedPacket      
{ 
private:
  unsigned short m_nTag;	//!< Tag identifying event in the buffer.
  std::string m_sName;		//!< String name of tag.
  std::string m_sDescription;	//!< Long description of tag.
  std::string m_sVersion;		//!< Version of the packet.
  std::string m_sInstantiationDate;	//!< Date/time object instantiated.
  unsigned short* m_pHeaderPtr; //!< 'pointer' to header of current packet.
  bool m_fPacketInProgress;	//!< true if packet being built now.
 
public:
	// Constructors, destructors and other cannonical operations: 

  CDocumentedPacket (unsigned short nTag,
		     const std::string&  rName,
		     const std::string&  rDescription,
		     const std::string&  rVersion); //!<  Constructor
   virtual ~CDocumentedPacket();
      
private:  
  CDocumentedPacket(const CDocumentedPacket& rhs); //!< Copy constructor.
  CDocumentedPacket& operator= (const CDocumentedPacket& rhs); //!< Assignment
public:
   int         operator==(const CDocumentedPacket& rhs) const //!< Comparison for equality.
   {
      return (m_sName == rhs.m_sName);
   }
   int         operator==(const std::string& rhs) const 
   {
      return m_sName == rhs;
   }
   int         operator!=(const CDocumentedPacket& rhs) const
   {
      return !(operator==(rhs));
   }

   int         operator!=(const std::string& rhs) const
   {
      return !(operator==(rhs));
   }
   // Selectors for class attributes:
public:
  
  unsigned short getTag() const {
    return m_nTag;
  }
  
  std::string getName() const {
    return m_sName;
  }
  
  std::string getDescription() const {
    return m_sDescription;
  }
  
  std::string getVersion() const {
    return m_sVersion;
  }
  
  std::string getInstantiationDate() const {
    return m_sInstantiationDate;
  }
  
  unsigned short* getHeaderPtr() const; // Throws if pkt not open.
 

  bool getPacketInProgress() const {
    return m_fPacketInProgress;
  }
  
  // Class operations:

public:
  std::string Format ()  ;

  unsigned short* Begin (unsigned short* rPointer)  ;
  unsigned short* End (unsigned short* rBuffer)  ;
  
};

#endif
