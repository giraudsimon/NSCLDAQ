/*********************************************************
 Declaration of Scaler class for DDAS
 Access statistics directly from the Pixie16 modules
 H.L. Crawford 6/13/2010
*********************************************************/

#ifndef MYSCALER_H
#define MYSCALER_H

#include <config.h>
#include <CScaler.h>
#include <vector>
//#include <stdint.h>
//#include "pixie16app_globals.h"

//#ifdef HAVE_STD_NAMESPACE
using namespace std;
//#endif

class CMyScaler : public CScaler
{
public:
   typedef struct _Counters {
     size_t s_nTriggers;
     size_t s_nAcceptedTriggers;
   } Counters;
   typedef struct _Statistics {
      Counters s_cumulative;
      Counters s_perRun;
   } Statistics;
 private:
  unsigned short numModules;
  unsigned short crateID;
  unsigned short moduleNumber;
  double PreviousCounts[16];
  double PreviousCountsLive[16];

  vector<uint32_t> scalers;
  Statistics m_Statistics;
 public:
  CMyScaler(unsigned short moduleNr, unsigned short crateid); // Constructor
  ~CMyScaler();
  virtual void initialize();
  virtual vector<uint32_t> read();
  virtual void clear();
  virtual void disable();
  virtual unsigned int size() {return 32;};
  
  //
  const Statistics& getStatistics() const { return m_Statistics;}
private:
  void clearCounters(Counters& c);
};

#endif
