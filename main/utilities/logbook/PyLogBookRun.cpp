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

/** @file:  PyLogBookRun.cpp
 *  @brief: Implement the Python type that encapsulates LogBookRun.
 */
#include "PyLogBookRun.h"
#include "PyLogbook.h
#include "LogBook.h"
#include "LogBookPerson.h"
#include "LogBookShift.h"
#include "LogBookRun.h"
/**
 *  The Transition type only stands in the presence of a run object.
 *  It's used to provide information about each transition.
 */
// Canonicals for transitions:

/**
 * PyTransition_new
 *    Allocator
 *  @param type - pointer to the type object
 *  @param args, kwargs - ignored parameters
 *  @return PyObject* new object but not yet filled  in (see PyTransition_init)
 */
static PyObject*
PyTransition_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    pPyRunTransition self = reinterpret_cast<pPyRunTransition>(type->tp_alloc(type, 0));
    if (self) {
        self->m_pRun = nullptr;
        self->m_nIndex = -1;           // Illegal
    }
    return self;
}
/**
 * PyTransition_init
 *   Initialize transition storage. We're passed the following
 *   parameters in order:
 *   -  'run'  - PyRun object.
 *   -  'index' - Transition number (Must be valid)
 *  @param self - pointer to the object to initialize.
 *  @param args - Positional arguments.
 *  @param kwargs - Keyword arguments.
 *  @return int 0 for success, -1 on failure.
 */
static int
PyTransition_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    PyObject* pRun;
    int       index;
    const char** keywords= {"run", "index", nullptr};
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|OI", const_cast<char**>(keywords),
        &pRun, &index
    )) {
        return -1;
    }
    // Must have a logbook run and the index must be valid:
    
    if (!PyRun_isRun(pRun)) {
        PyErr_SetString(
            logbookExeptionObject,
            "Need a run object when constructing a transition"
        );
        return -1;
    }
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(pRun);
    if ((index < 0) || (index > PyRun_GetRun(pThis)->numTransitions())) {
        PyErr_SetSTring(
            logbookExecptionObject,
            "Transition index out of range in transition constructor"
        );
        return -1;
    }
    Py_INCREF(pRun);              // Hold a reference to the run .
    pThis->m_pRun = pRun;
    pThis->m_nIndex = index;
    return 0;
}
/**
 * PyTransition_dealloc
 *    Free transition storage.
 * @param self - pointer the transition.
 */
static void
PyTransition_dealloc(PyObject* self)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(pRun);
    Py_XDECREF(pThis->m_pRun);
    Py_TYPE(self)->tp_free(self);
}

/////////  Transition getters.

// get_transitionid
// get_transition
// get_transitionname
// get_transitiontime
// get_transitioncomment
// get_transitionshift

/////////// Transition tables:

//  Getters:
static PyGetSetDef PyTransitionGetters[] = {
    //name  get     set        docstring                    closure
    {"id", get_transitionid, nullptr, "Get transition primary key", nullptr},
    {"transition", get_transition, nullptr, "Get transition type id", nullptr},
    {"transition_name", get_transitionname, nullptr, "Get textual transition type"},
    {"time", get_transitiontime, nullptr, "Get transition timestamp", nullptr},
    {"comment", get_transitioncomment, nullptr, "Get transition comment", nullptr},
    {"shift", get_trasitionshift, nullptr, "Get shift on duty for transition"}
    
    
    // End sentinel:
    
    {NULL, NULL, NULL, NULL, NULL}
};

   // Type struct:
   
PyTypeObject PyRunTransitionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "LogBook.transition",
    .tp_basicsize = sizeof(PyRunTransition),
    .tp_itemsize = 0,
    .tp_dealloc  = PyTransition_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc   = "Run state transition",
    .tp_methods = nullptr,
    .tp_getset = PyTransitionGetters,
    .tp_init   = (initproc)PyTransition_init,
    .tp_new    = PyTransition_new
};

/////////////////////////////////////////////////////////////////////////////
// LogBook.Run canonicals:

/**
 * PyRun_new
 *    Allocate storage for a new LogBook.Run object.
 *
 *  @param type - pointer to type object.
 *  @param args, kwargs - positional and keyword parameters we don't care about
 *  @return PyObject* allocated strorage.nullptr if failed with an exception
 *                raised.
 */
static PyObject*
PyRun_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
        // Allocate the storage and fill in the data pointers with
        // nullptr so Py_XDECREF and delete won't choke.
        
        pPyLogBookRun self = reinterpret_cast<pPyLogBookRun>(type->tp_alloc(type, 0));
        if (self) {
            self->m_book = nullptr;
            self->m_pRun = nullptr;
        }
        return reinterpret_cast<PyObject*>(self);
}
/**
 * PyRun_init
 *    Initialize storage of a PyLogBookRun object given the constructor
 *    parameters.
 * @param self   - pointer to the PyLogBookRun storage.
 * @param args   - Positional parameters.
 * @param kwargs - Keyword arguments.
 *   Parmaters are, order 'logbook', 'run' the logbook object and the run number
 *   of the run respectively.
 * @return int  0 for success, and -1 for failure with an exception raised.
 */
static int
PyRun_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    PyObject* pBook;
    int       run;
    const char* keywords[] = {"logbook", "id", nullptr};
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|Oi", const_cast<char**>(keywords),
        &pBook, &run
    )) {
        return -1;    
    }
    // pBook must be a logbook:
    
    if (!PyLogBook_isLogBook(pBook)) {
        PyErr_SetString(
            logbookExceptionObject,
            "Construction logbook object was not a logbook"
        );
        return -1;
    }
    // Construct the underlying LogBookRun object:
    // and fill in the storage.
    
    LogBookRun* pRun(nullptr);
    LogBook*    book = PyLogBook_getLogBook(pBook);
    try {
        pRun = book->findRun(run);
        
        // Success:
        
        Py_INCREF(pBook);
        pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
        pThis->m_book = pBook;
        pThis->m_pRun = pRun;
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        return -1;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "An unanticipated exception type was caught in PyRun_init"
        );
        return -1;
    }
    return 0;                  // Success   
}
/**
 * PyRun_dealloc
 *   Destructor for run objects.
 *
 * @param self - pointer to PyLogBookRun storage.
 */
static void
PyRun_dealloc(PyObject* self)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    Py_XDECREF(pThis->m_book);
    delete pThis->m_pRun;
    Py_TYPE(self)->tp_free(self);
}


/////////////////////////////////////////////////////////////
// Log book Run attribute getters.


/////////////////////////////////////////////////////////////
// Log Book Run instance methods.

///////////////////////////////////////////////////////////
// Tables for logBook.Run type:

// Setters and getters:

