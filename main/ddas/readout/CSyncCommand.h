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
# @file   CSyncCommand.h
# @brief  Header for class that implements the ddas_sync command.
# @author <fox@nscl.msu.edu>
*/

#ifndef CSYNCCOMMAND_H
#define CSYNCCOMMAND_H

#include <TCLObjectProcessor.h>


// Forward class definitions:   

class CTCLInterpreter;
class CTCLObject;
class CMyEventSegment;

/**
 *  @class CSyncCommand
 *     Provides the ddas_sync command for the ddas readout  program.
 *     *  Construction maintains a pointer to the event segment.
 *     *  The class registers the "ddas_sync" command on the main interp.
 *     *  When invoked, simply calls the synchronize method of the event
 *        segment.
 *  @note  NOTE A more refined approach would be to refuse to perform the operation
 *         when the run is in progress.  At this time, however we're going to
 *         (heaven help us) rely on the user to know that they really need to
 *         do a clock synchronization.
 */
class CSyncCommand : public CTCLObjectProcessor
{
private:
    CMyEventSegment*  m_pSegment;
    
public:
    CSyncCommand(CTCLInterpreter& interp, CMyEventSegment* pSeg);
    virtual ~CSyncCommand();
    
    int operator() (CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif
