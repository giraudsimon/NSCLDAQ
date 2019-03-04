
#include <COneShotException.h>


COneShotException::COneShotException(std::string reason)
  : CException("OneShot"),
  m_reason(reason)
{}

COneShotException::COneShotException(std::string context, std::string reason)
  : CException(context),
  m_reason(reason)
{}


const char* COneShotException::ReasonText() const
{
  return m_reason.c_str();
}
