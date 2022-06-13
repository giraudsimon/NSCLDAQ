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

/** @file:  CTransformerApp.h
 *  @brief: Base class for the application class for the Transformer program.
 */
#ifndef CTRANSFORMERAPP_H
#define CTRANSFORMERAPP_H

#include "swtriggerflags.h"
#include "CBuiltRingItemExtender.h"

typedef CBuiltRingItemExtender::CRingItemExtender Extender, *pExtender;
typedef Extender* (*ExtenderFactory)();
/**
 * @class CTransformerApp
 *    This is the base class of the application class for
 *    the transformer application.  The application implementions are supposed to
 *    - Set up processor objects and communications between them for a specific
 *      transport family.
 *    - Execute threads or the appropriate process (again depending on the
 *      communications framework).
 *
 * This application typically constis of a
 *     -   Data source element to get data from the source and fan it out to workers
 *     -   Workers that call user code to transform the data.
 *     -   A sort task into which the workers fan their data for timestamp re-ordering.
 *     -   An output task that puts sorted data to the sink.
 *
 * \verbatim
 *         source  - Worker farm - sorter - output (sink).
 * \endverbatim
 */
class CTransformerApp
{
protected:
    gengetopt_args_info m_params;
public:
    CTransformerApp(gengetopt_args_info p) : m_params(p) {}
    virtual ~CTransformerApp() {}
    
    virtual int operator()() = 0;                // Concrete class implement this.
public:
    ExtenderFactory getExtenderFactory();
};
#endif