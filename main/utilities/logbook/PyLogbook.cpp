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

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif

#include <Python.h>
#include "PyLogbook.h"
#include "PyLogBookPerson.h"

#include "LogBook.h"
#include "LogBookPerson.h"


#include <stdexcept>
// forward definitions

LogBook* PyLogBook_getLogBook(PyObject* obj);

// Exception type we'll be raising:

PyObject* logbookExceptionObject(nullptr);

/////////////////////////////////////////////////////////////////
// Utilities for PyLogBook object instances.

static PyObject*
newPerson(int id, PyObject* logbook)
{
    return PyObject_CallFunction(
        reinterpret_cast<PyObject*>(&PyPersonType), "IO", id, logbook
     );
}

template<class T>
void freeVector(std::vector<T> v)
{
    for (int i =0; i < v.size(); i++) delete v[i];
}

void freeTuple(PyObject* tuple)
{
    int n = PyTuple_Size(tuple);
    for (int i =0; i < n; i++) {
        PyObject* pobj = PyTuple_GetItem(tuple, i);
        Py_XDECREF(pobj);
    }
    Py_DECREF(tuple);
    
}


///////////////////////////////////////////////////////////////
// Canonicals for PyLogBook (LogBook.LogBook) class/type.

/**
 * PyLogBook_new
 *    Create the object storage for a LogBook.LogBook type instance.
 *    Note that the m_pBook member will be initialized to nullptr
 *    so that it can be deleted without penalty in case  the deatailed
 *    construction fails.
 * @param type  - Pointer to the PyLogBook type struct.
 * @param args  - Positional arguments (ignored).
 * @param kwds  - Keyword arguments (ignored).
 * @return  PyObject* - pointer to the new object (PyLogBook struct)
 *                allocated an initialized but not constructed.
 * @retval NULL if failed.
 */
static PyObject*
PyLogBook_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    // Evidently tp_alloc will incref.
    
    PyLogBook* self = reinterpret_cast<PyLogBook*> (type->tp_alloc(type, 0));
    if (self) {
        self->m_pBook = nullptr;
    }
    return reinterpret_cast<PyObject*>(self);  
}
/**
 * PyLogBook_init
 *   Actual constructor for a LogBook.LogBook object once storage
 *   was allocated.
 *
 * @param self   - pointer to the PyLogBook object that contains object
 *                 storage
 * @param args   - Positional arguments (if no keywords this is the log filename).
 * @param kwargs - we support "filename" as the filename for the logbookl.
 * @return status of the attempt, 0 for success, -1 for failure.
 *         On failure a LogBook.error is raised if construction fails.
 */
static int
PyLogBook_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    static const char* kwlist[] = {"filename", NULL};
    const char* filename(nullptr);
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|s", const_cast<char**>(kwlist), &filename)
    ) {
        return -1;
    }
    try {
        PyLogBook* pThis = reinterpret_cast<PyLogBook*>(self);
        pThis->m_pBook = new LogBook(filename);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        return -1;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unanticipated excpetion type caught in PyLogBook_init"
        );
        return -1;
    }
    return 0;
}
/**
 * PyLogBook_dealloc
 *   Destructor for an instance of LogBook.LogBook:
 *
 * @param self - pointer to the object to delete.
 */
static void
PyLogBook_dealloc(PyObject* self)
{
    PyLogBook* pThis = reinterpret_cast<PyLogBook*>(self);
    delete pThis->m_pBook;
    pThis->m_pBook = nullptr;              // For good measure.
    Py_TYPE(self)->tp_free(self);
}
/////////////////////////////////////////////////////////////////
// LogBook object methods:

/**
 * addPerson (add_person)
 *    Implements the add_person method.
 *    Creates a new person in the database and returns a LogBook.Person
 *    object that encapsulates that new person.
 *
 *  @param self - Pointer to the PyLogBook struct for this object.
 *  @param args - positional arguments.
 *  @param kwargs - Keyword parameters 'lastname', 'firstname', 'salutation'
 *  @return PyObject* - A newly created LogBook.Person object on success.
 *  @retval NULL - failed, and an exception is raised.
 */
static PyObject*
addPerson(PyObject* self, PyObject* args, PyObject* kwargs)
{
    const char* lastname(nullptr);
    const char* firstname(nullptr);
    const char* salutation(nullptr);
    const char* keywords[] = {"lastname", "firstname", "salutation", nullptr};
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|sss", const_cast<char**>(keywords),
        &lastname, &firstname, &salutation
    )) {
        return nullptr;
    }
    LogBook*   pBook = PyLogBook_getLogBook(self);
    PyObject* result = nullptr;
    try {
        LogBookPerson* person = pBook->addPerson(lastname, firstname, salutation);
        int id = person->id();
        delete person;
        
        // Now we can wrap this the Python way.
                
        result = newPerson(id, self);

    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unanticipated Exception type in LogBook.add_person"
        );
       
    }
    return result;
}/**
  * findPeople
  *  @param self - pointer to the logbook object storage.
  *  @param args - Only one argument, a potentially null where clause.
  *  @return PyObject* Possibly empty n-tuple containing the
  *               matching person objects.
  */
static PyObject*
findPeople(PyObject* self, PyObject* args)
{
    const char* where(nullptr);
    if (!PyArg_ParseTuple(args, "s", &where)) {
        return nullptr;
    }
    LogBook* book = PyLogBook_getLogBook(self);
    std::vector<LogBookPerson*> vResult;
    try {
        vResult = book->findPeople(where);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        return nullptr;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unanticipated excetpion type in LogBook.findPeople"
        );
        return nullptr;
    }
    PyObject* result = PyTuple_New(vResult.size());
    if (!result) {
        return nullptr;
    }
    
    try {
        for (int i =0; i < vResult.size(); i++) {
            PyTuple_SET_ITEM(result, i, newPerson(vResult[i]->id(), self));
            delete vResult[i];
            vResult[i] = nullptr;        // Prevent double del on catch
        }
    }
    catch (LogBook::Exception& e) {
        freeVector(vResult);
        freeTuple(result);
        PyErr_SetString(
            logbookExceptionObject, e.what()
        );
        return nullptr;
    }
    catch (...) {
        freeVector(vResult);
        freeTuple(result);
        PyErr_SetString(
            logbookExceptionObject,
            "Unanticipated exception in LogBook.findPeople"
        );
        return nullptr;
    }
    
    return result;
}


///////////////////////////////////////////////////////////////
// Table for the PyLogBook type (LogBook.LogBook)

static PyMethodDef PyLogBook_methods [] = {   // methods
    {
        "add_person", (PyCFunction)addPerson,
        METH_VARARGS | METH_KEYWORDS, "Add a new person."
    },
    {
        "find_people", findPeople, METH_VARARGS, "Find matching people"
    },

    // Ending sentinel:
    
     {NULL, NULL, 0, NULL}
};

// Python's Type definition for PyLogBook:

static PyTypeObject PyLogBookType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "LogBook.LogBook",
    .tp_basicsize = sizeof(PyLogBook),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)PyLogBook_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc  = "Logbook Class",
    .tp_methods = PyLogBook_methods,  
    .tp_init = (initproc)PyLogBook_init,
    .tp_new   = PyLogBook_new
};

/////////////////////////////////////////////////////////////////
// LogBook.LogBook utilities for other type implementations:

/**
 * PyLogBook_ isLogBook
 *   @param obj - An object.
 *   @return bool -true if obj is a logbook as determined by PyObject_IsInstance
 */
bool
PyLogBook_isLogBook(PyObject* obj)
{
    return PyObject_IsInstance(
        obj, reinterpret_cast<PyObject*>(&PyLogBookType)
    ) != 0;
}
/**
 * PyLogBook_getLogBook(PyObject* obj)
 *
 *  @param obj - Object that's already been validatated to be a logbook.
 *  @return LogBook* - Pointer to the encapsulated C++ logbook object.
 */
LogBook*
PyLogBook_getLogBook(PyObject* obj)
{
    PyLogBook* pbook = reinterpret_cast<PyLogBook*>(obj);
    return pbook->m_pBook;
}
///////////////////////////////////////////////////////////////////////////////
// Module level code for LogBook:

/**
 * create
 *    Create a new logbook.
 * @param self - module object.
 * @param args - parameter list.   We require four string like parameters:
 *              - filename     - Path to the logbook file.
 *              - experiment   - Experiment designation (e.g. '0400x').
 *              - spokesperson - Name of the experiment spokesperson (e.g. 'Ron Fox)
 *              - purpose      - Brief experiment purpose.
 * @return PyObject* - PyNULL if successful else an exception gets
 *                raised normally of type LogBook.error
 */
PyObject*
create(PyObject* self, PyObject* args)
{
    const char* filename;
    const char* experiment;
    const char* spokesperson;
    const char* purpose;
    
    if (!PyArg_ParseTuple(
        args, "ssss", &filename, &experiment, &spokesperson, &purpose)
    ) {
        return NULL;        // ParseTuple raise the exception
    }
    try  {
        LogBook::create(filename, experiment, spokesperson, purpose);
        Py_RETURN_NONE;
    }
    catch (std::exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "LogBook.create threw an unanticipated exception type"
        );
    }
    return NULL;
    
    
}

///////////////////////////////////////////////////////////////////////////////
// Stuff needed to make the module happen.

/**
 * logbookMethodTable
 *     This table defines LogBook module level methods - these correspond
 *     to the static methods of the C++ API's LogBook class.
 */
static PyMethodDef logbookMethodTable[] = {
    
    {"create", create, METH_VARARGS, "Create a new logbook"},

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
        // Ready LogBookType.
        
        if (PyType_Ready(&PyLogBookType) < 0) {
            return NULL;
        }
        
        // Ready  Person type.
        
        if (PyType_Ready(&PyPersonType) < 0) {
            return NULL;
        }
        
        // Initialize our module:
        
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
            
            // Add the log book type:
            
            Py_INCREF(&PyLogBookType);
            if (PyModule_AddObject(
                module, "LogBook",
                reinterpret_cast<PyObject*>(&PyLogBookType)
            ) < 0) {
                Py_DECREF(&PyLogBookType);
                Py_XDECREF(logbookExceptionObject);
                Py_DECREF(module);
                return NULL;
            }
            // Now add the LogBook.Person type:
            
            Py_INCREF(&PyPersonType);
            if (PyModule_AddObject(
                module, "Person",
                reinterpret_cast<PyObject*>(&PyPersonType))
                < 0
            )  {
                Py_DECREF(&PyPersonType);
                Py_DECREF(&PyLogBookType);
                Py_XDECREF(logbookExceptionObject);
                Py_DECREF(module);
                
                return NULL;
            }
        }
        return module;
    }
}                 // Extern C - required for Python interpreter linkage.