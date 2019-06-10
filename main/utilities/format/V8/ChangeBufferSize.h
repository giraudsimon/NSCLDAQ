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


#ifndef DAQ_V8_TEST_CHANGEBUFFERSIZE_H
#define DAQ_V8_TEST_CHANGEBUFFERSIZE_H

namespace DAQ {
  namespace V8 {
    namespace Test {

      /*!
       * \brief A simple tool to change the buffer size within a scope
       *        and return it to the original value automatically
       */
      struct ChangeBufferSize {
        std::size_t oldSize;
        ChangeBufferSize(std::size_t bsize) : oldSize(gBufferSize) {
          gBufferSize = bsize;
        }
        ~ChangeBufferSize() {
          gBufferSize = oldSize;
        }
      };

    } // namespace Test
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_TEST_BUFFERSIZECHANGER_H
