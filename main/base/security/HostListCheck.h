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

// Class: CHostListCheck                     //ANSI C++
//
// Encapsulates host list authentication.
// {Add,Delete}AclEntry allow named hosts to be added.
//  gethostbyname is used to resolve these to IP adresses which,
//  represented in a.b.c.d form are entered in the map.
//  {Add,Delete}IpAddress are used to add an IP address.
//  The address is represented textually in a.b.c.d form and
//   entered in the map.
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved HostListCheck.h
//

#ifndef HOSTLISTCHECK_H  //Required for current class
#define HOSTLISTCHECK_H

#include "AccessListCheck.h"
#include "StringInteractor.h"
#include <netinet/in.h>
#include <string>
#include <iostream>

                               
class CHostListCheck  : public CAccessListCheck        
{                       
			
protected:

public:

   // Constructors and other cannonical operations:

  CHostListCheck ()  
  { 
  } 
   ~ CHostListCheck ( )  // Destructor 
  { }  
  
   //Copy constructor 

  CHostListCheck (const CHostListCheck& aCHostListCheck )   : 
    CAccessListCheck (aCHostListCheck) 
  { 
  }                                     

   // Operator= Assignment Operator 

  CHostListCheck& operator= (const CHostListCheck& aCHostListCheck) {
    if(this != &aCHostListCheck) {
      CAccessListCheck::operator=(aCHostListCheck);
    }
    return *this;
  }
 
   //Operator== Equality Operator 
private:
  int operator== (const CHostListCheck& aCHostListCheck) const;
public:
       
public:
  virtual bool Authenticate(CInteractor& rInteractor) {
    return Authenticate(GetLine(rInteractor));
   } 
  bool Authenticate(const std::string& rHostname);

  virtual   void AddAclEntry (const std::string& rHostname) {
    std::cerr << ">Warning, CHostListCheck::AddAclEntry called "
         << " Should call AddIpAddress\n";
    CAccessListCheck::AddAclEntry(rHostname);
  }
  virtual   void DeleteAclEntry (const std::string& rHostname) {
    std::cerr << ">Warning, CHostListCheck::DeleteAclEntry called "
         << " Should call DeleteIpAddress\n";
    CAccessListCheck::DeleteAclEntry(rHostname);
  }
  bool Authenticate(in_addr Address) {
    CStringInteractor Host(EncodeAddress(Address));
    return CAccessListCheck::Authenticate(Host);
  }
  void AddIpAddress (in_addr Address)    ;
  void DeleteIpAddress (in_addr address)    ;
  
protected:
	std::string EncodeAddress(in_addr Address);

};

#endif
