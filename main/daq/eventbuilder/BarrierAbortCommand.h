/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  BarrierAbortCommand.h
 *  @brief: EVB::abortbarrier - command to abort waiting for barrier.
 */
#ifndef BARRIERABORTCOMMAND_H
#define BARRIERABORTCOMMAND_H
#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CBarrierAbortCommand
 *    Provides an EVB::abortbarrier. This just gets the fragment instance
 *    and invokes the abortBarrierProcessing method.
 */
class  CBarrierAbortCommand : public CTCLObjectProcessor
{
public:
    CBarrierAbortCommand(CTCLInterpreter& interp, const char* cmd);
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif