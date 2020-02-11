/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

#include <CDataSinkException.h>


CDataSinkException::CDataSinkException(std::string reason)
  : CException("DataSink operation"),
  m_reason(reason)
{}

CDataSinkException::CDataSinkException(std::string context, std::string reason)
  : CException(context),
  m_reason(reason)
{}


const char* CDataSinkException::ReasonText() const
{
  return m_reason.c_str();
}
