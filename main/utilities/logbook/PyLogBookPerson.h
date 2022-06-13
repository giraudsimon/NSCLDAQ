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

/** @file:  PyLogBookPerson.h
 *  @brief: Provides definitions needed by other clients of PyLogBookPerson.h
 */

#ifndef   PYLOGBOOKPERSON_H
#define   PYLOGBOOKPERSON_H

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif

#include <Python.h>
class LogBookPerson;

extern PyTypeObject PyPersonType;

bool PyPerson_isPerson(PyObject* p);
LogBookPerson* PyPerson_getPerson(PyObject* p);
int       PyPerson_IterableToVector(
    std::vector<LogBookPerson*>& result, PyObject* iterable
);
PyObject* PyPerson_newPerson(PyObject* logbook, int personId);
void PyPerson_InitType();
#endif
