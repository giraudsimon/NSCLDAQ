/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

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
  Encapsulates the order in which modules are read out.
  CReadOrder maintains an ordered list of modules to read.
  There is normally only one CReadorder object, but that's not
  necessary.  At each key point in the skeleton the read order
  object is invoked to iterate through the set of active modules
  via its entry points:
  - Initialize
  - Prepare
  - Read
  - Clear
  These simply iterate and delegate to the modules in the list.
  For now iteration is done via the STL for_each generic 
  fuctions and the helper classes:
  

*/
#ifndef CREADORDER_H  //Required for current class
#define CREADORDER_H

#include "CReadableObject.h"
#include <list>
#include <string>
#include <tcl.h>
#include <stdint.h>

using namespace std;
// Forward Definitions:

class CTCLInterpreter;
class CTCLResult;
class CDigitizerDictionary;
class CBoolConfigParam;
class CIntConfigParam;


class CReadOrder    : public CReadableObject
{
public:                           // Data types:
  typedef std::list<CReadableObject*> ModuleList;     //!< Ordered list of modules.
  typedef ModuleList::iterator    ModuleIterator; //!< Iterator for list of modules
private:
  
  // Private Member data:
  
  ModuleList            m_ReadoutList;           //!< What we read 
  CDigitizerDictionary* m_pModules;              //!< Known modules we could read. 
  int                   m_nPacketId;             //!< Our id if packetizing. 
  bool                  m_fPacketize;            //!< true if packetizing. 
  CIntConfigParam*      m_pPacketIdParam;        //!< Ptr to packet parameter 
  CBoolConfigParam*     m_pPacketizeParam;       //!< Ptr to packetizer parameter. 
   
private:
  // Nested helper classes.
/*
  Helper classes used during for_each iterations through
 the module list.s
*/
  class ModuleRead 
  {
  private:
    uint16_t*  m_pBuffer;
  public:
    ModuleRead(uint16_t* pBuf) :
      m_pBuffer(pBuf) {}
    void operator()(CReadableObject*  p) {
      p->Read(m_pBuffer);
    }
  };
  class ModuleInitialize
  {
    public:
      void operator()(CReadableObject* p) {
        p->Initialize();
      }
  };
  class ModulePrepare
  {
    public:
      void operator()(CReadableObject* p) {
        p->Prepare();
      }
  };
  class ModuleClear
  {
    public:
      void operator()(CReadableObject* p) {
        p->Clear();
      }
  };
   class CompareName {
      private:
         std::string m_Name;
      public:
         CompareName(const std::string& rName) :
            m_Name(rName)
         {}
         int operator()(CReadableObject* p) {
            return (m_Name == p->getName());
         }
   };
   class Lister {
   private:
      CTCLResult& m_rResult;
      std::string           m_sPattern;
   public:
      Lister(CTCLResult& rResult, const std::string & rPattern) :
	 m_rResult(rResult),
	 m_sPattern(rPattern)
      {}
      void operator()(CReadableObject* pModule)
      {
	 if(Tcl_StringMatch(pModule->getName().c_str(),
				   m_sPattern.c_str())) {
	    std::string element   = pModule->getName();
	    element         +=  " ";
	    element         += pModule->getType();
	    m_rResult.AppendElement(element);
	 }
      }
   };
public:
   // Constructors and other cannonical functions.
   
  CReadOrder (CTCLInterpreter* pInterp,
	      CDigitizerDictionary* pDictionary,
	      const std::string& rCommand=std::string("readout"));
  virtual ~CReadOrder ( ); 
private:
  CReadOrder (const CReadOrder& aCReadOrder );
  CReadOrder& operator= (const CReadOrder& aCReadOrder);
  int operator== (const CReadOrder& aCReadOrder) const;
  int operator!= (const CReadOrder& rhs) const;
public:

// Selectors:

public:
   ModuleList getReadoutList() const
   {
     return m_ReadoutList;
   }
  
  // Attribute mutators:
  
protected:
  void setReadoutList(const ModuleList& rReadoutlist) 
  {
    m_ReadoutList = rReadoutlist;
  }
  
  // Class operations:
  
public:

  // Command processors:

  int operator()(CTCLInterpreter& rInterp,
		 CTCLResult&        rResult,
		 int nArgc, char** pArgv);    //!< Command processing operator.
  int AddCommand(CTCLInterpreter& rInterp,
		 CTCLResult&       rResult,
		 int nArgc, char** pArgv);    //!< Add a module to our readout list
  int RemoveCommand(CTCLInterpreter& rInterp,
		    CTCLResult&       rResult,
		    int nArgc, char** pArgv); //!< Remove a module from our readout list
  int ListCommand(CTCLInterpreter& rInterp,
		  CTCLResult&       rResult,
		  int nArgc, char** pArgv);   //!< List modules in our list.
  


  // Overridables and overrides:

  virtual void   Initialize ()   ;            //!< 1-time Initialize our modules.
  virtual void   Prepare ()   ;               //!< Prepare modules for readout.  
  virtual int    Read(void* pBuffer);         //!< Read to ordinary buffer. 
  virtual void   Clear ()   ;                 //!< Clear after read & @ Run start. 
  virtual std::string getType() const;	              //!< Return module type information. 

  // Iteration through the module list.

  int            readersize ()   ;                  //!< # modules in the list. 
  ModuleIterator readerbegin ()   ;                 //!< Start iterator.
  ModuleIterator readerend ()   ;                   //!< End iterator.
  ModuleIterator readerfind(const std::string& rName);   //!< Find module by name.
  std::string         Usage();                     //!< Command usage.

  // External removal.

  void           Add    (CReadableObject* pModule)   ; //!< Add a module.
  void           Remove (ModuleIterator p)   ; //!< Remove module.
  void           Remove (CReadableObject* pModule); //!< Remove module
  
  virtual void   OnDelete();	              //!< Just prior to deletion.

};

#endif

