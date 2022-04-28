/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CMarkerObject.h
# @brief  Controls for marker object
# @author <fox@nscl.msu.edu>
*/

#ifndef _CMARKEROBJECT_H
#define _CMARKEROBJECT_H


#ifndef __CCONTROLHARDWARE_H
#include "CControlHardware.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#include "CControlModule.h"


class CVMUSB;



/**
 * @class CMarkerObject
 *    This is a control object whose sole purpose in life
 *    is to add a marker to the monitor list.  It can be used, if desired
 *    to ensure that the data buffers get periodically flushed,
 *    since monitor list execution will flush the data buffer for any list
 *    that has a buffer going (unless mixed mode is requested).
 *  Configuration option is:
 *    - -value - the value of the marker.
 *  There are no settable or gettable parameters.
 *  
 */
class CMarkerObject : public CControlHardware
{
private:
    CControlModule* m_pConfiguration;
public:
    CMarkerObject();
    CMarkerObject(const CMarkerObject& rhs);
    virtual ~CMarkerObject();
    CMarkerObject& operator=(const CMarkerObject& rhs);
    int operator==(const CMarkerObject& rhs) const;
    int operator!=(const CMarkerObject& rhs) const;
    
    // The CControlHardware interface:
    
public:
    virtual void onAttach(CControlModule& configuration);  //!< Create config.

    virtual std::string Update(CVMUSB& vme);               //!< Update module.
    virtual std::string Set(CVMUSB& vme, 
                            std::string parameter, 
                            std::string value);            //!< Set parameter value
    virtual std::string Get(CVMUSB& vme, 
                            std::string parameter);        //!< Get parameter value.
    
    virtual void addMonitorList(CVMUSBReadoutList& vmeList);     //!< add items to the monitor list.
    virtual void* processMonitorList(void* pData, size_t remaining);  

    
    virtual CControlHardware* clone() const;	     //!< Virtual copy constr.
    void    copyIn(const CMarkerObject& rhs);
};

#endif
