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

/** @file:  TclNoteInstance.cpp
 *  @brief: Implement the Tcl wrapping of a logbook note.
 */
#include "TclNoteInstance.h"
#include "TclLogBookInstance.h"
#include "TclRunInstance.h"
#include "TclLogbook.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "LogBookRun.h"
#include "LogBookNote.h"
#include "LogBook.h"
#include <Exception.h>
#include <stdexcept>
#include <sstream>

std::map<std::string, TclNoteInstance*> TclNoteInstance::m_instanceMap;

/**
 * constructor:
 *    @param interp - interpreter on which the command will be registered.
 *    @param name   - name of the new command.
 *    @param pBook  - Pointer to a logbook we will include in our encapsulation
 *    @param pNote  - Pointer to the note object we are encapsulating
 */
TclNoteInstance::TclNoteInstance(
    CTCLInterpreter& interp, const char* name, std::shared_ptr<LogBook>& pBook,
        LogBookNote* pNote
) :
    CTCLObjectProcessor(interp, name, true),
    m_logBook(pBook), m_note(pNote)
{
    // Register the command in the instance map:
    
    m_instanceMap[name] = this;
}
/**
 * destructor
 *    Just remove ourself from the instance map.  The smart pointers
 *    will take care of any additional object destruction we may need.
 */
TclNoteInstance::~TclNoteInstance()
{
    auto p = m_instanceMap.find(getName());
    if (p != m_instanceMap.end()) m_instanceMap.erase(p);
}
/**
 * operator()
 *    Called when the command ensemble's base command is issued.
 *    Currently a stub
 *  @param interp - interpreter running the command.
 *  @param objv   - command line words.
 *  @return int   - TCL_OK - success, TCL_ERROR -failure.
 */
int
TclNoteInstance::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    try {
        bindAll(interp, objv);
        requireAtLeast(objv, 2, "Usage: <note-instance> <subcommand> ?...?");
        std::string subcommand = objv[1];
        
        if (subcommand == "destroy") {
            delete this;
        } else if (subcommand == "id") {
            id(interp, objv);
        } else if (subcommand == "run") {
            run(interp, objv);
        } else if (subcommand == "author") {
            author(interp, objv);
        } else if (subcommand == "timestamp") {
            timestamp(interp, objv);
        } else if (subcommand == "contents") {
            contents(interp, objv);
        } else if (subcommand == "images") {
            images(interp, objv);
        } else if (subcommand == "substituteImages") {
            substituteImages(interp, objv);
        } else {
            std::stringstream msg;
            msg << subcommand << " is not a valid subcommand for a note instance";
            std::string e(msg.str());
            throw e;
        }
    }
    catch (std::string& msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;

    }
    catch (std::exception& e) {       // Note LogBook::Exception is derived from this
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult(
            "Unexpected exception type caught in TclPersonInstance::operator()"
        );
        return TCL_ERROR;
    }

    return TCL_OK;
}
//////////////////////////////////////////////////////////////////////////////
// Static methods

/**
 * getCommandObject
 *    Return a pointer to the command object given its name
 * @param name -command name string.
 * @return TclNoteInstance* - pointer to the command object with that name.
 */
TclNoteInstance*
TclNoteInstance::getCommandObject(const std::string& name)
{
    auto p = m_instanceMap.find(name);
    if (p == m_instanceMap.end()) {
        std::stringstream msg;
        msg << "There is no Tcl note instance command: " << name;
        std::string e(msg.str());
        throw std::out_of_range(e);
    }
    return p->second;
}
//////////////////////////////////////////////////////////////////////////
//  Private command execution methods.

/**
 * id
 *   Set the result with the id (primary key) of the note in its root table.
 *
 * @param interp - interpreter running the command.
 * @param objv   - command words in the command.
 */
void
TclNoteInstance::id(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <note-instance> id");
    
    CTCLObject result;
    result.Bind(interp);
    result = m_note->getNoteText().s_id;
    interp.setResult(result);
}
/**
 * run
 *   Set the result with the run encapsulation of the run under which the
 *   note was created.  If no note was associate with the run, an empty
 *   string is returne.
 * @param interp - interpreter running the command.
 * @param objv   - The command words.
 */
void
TclNoteInstance::run(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <note-instance> run");
    
    int runId = m_note->getNoteText().s_runId;
    if (runId > 0) {
        LogBookRun* pRun = m_logBook->getRun(runId);
        TclLogBookInstance tempBook(
            &interp, TclLogbook::createObjectName("__temp__").c_str(), m_logBook
        );
        interp.setResult(tempBook.wrapRun(interp, pRun));
    } else {
        interp.setResult("");
    }
}
/**
 * author
 *    sets the result to  a wrapped person that for the note's author.
 * @param interp - interpreter running the command.
 * @param objv   - The command words.
 */
void
TclNoteInstance::author(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <note-instance> author");
    int authorId = m_note->getNoteText().s_authorId;
    LogBookPerson* pPerson = m_logBook->getPerson(authorId);
    TclLogBookInstance tempBook(
        &interp, TclLogbook::createObjectName("__temp__").c_str(), m_logBook
    );
    interp.setResult(tempBook.wrapPerson(interp, pPerson));
}
/**
 * timestamp
 *    Sets the result to the [clock seconds] at which the note was written.
 * @param interp -interpreter running the command.
 * @param objv  - command words.
 */
void
TclNoteInstance::timestamp(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <note-instance> timestamp");
    CTCLObject result;
    result.Bind(interp);
    result = (Tcl_WideInt)(m_note->getNoteText().s_noteTime) ;
    interp.setResult(result);
}
/**
 * contents
 *    Sets the result to the raw note contents.
 *  @param interp - interpreter running the command.
 *  @param objv   - The command words.
 *  
 */
void
TclNoteInstance::contents(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <note-instance> contents");
    interp.setResult(m_note->getNoteText().s_contents);
}
/**
 * images
 *    Sets the result to a list of dicts, one for each image the note
 *    includes.  See the header for the key/values these dicts have.
 * @param interp - interpreter running the command.
 * @param objv   - The command words.
 */
void
TclNoteInstance::images(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <noteinstance> images");
    CTCLObject result;
    result.Bind(interp);
    
    size_t nImages = m_note->imageCount();
    for (int i =0; i < nImages; i++) {
        CTCLObject item = makeImageDict(interp, (*m_note)[i]);
        result += item;
    }
    interp.setResult(result);
}
/**
 * substituteImages
 *    -   Exports the note's images to files in the notebook temp directory.
 *    -   Edits the note's references to the images to match where they now live.
 *    -   Sets the result with the resulting text.
 *
 *   @param interp - interpreter running the command.
 *   @param objv   - The command words.
 */
void
TclNoteInstance::substituteImages(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "Usage: <note-image> substituteImages");
    interp.setResult(m_note->substituteImages());
}

//////////////////////////////////////////////////////////////////////////////
// Utility methods (private).

/**
 * makeImageDict
 *     Creates a single dict that describes an image in a note.
 *  @param interp  - interpreter used to create the dict.
 *  @param image   - Const reference to the image definitions.
 *  @return CTCLObject - Object encapsulated dict.
 */
CTCLObject
TclNoteInstance::makeImageDict(
    CTCLInterpreter& interp, const LogBookNote::NoteImage& image
)
{
    Tcl_Obj* pDict = Tcl_NewDictObj();
    Tcl_Obj* key;
    Tcl_Obj* value;
    
    // offset:
    
    key  = Tcl_NewStringObj("offset", -1);
    value = Tcl_NewIntObj(image.s_noteOffset);
    Tcl_DictObjPut(interp, pDict, key, value);
    
    // originalFilename
    
    key = Tcl_NewStringObj("originalFilename", -1);
    value =Tcl_NewStringObj(image.s_originalFilename.c_str(), -1);
    Tcl_DictObjPut(interp, pDict, key, value);
    
    // imageLength
    
    key = Tcl_NewStringObj("imageLength", -1);
    value = Tcl_NewIntObj(image.s_imageLength);
    Tcl_DictObjPut(interp, pDict, key, value);
    
    // imageData:
    
    key = Tcl_NewStringObj("imageData", -1);
    value = Tcl_NewByteArrayObj(
        reinterpret_cast<const unsigned char*>(image.s_pImageData),
        image.s_imageLength
    );
    Tcl_DictObjPut(interp, pDict, key, value);
    
    CTCLObject result(pDict);
    result.Bind(interp);
    return result;
}

