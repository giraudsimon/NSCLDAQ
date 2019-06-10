#include <CBusy.h>

class CMyBusy : public CBusy
{
public:
	// Constructors, destructors and other cannonical operations: 
  
  CMyBusy();                      //!< Default constructor
  ~CMyBusy() { } //!< Destructor.
  
  
  // Selectors for class attributes:
public:
  
  // Mutators:
protected:  
  
  // Class operations:
public:  
  virtual   void GoBusy () ;
  virtual   void GoClear ();
  //virtual   void ModuleClear();
};
