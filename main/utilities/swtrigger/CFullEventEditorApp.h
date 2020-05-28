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

/** @file:  CFullEventEditorApp.h
 *  @brief: Defines ABC for full event editor application class.
 */

#ifndef CFULLEVENTEDITORAPP_H
#define CFULLEVENTEDITORAPP_H
#include "swtriggerflags.h"
#include <CFullEventEditor.h>
class CTransport;
class CFullEventEditor;

/**
 * @class CFullEventEditorApp
 *     An Abstract base class for transport dependent applications
 *     Provides some common facilities used by all of these classes.
 */
class CFullEventEditorApp
{
public:
    typedef CFullEventEditor::Editor* (*createEditor)();
protected:
    gengetopt_args_info m_params;            // Derived classes can fish this out.
    createEditor        m_pEditorFactory;    // Editor creator.
    
public:
    CFullEventEditorApp(gengetopt_args_info& p);
    virtual ~CFullEventEditorApp();
    CTransport* createRingSink();           // Creates a ring transport sink.
    
    
    virtual int operator()() = 0;
    CFullEventEditor::Editor* createUserEditor();
};


#endif