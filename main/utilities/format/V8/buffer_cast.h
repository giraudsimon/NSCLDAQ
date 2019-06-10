
#ifndef BUFFER_CAST_H
#define BUFFER_CAST_H

namespace DAQ 
{
  namespace V8
  {
    template<class T1>
      T buffer_cast(const CRawBuffer&) {

        return T();
      } 
  }
}
#endif
