// ddas timestamp extractor for NSCL event builder
// compile this function into a .so for use by
// NSCL event builder.

#include <DataFormat.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <fstream>

using namespace std;

extern "C" {
uint64_t timestamp(pPhysicsEventItem item)
{

  // NSCL DAQ 11.* and greater does not extract time stamp from this function
  // instead, timestamp is set into body header within CMyEventSegement.cpp
  // Return value of this function is ignored.

  return 0;

  /*
  // ddas data easily accessible in 32 bit words
  // get a pointer to the body of the ring item and 
  // reinterpret into 32-bit words
  uint32_t *body = reinterpret_cast<uint32_t*>(item->s_body);
  
  //body[0] - size of body, default information placed by ring item
  //body[1] - bits identfying module version information
  //body[2] - channel identifying information
  //body[3] - lower 32 bits of the 48-bit ddas timestamp
  //body[4] - upper 16 bits of the 48-bit ddas timestamp contained in the lower 16 bits of the word
  
  uint64_t tstamplo  = body[3];
  uint64_t tstamphi  = body[4] & 0xffff;
  uint64_t tstampFull = tstamplo | (tstamphi << 32);
    
  if (tstampFull>0xffffffffffffLL) {  
      cout << "tstamp bigger than 48bits" << std::endl;
      cout << "tstamp = " << tstampFull << std::endl;
      if (tstampFull==0xffffffffffffffffLL) {
          exit(1);
      }
  }

  double conversion;
  // sampling rate in MHz
  int ModMSPS = body[1] & 0xffff;
  //conversion = 1000000000. / (ModMSPS * 1000000.);
  if( (ModMSPS == 100) ){
    conversion = 10;
  }
  else if( (ModMSPS == 250) ){
    conversion = 8;
  }
  else if( (ModMSPS == 500) ){
    conversion = 10;
  }
  else{
    cout << " WARNING - Module time calibration not found " << endl;
  }
  //convert clock ticks to real time in nanoseconds
  tstampFull = tstampFull * conversion;

  return tstampFull;

  assert(0);

  */

} 

}
