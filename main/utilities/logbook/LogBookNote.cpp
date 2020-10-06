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

/** @file:  LogBookNote.cpp
 *  @brief: Impelement the log book note class.
 */
#include "LogBookNote.h"
#include "LogBookRun.h"
#include "LogBook.h"

#include <CSqlite.h>
#include <string.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////
// NoteImageCanonicals.

/**
 * DefaultConstructor
 *    Just initialize the image information:
 */
LogBookNote::_NoteImage::_NoteImage() :
    s_imageLength(0),
    s_pImageData(nullptr)
{}
/**
 * copy constructor:
 *
 * @param rhs - the object we're copying.
 */
LogBookNote::_NoteImage::_NoteImage(const _NoteImage& rhs)
{
    CopyIn(rhs);
}
/**
 * destructor:
 */
LogBookNote::_NoteImage::~_NoteImage()
{
    free(s_pImageData);            // No-op if nullptr.
}
/**
 * assignment
 *    @param rhs - right hand side of the assignment.
 *    @return *this
 */
LogBookNote::_NoteImage&
LogBookNote::_NoteImage::operator=(const _NoteImage& rhs)
{
    if (this != &rhs) {
        CopyIn(rhs);
    }
    return *this;
}
/**
 * CopyIn
 *   Common code to copy from a rhs object to this.
 * @param rhs - What we copy from
 */
void
LogBookNote::_NoteImage::CopyIn(const _NoteImage& rhs)
{
    s_id               = rhs.s_id;
    s_noteId           = rhs.s_noteId;
    s_noteOffset       = rhs.s_noteOffset;
    s_originalFilename = rhs.s_originalFilename;
    s_imageLength      = rhs.s_imageLength;
    memcpy(s_pImageData, rhs.s_pImageData, s_imageLength);
}
////////////////////////////////////////////////////////////////
// LogBookNote implementation

///////////////////////////////////////////////////////////////
// Static members:



/////////////////////////////////////////////////////////////
// Private members
