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

/** @file:  PyLogBook.cpp
 *  @brief: Python module to provide an API for the logbook subsystem.
 */
/**
 *  This module provides the Logbook module which is an API to the
 *  nscldaq supported logbook. Note that while it's tempting to build a pure
 *  python interface on top of the python Sqlite module, that would introduce
 *  maintenance problems if the logbook schema e.g. changed.  This module
 *  is therefore a compiled module built on to pof the C++ API.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

static PyObject* logbookExceptionObject(nullptr);


/**
 * logbookMethodTable
 *     This table defines LogBook module level methods - these correspond
 *     to the static methods of the C++ API's LogBook class.
 */
static PyMethodDef logbookMethodTable[] = {
    
    //   End of methods sentinnel
    
    {NULL, NULL, 0, NULL}
};

/**
 * logbookModuleTable
 *    The logbook python module definition table.  Top level description
 *    of all of the module.
 */
static PyModuleDef logbookModuleTable = {
    PyModuleDef_HEAD_INIT,
    "LogBook",                       // Module name.
    nullptr,                         // at present no docstrings
    -1,                              // No per interpreter state.
    logbookMethodTable               // method table for LogBook. methods.
};

/**
 * PyInit_LogBook
 *    The logbook module initialization function.  Registers the module
 *    and the LogBook.exception exception we will use to report errors.
 *
 * Depends on:
 *    - logbookModuleTable  - the log book module table.
 *    - logbookExceptionObject - the logbook exception type.
 */

extern "C"
{
    PyMODINIT_FUNC
    PyInit_LogBook()
    {
        PyObject* module= PyModule_Create(&logbookModuleTable);
        if (module) {
            // with the module initialized we need to create the exception.
            
            logbookExceptionObject =
                PyErr_NewException("LogBook.error", NULL, NULL);
            Py_XINCREF(logbookExceptionObject);
            if (PyModule_AddObject(module, "error", logbookExceptionObject) < 0) {
              Py_XDECREF(logbookExceptionObject);
              Py_CLEAR(logbookExceptionObject);
              Py_DECREF(module);                   // deletes the module.
              return nullptr;
            }
            
        }
        return module;
    }
}                 // Extern C - required for Python interpreter linkage.