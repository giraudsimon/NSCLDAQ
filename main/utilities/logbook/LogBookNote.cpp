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
#include <CSqliteException.h>
#include <CSqliteStatement.h>
#include <CSqliteTransaction.h>

#include <string.h>
#include <stdlib.h>

#include <sstream>

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
LogBookNote::_NoteImage::_NoteImage(const _NoteImage& rhs) :
    s_pImageData(0), s_imageLength(0)
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
    free(s_pImageData);
    s_pImageData = malloc(s_imageLength);
    if (!s_pImageData) {
        throw LogBook::Exception(
            "Copying a NoteImage - unable to malloc image data"
        );
    }
    memcpy(s_pImageData, rhs.s_pImageData, s_imageLength);
}
////////////////////////////////////////////////////////////////
// LogBookNote implementation

/**
 * constructor
 *     This constructor constructs the note from the information
 *     about it in the database.
 *  @param db  - references the database connection object.
 *  @param id  - The primary key value of the note.
 */
LogBookNote::LogBookNote(CSqlite& db, int noteId)
{
    bool found(false);
    try {
        CSqliteStatement find(
            db,
            "SELECT run_id, note_time, note,                           \
                    note_image.id AS image_id, note_offset,            \
                    original_filename, image                           \
            FROM note                                                  \
            LEFT JOIN note_image ON note.id = note_id                  \
            WHERE note.id = ?"
        );
        find.bind(1, noteId);
        
        // Notes may not have images  in which case the fields
        // from note_image will be nulls; and we'll only get one
        // returned record.  Otherwise we'll get one returned record
        // for each image.
       
        while (!(++find).atEnd()) {
            found = true;
            m_textInfo.s_id    = noteId;
            m_textInfo.s_runId = find.getInt(0);
            m_textInfo.s_noteTime = find.getInt(1);
            m_textInfo.s_contents = find.getString(2);
            
            // We need to be sure we have image data
            // If so we emplace an image into the s_imageInfo
            // field of m_Note and fill it in from the data
            // in the record.  Note that the blob contents are
            // gotten as a pointer that's valid only until the next
            // ++find so we need to malloc/copy the data:
            
            if (find.columnType(3) != CSqliteStatement::null) {
                m_imageInfo.emplace_back();
                NoteImage& image(m_imageInfo.back());
                image.s_id = find.getInt(3);
                image.s_noteId = noteId;
                image.s_noteOffset = find.getInt(4);
                image.s_originalFilename = find.getString(5);
                image.s_imageLength      = find.bytes(6);
                image.s_pImageData       = malloc(image.s_imageLength);
                if (!image.s_pImageData) {
                    throw LogBook::Exception(
                        "Failed allocating storage for an image in LogbookNote constructor"
                    );
                } else {
                    memcpy(
                        image.s_pImageData, find.getBlob(6),
                        image.s_imageLength
                    );
                }
            }
            
        }
    }
    catch (CSqliteException& e) {
        LogBook::Exception::rethrowSqliteException(
            e, "Failed to lookupt note"
        );
    }
    
    // If found is still false, this is an error:
    
    if (!found) {
        std::stringstream msg;
        msg << "There is no note with the primary key value " << noteId;
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
}
/**
 * destructor
 */
LogBookNote::~LogBookNote()
{
    // Destructors of the images take care of the malloc'd data.
}
/**
 * getAssociatedRun
 *   Return an object encapsulating the run that 
*/
///////////////////////////////////////////////////////////////
// Static members:



/////////////////////////////////////////////////////////////
// Private members


