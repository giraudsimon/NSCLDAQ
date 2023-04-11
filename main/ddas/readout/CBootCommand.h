/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CBootCommand.h
# @brief  Class for the ddasboot command
# @author <fox@nscl.msu.edu>
*/

#ifndef CBOOTCOMMAND_H
#define CBOOTCOMMAND_H

#include <TCLObjectProcessor.h>

class CMyEventSegment;
class CTCLInterpreter;
class CTCLObject;

/**
 * @class CBootCommand
 *
 *     This class implements the ddasboot command.  It is added to to the
 *     Tcl interpreter that runs ddasreadout so that the DDAS modules can be
 *     booted on demand rather than every time the Readout program starts.
 *
 *    Syntax:
 *    \verbatim
 *       ddasboot
 *    \endverbatim
 */
class CBootCommand : public CTCLObjectProcessor
{
private:
    CMyEventSegment* m_pSegment;
public:
    CBootCommand(CTCLInterpreter& interp, const char* pCmd, CMyEventSegment* pSeg);
    virtual ~CBootCommand();
    
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif