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

  // ddas data easily accessible in 32 bit words
  // get a pointer to the body of the ring item and 
  // reinterpret into 32-bit words
  uint32_t *body =
    reinterpret_cast<uint32_t*>(
        bodyPointer(reinterpret_cast<pRingItem>(item)
        )
    );
  
  //body[0] - size of body, default information placed by ring item
  //body[1] - channel identifying information
  //body[2] - lower 32 bits of the 48-bit ddas timestamp
  //body[3] - upper 16 bits of the 48-bit ddas timestamp contained in the lower 16 bits of the word
  
  uint64_t tstamplo  = body[2];
  uint64_t tstamphi  = body[3] & 0xffff;
  uint64_t tstampFull = tstamplo | (tstamphi << 32);
    
  if (tstampFull>0xffffffffffffLL) {  
      cout << "tstamp bigger than 48bits" << std::endl;
      cout << "tstamp = " << tstampFull << std::endl;
      if (tstampFull==0xffffffffffffffffLL) {
          exit(1);
      }
  }

  return tstampFull;

  assert(0);

} 

}
