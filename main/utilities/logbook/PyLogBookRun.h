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

/** @file:  PyLogBookRun.h
 *  @brief: Definitions for pythonic encapsulation of runs.
 */
#ifndef PYLOGBOOKRUN_H
#define PYLOGBOOKRUN_H

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif

#include <Python.h>
#include <vector>

class LogBook;
class LogBookRun;

// Run objects:

typedef struct {
    PyObject_HEAD
    PyObject*   m_book;          // Logbook object.
    LogBookRun* m_pRun;
} PyLogBookRun, *pPyLogBookRun;

extern PyTypeObject  PyLogBookRunType;

// Transition objects:

typedef struct {
    PyObject_HEAD
    PyObject*     m_book;
    PyObject*     m_pRun;
    int           m_nIndex;
} PyRunTransition, *pPyRunTransition;

extern PyTypeObject PyRunTransitionType;

// Useful functions:

bool PyRun_isRun(PyObject* pObject);
LogBookRun* PyRun_getRun(PyObject* pObject);
PyObject* PyRun_newRun(PyObject* logBook, LogBookRun* pRun);
PyObject* PyRun_TupleFromVector(
    PyObject* logbook, const std::vector<LogBookRun*>& runs
);

void PyRun_InitType();
#endif
