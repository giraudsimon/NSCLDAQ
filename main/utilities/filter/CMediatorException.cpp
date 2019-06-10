
#include <CMediatorException.h>


CMediatorException::CMediatorException(std::string reason)
  : CException("Mediator"),
  m_reason(reason)
{}

CMediatorException::CMediatorException(std::string context, std::string reason)
  : CException(context),
  m_reason(reason)
{}


const char* CMediatorException::ReasonText() const
{
  return m_reason.c_str();
}
