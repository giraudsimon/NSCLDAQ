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

/** @file:  CFullEventEditorApp.cpp
 *  @brief: Implement the full editor base class non virt. methods.
 */

#include "CFullEventEditorApp.h"
#include "CTransport.h"
#include "CRingItemTransportFactory.h"
#include "CRingItemTransport.h"
#include "CFullEventEditor.h"

#include <dlfcn.h>
#include <stdexcept>


/**
 * constructor
 *    Save the ars and init the editor creator to null.
 * @param p - parameters parsed by gengetopt.
 */
CFullEventEditorApp::CFullEventEditorApp(gengetopt_args_info& p) :
    m_params(p),
    m_pEditorFactory(nullptr)
{}
/**
 * destructor is an no-op for now
 */

CFullEventEditorApp::~CFullEventEditorApp() {}

/** createRingSink
 *    Creates a transport to the data sink
 *
 *  @return CTransport -the generated transport.
 */
CTransport*
CFullEventEditorApp::createRingSink()
{
    return CRingItemTransportFactory::createTransport(
        m_params.sink_arg, CRingBuffer::producer
    );
}
/**
 *  createUserEditor
 *     Create a new instance of the user editor object.
 *     The first time we open the shared lib and locate the
 *     factory function.  Subsequent times we just need to
 *     invoke that function.
 * @return CFullEventEditor::Editor*
 */
CFullEventEditor::Editor*
CFullEventEditorApp::createUserEditor()
{
    if (!m_pEditorFactory) {
        std::string libname = m_params.classifier_arg;
        void* pDll  =
	  dlopen(libname.c_str(), RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
        if (!pDll) {
            std::string msg = "Editor library load failed: ";
            msg += dlerror();
            throw std::runtime_error(msg);
        }
        createEditor pFactory = reinterpret_cast<createEditor>(dlsym(pDll, "createFullEventEditor"));
        if (!pFactory) {
            std::string msg = "Failed to find 'createFullEventEditor' in editor library: ";
            msg += dlerror();
            throw std::runtime_error(msg);
        }
        m_pEditorFactory = pFactory;
    }
    return (m_pEditorFactory)();
}
