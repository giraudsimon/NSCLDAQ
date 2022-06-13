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

/** @file:  PyNote.h
 *  @brief: Python wrapping of notes
 */
#ifndef PYNOTE_H
#define PYNOTE_H
#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif

#include <Python.h>
#include <vector>
#include <LogBookNote.h>

class LogBook;
class LogBookRun;

// We have two types:

// Images in notes:

typedef struct {
    PyObject_HEAD
    PyObject* m_book;
    PyObject* m_note;
    int       m_imageIndex;
    std::string *m_imageFile;         // Images get exported on reference
} PyNoteImage, *pPyNoteImage;

extern PyTypeObject PyNoteImageType;

bool PyImage_isImage(PyObject* pObject);
const LogBookNote::NoteImage* PyImage_getImage(PyObject* pObject);
PyObject* PyImage_create(PyObject* book, PyObject* note, int index);

// Notes themselves.

typedef struct {
    PyObject_HEAD
    PyObject* m_book;
    LogBookNote* m_pNote;
} PyNote, *pPyNote;

extern PyTypeObject PyNoteType;


bool PyNote_isNote(PyObject* pObject);
LogBookNote* PyNote_getNote(PyObject* pObject);
PyObject* PyNote_create(PyObject* logbook, LogBookNote* pNote);

void PyNote_InitType();
#endif
