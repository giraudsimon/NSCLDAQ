
#ifndef CXLMTIMESTAMP_H
#define CXLMTIMESTAMP_H

#include <CXLM.h>

class CXLMTimestamp : public XLM::CXLM
{

public:
  CXLMTimestamp();
  CXLMTimestamp(const CXLMTimestamp& rhs);
  virtual ~CXLMTimestamp();

private:
  CXLMTimestamp& operator=(const CXLMTimestamp& rhs); // assignment not allowed.
  int operator==(const CXLMTimestamp& rhs) const;	  // Comparison for == and != not suported.
  int operator!=(const CXLMTimestamp& rhs) const;


public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual CReadoutHardware* clone() const; 

};


#endif
