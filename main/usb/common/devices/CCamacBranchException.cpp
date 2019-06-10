
#include <CCamacBranchException.h>
#include <sstream>

const char* CInvalidA::what() const
{
    std::stringstream msg;
    msg << m_a << " is an invalid A value";

    return msg.str().c_str();
}
