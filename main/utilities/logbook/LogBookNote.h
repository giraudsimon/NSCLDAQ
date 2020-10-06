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

/** @file:  LogBookNote.h
 *  @brief: Define interface to notes.
 */
#ifndef LOGBOOKNOTE_H
#define LOGBOOKNOTE_H
#include <vector>
#include <string>
#include <time.h>
class CSqlite;
class LogBookRun;
/**
 * LogBookNote is a class that provides support for textual notes
 * in Markdown format.  The notes can have associated images that
 * are stored in the database as well.  The note text and associated
 * images are considered a single unit from that standpoint.
 * The class also provides a mechanism for substituting the images
 * into the markdown so that it can be rendered for external use.
 * When that is done, the images are extracted into a temporary directory
 * and references to them in the original markup are fixed up to point
 * to the extracted image.
 */
class LogBookNote
{
public:
    typedef struct _NoteText {
        int        s_id;
        int        s_runId;     // Associated run.
        time_t     s_noteTime;
        std::string s_contents;
    } NoteText, *pNoteText;
    typedef struct _ImageInfo {
        std::string s_filename;
        size_t      s_offset;
    } ImageInfo, *pImageInfo;
    typedef struct _NoteImage {
        int         s_id;
        int         s_noteId;
        int         s_noteOffset;
        std::string s_originalFilename;
        size_t      s_imageLength;
        void*       s_pImageData;
        _NoteImage();
        _NoteImage(const _NoteImage& rhs);
        ~_NoteImage();
        _NoteImage& operator=(const _NoteImage& rhs);
        void CopyIn(const _NoteImage& rhs);
    } NoteImage, *pNoteImage;
    
private:
    NoteText               m_textInfo;
    std::vector<NoteImage> m_imageInfo;
public:
    LogBookNote(CSqlite& db, int noteId);
    virtual ~LogBookNote();
    LogBookNote(const LogBookNote& rhs);
    LogBookNote& operator=(const LogBookNote& rhs);
    
    LogBookRun* getAssociatedRun() const;
    const NoteText& getNoteText() const;
    size_t          imageCount() const;
    const NoteImage& operator[](int n) const;
    std::string substituteImages(CSqlite& db);
    
    
    static LogBookNote* create(
        CSqlite& db, LogBookRun* run, const char* string,
        const std::vector<ImageInfo>& images
    );
    static std::vector<int> listRunNoteIds(CSqlite& db, int runId);
    static std::vector<int> listNonRunNotes(CSqlite& db);
    static std::vector<LogBookNote*> getRunNotes(CSqlite& db, int runId);
    
private:
    void CopyIn(const LogBookNote& rhs);
    void ExportImage(int index);
    void freeData();
};

#endif