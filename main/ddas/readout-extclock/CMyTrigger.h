#include <CEventTrigger.h>
#include <time.h>
#include "pixie16sys_export.h"

class CMyTrigger : public CEventTrigger
{
 
private:

  bool m_retrigger;           // retrigger flag for pixie16 buffer readout
  unsigned int nFIFOWords;   // words in pixie16 output data buffer
  int NumberOfModules;        // number of pixie16 modules
  unsigned short ModNum;      // pixie16 module number

  unsigned       m_fifoThreshold; 
public:
	// Constructors, destructors and other cannonical operations: 
  
  CMyTrigger ();                      //!< Default constructor.
  ~CMyTrigger (); //!< Destructor.
  
  
  // Selectors for class attributes:
public:
  time_t start,end;
  // Mutators:
protected:  
  
  // Class operations:
public:  
  virtual void setup();
  virtual void teardown();
  virtual   bool operator() ();
  virtual   void Initialize( int nummod ); 
  void Reset();
  //int GetNumberOfModules() {return NumberOfModules;}

};
