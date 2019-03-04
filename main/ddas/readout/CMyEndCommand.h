#ifndef MYEND_H
#define MYEND_H

#include <CEndCommand.h>
#include <tcl.h>

class CTCLInterpreter;
class CTCLObject;
class CMyEventSegment;
class CExperiment;


class CMyEndCommand : public CEndCommand
{
  public:
    struct EndEvent {
      Tcl_Event s_rawEvent;
      CMyEndCommand* s_thisPtr;
    };
 
private:

public:
	// Constructors, destructors and other cannonical operations: 
  
    CMyEndCommand(CTCLInterpreter& interp, CMyEventSegment *myevseg, CExperiment* exp);              //!< Default constructor.
  ~CMyEndCommand (); //!< Destructor.
  
private:
  CMyEndCommand(const CMyEndCommand& rhs);
  CMyEndCommand& operator=(const CMyEndCommand &rhs);
  int operator==(const CMyEndCommand& rhs) const;
  int operator!=(const CMyEndCommand& rhs) const;
    
  CMyEventSegment *myeventsegment;
  CExperiment*     m_pExp;
  int NumModules;
  
  // Class operations:
public:  
  int transitionToInactive();
  int readOutRemainingData();
  int endRun();
  void rescheduleEndTransition();
  void rescheduleEndRead();
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

protected:
  static int handleEndRun(Tcl_Event* pEvt, int flags);
  static int handleReadOutRemainingData(Tcl_Event* pEvt, int flags);
 
};
#endif
