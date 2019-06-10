/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

#ifndef CV8BUFFER_H
#define CV8BUFFER_H

#include <V8/bheader.h>

namespace DAQ
{
  namespace V8 {

    class CRawBuffer;

    /*! \brief Abstract base class for all version 8 buffers   
     *
     * There is no good way to specify it in an abstract base class,
     *  but all derived classes will also be constructible from 
     *  CRawBuffer objects. This is by convention.
     */
    class CV8Buffer
    {
    public:
      virtual ~CV8Buffer() {}
      virtual bheader getHeader() const = 0;
      virtual BufferTypes type() const = 0;
      virtual void toRawBuffer(CRawBuffer& buffer) const = 0;
    };

  }
}

#endif // CV8BUFFER_H
