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

/** @file:  CBuiltRingItemEditorApp.h
 *  @brief: Abstract base class for the built ring item editor application class
 */

#ifndef CBUILTRINGITEMEDITORAPP_H
#define CBUILTRINGITEMEDITORAPP_H

#include "swtriggerflags.h"
#include "CBuiltRingItemEditor.h"

typedef CBuiltRingItemEditor::BodyEditor Editor, *pEditor;
typedef Editor* (*EditorFactory)();

/**
 * @class  CBuiltRingItemEditorApp
 *    This is the base class for application level code for the
 *    ring item editor application.  Having a base and derived classes
 *    allows us to have specific application classes for various communication
 *    transports and patterns.  For example a ZMQThreaded might setup the
 *    classes to support threaded parallelism and ZMQ communication while an
 *    MPI class might do the same for MPI process parallelism and MPI
 *    communication.
 *
 *    This is the mechanism we have to implement the --parallel-strategy
 *    option in the program options.
 */
class CBuiltRingItemEditorApp
{
protected:
    gengetopt_args_info m_args;          // Parsed parameters.
public:
    CBuiltRingItemEditorApp(gengetopt_args_info args) :
        m_args(args)
    { }
    ~CBuiltRingItemEditorApp() {}
    virtual void operator()() = 0;
public:
    EditorFactory getEditorFactory();
};
#endif