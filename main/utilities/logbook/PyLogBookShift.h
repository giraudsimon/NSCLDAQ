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

/** @file:  PyLogBookShift.h
 *  @brief: Definitions for a shift.
 */
#ifndef PYLOGBOOKSHIFT_H
#define PYLOGBOOKSHIFT_H

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include <Python.h>
#include <string>
#include <vector>
class LogBookShift;
class LogBook;

typedef struct {
    PyObject_HEAD
    PyObject*  m_book;            // Logbook.
    LogBookShift*  m_shift;   
} PyLogBookShift, *pPyLogBookShift;

extern PyTypeObject PyLogBookShiftType;

PyObject* PyShift_newShift(PyObject* book, LogBookShift* shift);
bool      PyShift_isShift(PyObject* p);
PyObject* PyShift_TupleFromVector(
    PyObject* book, const std::vector<LogBookShift*>& shifts
);


void PyShift_InitType();
#endif
