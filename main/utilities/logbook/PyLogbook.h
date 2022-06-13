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

/** @file:  PyLogBook.h
 *  @brief: Header for PYLogBook subclasses.
 * 
 */
#ifndef PYLOGBOOK_H
#define PYLOGBOOK_H
#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif

#include <Python.h>
#include "LogBook.h"

class LogBook;
class LogBookPerson;

// Logbook instance object storage shape.

typedef struct {
    PyObject_HEAD
    LogBook*  m_pBook;
} PyLogBook;

// LogBook.error exception

extern PyObject* logbookExceptionObject;


// Useful functions:

extern bool PyLogBook_isLogBook(PyObject* pObject);
LogBook* PyLogBook_getLogBook(PyObject* pObject);
PyObject* PyLogBook_TupleFromPeople(
    PyObject* book, const std::vector<LogBookPerson*>& people
);
void freeTuple(PyObject* tuple);
template<class T>
void freeVector(std::vector<T> v)
{
    for (int i =0; i < v.size(); i++) delete v[i];
}
#endif