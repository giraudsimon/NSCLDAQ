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


//////////////////////////CDocumentedPacketManager.h file//////////////////////////////////

#ifndef CDOCUMENTEDPACKETMANAGER_H  
#define CDOCUMENTEDPACKETMANAGER_H
                               

#include <list>
#include <vector>
#include <string>

/*!
   Manages a set of documented packet types.  This is
   essentially a facade for an STL list of packet definitions.
   The additional item I can get is a preformatted
   array of strings which document all of the packets in
   the list.
 */

class CDocumentedPacket;
typedef std::list<CDocumentedPacket*> DocumentationPacketList;
typedef DocumentationPacketList::iterator DocumentationPacketIterator;

class CDocumentedPacketManager      
{ 
private:
   static  CDocumentedPacketManager* m_ptheInstance; //!< Instance pointer to singleton packet manager.
   DocumentationPacketList           m_PacketDefinitions; //!< List of packet types
public:
	// Constructors, destructors and other cannonical operations: 

   CDocumentedPacketManager ();                      //!< Default constructor.
   ~ CDocumentedPacketManager ( ) { } //!< Destructor.
     
      // As a singleton class,the members below make no sens, and are therefore
      // not implemented.
private:
   CDocumentedPacketManager(const CDocumentedPacketManager& rhs); //!< Copy constructor.
   CDocumentedPacketManager& operator= (const CDocumentedPacketManager& rhs); //!< Assignment
   int         operator==(const CDocumentedPacketManager& rhs) const; //!< Comparison for equality.
   int         operator!=(const CDocumentedPacketManager& rhs) const;
 public:

	// Selectors for class attributes:
public:

   DocumentationPacketList getDefinitions() {
      return m_PacketDefinitions;
   }

   static CDocumentedPacketManager* getInstance();

	// Mutators:
protected:  

  void setDefinitions(DocumentationPacketList Packets) {
      m_PacketDefinitions = Packets;
  }
	// Class operations:
public:
   void AddPacket (CDocumentedPacket& rPacket)  ;
   void DeletePacket (const std::string& rName)  ;
   DocumentationPacketIterator begin ()  ;
   DocumentationPacketIterator end ()  ;
   unsigned int size ()  ;
   std::vector<std::string> Format ()  ;
 
};

#endif
