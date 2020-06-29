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
#ifndef EVBFRAMEWORK_H
#define EVBFRAMEWORK_H


#include <stdint.h>
#include <list>

struct gengetopt_args_info;

/**
 * This file contains the header that clients of the event builder client framework
 * should use.  It contains type definitions as well as a definition of the
 * CEVBClientFramework API class.
 *
 * Conventions:
 *   - class names start with C
 *   - class attributes are of the form m_xxxx
 *   - struct names start with a _
 *   - struct members are of the form s_xxxx
 *   - structs are defined in a typedef whic defines an xxx as well as a *pxxx
 *   - Types/classes start with uppercase letters.
 *   - Members (procedural and data) are in camel case.
 *   
 */


/**
 * The struct below defines the metadata associated with each event fragment.
 */

typedef struct __attribute__((__packed__)) _ClientEventFragment {
  uint64_t s_timestamp;
  uint32_t s_sourceId;
  uint32_t s_size;
  uint32_t s_barrierType;
  void*    s_payload;
  
} ClientEventFragment, *pClientEventFragment;

/**
 * The typedef below describes a CEVBEventList which is really and std::list<ClientEventFragment>
 *
 */
typedef std::list<ClientEventFragment> CEVBFragmentList, *pCEVBFragmentList;

/**
 *  CEVBClientFramework represents the API of the event builder
 *  framework.
 */
class CEVBClientFramework {
public:
  static const struct gengetopt_args_info* getProgramOptions();
  static void submitFragmentList(CEVBFragmentList& flist);
  static int  main(int argc, char** argv);
};

#endif
