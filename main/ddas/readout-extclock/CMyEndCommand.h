#ifndef MYEND_H
#define MYEND_H

#include <CEndCommand.h>

class CTCLInterpreter;
class CTCLObject;
class CMyEventSegment;

class CMyEndCommand : public CEndCommand
{
 
private:

public:
	// Constructors, destructors and other cannonical operations: 
  
  CMyEndCommand(CTCLInterpreter& interp, CMyEventSegment *myevseg);              //!< Default constructor.
  ~CMyEndCommand (); //!< Destructor.
  
private:
  CMyEndCommand(const CMyEndCommand& rhs);
  CMyEndCommand& operator=(const CMyEndCommand &rhs);
  int operator==(const CMyEndCommand& rhs) const;
  int operator!=(const CMyEndCommand& rhs) const;
    
  CMyEventSegment *myeventsegment;
  int NumModules;
  
  // Class operations:
public:  
  virtual int ExecutePreFunction();
  virtual int ExecutePostFunction();
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);
 
};
#endif
