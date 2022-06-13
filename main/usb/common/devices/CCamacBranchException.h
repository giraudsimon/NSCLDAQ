#ifndef CCAMACBRANCHEXCEPTION_H
#define CCAMACBRANCHEXCEPTION_H

#include <iostream>
#include <sstream>

class CCamacBranchException
{
public :
  CCamacBranchException() {}
  virtual ~CCamacBranchException() {}
  virtual const char* what() const { return "CamacBranchException"; }    
  
};

class CBadBCNAF : public CCamacBranchException
{
    public:
        const char* what() const { return "Invalid bcnaf code provided"; }    

};

class CInvalidA : public CCamacBranchException
{
private:
  int m_a;
public:
  CInvalidA(int a) : CCamacBranchException(), m_a(a) {}
  ~CInvalidA() {}
  virtual const char* what() const {
    std::stringstream msg;
    msg << m_a << " is an invalid A value";

    return msg.str().c_str();
  }

};

class CBadFirmwareFile : public CCamacBranchException
{
    private:
        std::string m_fname;
    public:
        CBadFirmwareFile(const std::string& fname) : m_fname(fname) {}
        const char* what() const { return (m_fname + " is invalid").c_str(); }    

};
#endif
