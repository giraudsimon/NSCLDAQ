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

/** @file:  TclNoteInstance.h
 *  @brief: Defines a class that wraps LogBookNote instances in command object.
 */
#ifndef TCLNOTEINSTANCE_H
#define TCLNOTEINSTANCE_H
#include <TCLObjectProcessor.h>
#include <TCLObject.h>

#include "LogBookNote.h"

#include <map>
#include <memory>

class LogBook;

/**
 * @class TclNoteInstnace
 *    This class wraps a LogBookNote instance in a Tcl command ensemble.
 *    The ensemble has the following subcommands:
 *
 *    -  id      - Returns the id of the note.
 *    -  run     - Returns a run command or "" for the associated run.
 *    -  author  - Returns an author encapsulated command.
 *    -  timestamp - returns a [clock seconds] at which the note was created.
 *    -  contents - the raw note contents.  This is assumed to be in Markdown format
 *    -  images  - Returns a possibily empty list of dicts.  Each dics
 *                 describes on image included in the Markdown note text See
 *                 Image Dictionaries below for more.
 *    -  substituteImages - returns the note text with:
 *                 * Any images extracted to temporary files.
 *                 * Any image references fixed up so that the links will work
 *                   to include the image inline.
 *
 *    Image Dictionaries
 *       Each image dscribed by the images subcommandis a dict with the following
 *       key/value pairs:
 *
 *       -  offset - the byte offset into the note at which its link to this
 *                   image begins.
 *       -  originalFilename - the original filename of the image.
 *       -  imageLength      - Number of bytes of image data.
 *       -  imageData  - Binary object containing the image data.  This, given
 *                       knowledge of the image type can be used to convert
 *                       the image into an actual Tcl photo presumably.
 */
class TclNoteInstance : public CTCLObjectProcessor
{
private:
    std::shared_ptr<LogBook>     m_logBook;
    std::shared_ptr<LogBookNote> m_note;
    
    static std::map<std::string, TclNoteInstance*> m_instanceMap;
    
public:
    TclNoteInstance(
        CTCLInterpreter& interp, const char* name,
        std::shared_ptr<LogBook>& pBook,
        LogBookNote* pNote
    );
    virtual ~TclNoteInstance();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    LogBookNote* getNote() {return m_note.get();}
public:
    static TclNoteInstance* getCommandObject(const std::string& name);
private:
    void id(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void run(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void author(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void timestamp(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void contents(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void images(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void substituteImages(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    CTCLObject makeImageDict(
        CTCLInterpreter& interp, const LogBookNote::NoteImage& image
    );
};

#endif