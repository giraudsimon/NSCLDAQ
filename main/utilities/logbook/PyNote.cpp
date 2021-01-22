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

/** @file:  PyNote.cpp
 *  @brief: Implementation of the LogBook.Note type.
 */
#include "PyNote.h"
#include "LogBookNote.h"
#include "LogBookRun.h"
#include "LogBook.h"

#include "PyLogbook.h"
#include "PyLogBookRun.h"
#include "PyLogBookPerson.h"

///////////////////////////////////////////////////////////////////
// Canonicals for LogBook.Image.

/**
 * PyImage_new
 *    Allocate storage for a new logbook image object.  These
 *    represent images that may be embedded in the markdown of the
 *    note.  This embeds the logbook, the note and the index of the
 *    image in the note.
 *  @param PyTypeObject* type - pointer to the type table.
 *  @param PyObject*     args - Ignored positional parameters.
 *  @param PyObject*   kwargs - Ignored positional parameters.
 *  @return PyObject* - Pointer to the allocated object.
 *  @note it's up to PyImage_init to actually initialize the object
 *        with meaningful data. Our initialization is just enought
 *        to support a meaningfull destruction of the object in
 *        PyImage_dealloc independent of whether or not the object
 *        was completely constructed.
 */
static PyObject*
PyImage_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    pPyNoteImage self = reinterpret_cast<pPyNoteImage>(type->tp_alloc(type, 0));
    if (self) {
        self->m_book = nullptr;
        self->m_note = nullptr;
        self->m_imageIndex = -1;
        self->m_imageFile = nullptr;
        
    }
    
    return reinterpret_cast<PyObject*>(self);
}
/**
 * PyImage_init
 *    Initialize a logbook image object that's been allocated
 *    by PyImage_new - it's here that we process any
 *    parameters to the constructor and shove appropriate values in our
 *    member data slots.
 *
 *  @param self   - pointer to the storage allocated by PyImage_new
 *  @param args   - Positional parameters.
 *  @param kwargs - Keyword parameters.
 *  @return int   - 0 for success, -1 for failure.
 *  @note The parmeters are 'logbook', 'note', 'index' in that order
 *        positionally to represent the logbook object, the note object
 *        and the index of the image within the note.
 *    
 */
static int
PyImage_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    PyObject* logbook(nullptr);
    PyObject* note(nullptr);
    int       index(-1);
    const char* keywords[] = {"logbook", "note", "index", nullptr};
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs,"|OOi", const_cast<char**>(keywords),
        &logbook, &note, &index
    )) {
        return -1;                 // epic fail.
    }
    // Validate the parameters:
    
    if (!logbook || !PyLogBook_isLogBook(logbook)) {
        PyErr_SetString(
            logbookExceptionObject,
            "'logbook' parameter is required and must be a logbook object"
        );
        return -1;
    }
    if (!note || !PyNote_isNote(note)) {
        PyErr_SetString(
            logbookExceptionObject,
            "'notebook' parameter is required and must be a note object"
        );
        return -1;
    }
    LogBookNote* pNote = PyNote_getNote(note);
    if ((index < 0) || (index >= pNote->imageCount())) {
        PyErr_SetString(
            logbookExceptionObject,
            "'index' parameter is required and must be a valid image index for the note"
        );
        return -1;
    }
    // Now that we've validated everything we can fill in the object:
    
    pPyNoteImage pThis = reinterpret_cast<pPyNoteImage>(self);
    Py_INCREF(logbook);
    Py_INCREF(note);
    pThis->m_book = logbook;
    pThis->m_note = note;
    pThis->m_imageIndex = index;
    
    
    return 0;
}
/**
 * PyImage_dealloc
 *    Frees resources associated with an image object.
 *    The logbook and note are un-referenced and we free our storage.
 * @param self - Pointer to a PyNoteImage object to delete.
 */
void
PyImage_dealloc(PyObject* self)
{
    pPyNoteImage pThis = reinterpret_cast<pPyNoteImage>(self);
    
    Py_XDECREF(pThis->m_book);
    Py_XDECREF(pThis->m_note);
    delete pThis->m_imageFile;
    
    Py_TYPE(self)->tp_free(self);       // free the storage.
}
/////////////////////////////////////////////////////////////////
// Getters for the attributes of the image:

/**
 * PyImage_getId
 *  get the image primary key.
 * @param self   - Pointer to this image.
 * @param closure - unsed closure parameter.
 * @return PyObject* - integer primary key.
 */
static PyObject*
PyImage_getId(PyObject* self, void* closure)
{
    auto pImage = PyImage_getImage(self);
    return PyLong_FromLong(pImage->s_id);
}

/**
 *  getNoteId
 *    Get the associate note's primary key.
 * @param self   - Pointer to this image.
 * @param closure - unsed closure parameter.
 * @return PyObject* - integer primary key of note.
 */
static PyObject*
PyImage_getNoteId(PyObject* self, void* closure)
{
    auto pImage = PyImage_getImage(self);
    return PyLong_FromLong(pImage->s_noteId);
}

/**
 * getIndex
 *    The the image's index. Notes can have zero or more
 *    images associated with them. This returns the index
 *    of the note represented by this object.
 *  @param self   - Pointer to this image.
 * @param closure - unsed closure parameter.
 * @return PyObject* - integer index.
 */
static PyObject*
PyImage_getIndex(PyObject* self, void* closure)
{
    pPyNoteImage pThis = reinterpret_cast<pPyNoteImage>(self);
    return PyLong_FromLong(pThis->m_imageIndex);
}

/**
 *   getOffset
 * Get the image's placement offset.  This value is the offset into
 * the note text at which the image embed text starts.
 *
 *  @param self   - Pointer to this image.
 * @param closure - unsed closure parameter.
 * @return PyObject* - integer index.
 */
static PyObject*
PyImage_getOffset(PyObject* self, void* closure)
{
    auto image = PyImage_getImage(self);
    return PyLong_FromLong(image->s_noteOffset);
}
/**
 * getOriginalFilename 
 *    Get the image's original filename.  This filenam is realy only used
 *    for documentation purposes and to seed the creation of the
 *    exported image file (see below).
 * @param self   - Pointer to this image.
 * @param closure - unsed closure parameter.
 * @return PyObject* - Containing the original filename.
 */
static PyObject*
PyImage_getOriginalFilename(PyObject* self, void* closure)
{
    auto image = PyImage_getImage(self);
    return PyUnicode_FromString(image->s_originalFilename.c_str());
}

/* getExportedFile
 *    Export (if needed) the image and get the exported filename.
 *    Note that this is needed because Python has a hard time of dealing
 *    with image data and internally converting it to an image.  Furthermore,
 *    all python image handling is done by external packages which we
 *    don't know have been loaded by the caller.  WHat we do to give
 *    access to the image data, therefore, is to export the image,
 *    if it's not yet been exported by this object and return the
 *    filename/path at which the exported image lives.
 * @param self   - Pointer to this image.
 * @param closure - unsed closure parameter.
 * @return PyObject* - Containing the exported filename.
 */
static PyObject*
PyImage_getExportedFile(PyObject* self, void* closure)
{
    pPyNoteImage pThis = reinterpret_cast<pPyNoteImage>(self);
    auto pNote = PyNote_getNote(pThis->m_note);
    auto image = PyImage_getImage(self);
    
    if (!pThis->m_imageFile) {
        std::string exportedFilename = pNote->exportImage(*image);
        pThis->m_imageFile = new std::string(exportedFilename);
    }
    return PyUnicode_FromString(pThis->m_imageFile->c_str());
}

///////////////////////////////////////////////////////////////
// Tables for the LogBook.Image type:

// Getter and setters:

static PyGetSetDef accessors[] = {
    {"id",  PyImage_getId,  nullptr, "Image primary key", nullptr},
    {"noteid", PyImage_getNoteId, nullptr, "Image note's primary key", nullptr},
    {"index", PyImage_getIndex, nullptr, "Image's index in the note"},
    {
        "offset", PyImage_getOffset, nullptr, "Image's position in  note text",
        nullptr
    },
    {
        "original_file", PyImage_getOriginalFilename, nullptr,
        "Path to original image file", nullptr
    },
    {
        "exported_file", PyImage_getExportedFile, nullptr,
        "Path to exported image file", nullptr
    },
    
    
    // End sentinell:
    // name,     getter,     setter,    docstring,    closure
    {nullptr,    nullptr,    nullptr,  nullptr,       nullptr}
};

// Type object:

PyTypeObject PyNoteImageType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    /** Can't do this in e.g. g++-v4.9.2
    .tp_name = "LogBook.Image",
    .tp_basicsize = sizeof(PyNoteImage),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)(PyImage_dealloc),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc  = "LogBook Image class - image associated with a note.",
    .tp_methods = nullptr,
    .tp_getset  = accessors,
    .tp_init = (initproc)PyImage_init,
    .tp_new  = PyImage_new  
    */
};
/**
 *  g++-4.9.2 e.g. requires active initialization as designated member
 *  initialization is not supported.
 */
static void
PyImage_InitType()
{
  PyNoteImageType.tp_name = "LogBook.Image";
  PyNoteImageType.tp_basicsize = sizeof(PyNoteImage);
  PyNoteImageType.tp_itemsize = 0;
  PyNoteImageType.tp_dealloc = (destructor)(PyImage_dealloc);
  PyNoteImageType.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
  PyNoteImageType.tp_doc  = "LogBook Image class - image associated with a note.";
  PyNoteImageType.tp_methods = nullptr;
  PyNoteImageType.tp_getset  = accessors;
  PyNoteImageType.tp_init = (initproc)PyImage_init;
  PyNoteImageType.tp_new  = PyImage_new;
}
///////////////////////////////////////////////////////////////////////
// PyNote canonicals:

/**
 * PyNote_new
 *    Allocate the storate associated with a LogBook.Note object.
 *    The storage is not completely filled in.  PyNote_init takes
 *    care of that.
 * @param type - Pointer to the type object that describes LogBook.Note
 * @param args, kwargs - unused positional and keyword parameters.
 * @return PyObject*   - the allocated storage as a partially initialized object.
 */
static PyObject*
PyNote_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    pPyNote pThis = reinterpret_cast<pPyNote>(type->tp_alloc(type, 0));
    if (pThis) {
        pThis->m_book  = nullptr;
        pThis->m_pNote = nullptr;
   }
    return reinterpret_cast<PyObject*>(pThis);
}
/**
 * PyNote_init
 *   Initialize the storage allocated by PyNote_new to
 *   turn it into a functional object.
 * @oaram self    - pointer to the storage.
 * @param args    - Positional parameters.
 * @param kwargs  - Keywords parameters.
 *                  We need a logbook and a note's primary key.
 *                  These are 'logbook', and 'note'.
 * @return int - 0 if successful and -1 if not.
 */
static int
PyNote_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    PyObject* notebook(nullptr);
    int       id(-1);
    const char* keywords[] = {"logbook", "id", nullptr};
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|Oi", const_cast<char**>(keywords),
        &notebook, &id
    )) {
        return -1;
    }
    // The notebook has to be a logbook:
    
    if (!PyLogBook_isLogBook(notebook)) {
        PyErr_SetString(
            logbookExceptionObject,
            "The 'logbook' parameter must be a LogBook.LogBook object"
        );
        return -1;
    }
    LogBook* pLogBook = PyLogBook_getLogBook(notebook);
    int result = 0;
    try {
        pPyNote pNote = reinterpret_cast<pPyNote>(self);
        pNote->m_pNote = pLogBook->getNote(id);
        pNote->m_book = notebook;
        Py_INCREF(pNote->m_book);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(
            logbookExceptionObject,
            e.what()
        );
        result = -1;
    }
    catch (...) {  
        PyErr_SetString(
            logbookExceptionObject,
            "PyNote_init(constructor) failed with an unanticipated exception type"
        );
        result = -1;
    }
    
    return result;
}
/**
 * PyNote_dealloc
 *    Deallocate storage and resources for a LogBook.Note object.
 * @param self - pointer to the object.
 */
static void
PyNote_dealloc(PyObject* self)
{
    pPyNote pThis = reinterpret_cast<pPyNote>(self);
    delete pThis->m_pNote;
    Py_XDECREF(pThis->m_book);
}
///////////////////////////////////////////////////////////////
/// PyNote getters:

/**
 *  getId
 *     Get the primary key.  Return the primary key of the note.
 * @param self   - pointer to the note object storage.
 * @param closure - unused closure object.
 * @return PyObject* - integer object containing the key.
 */
static PyObject*
PyNote_getid(PyObject* self, void* closure)
{
    LogBookNote* pNote = PyNote_getNote(self);
    int id = pNote->getNoteText().s_id;
    return PyLong_FromLong(id);
}

/**
 * getRun
 *    Return the run object associated with this note or None
 * @param self - pointer to the LogBook.Note object that is us.
 * @param closrue - unused closure parameter.
 * @return PyObject*  If there's an associated run else None.
 */
static PyObject*
PyNote_getRun(PyObject* self, void* closure)
{
    pPyNote pThis = reinterpret_cast<pPyNote>(self);
    
    auto pNote = PyNote_getNote(self);
    auto pBook = PyLogBook_getLogBook(pThis->m_book);
    auto pRun  = pBook->getNoteRun(*pNote);
    if (pRun) {
        return PyRun_newRun(pThis->m_book, pRun);
    } else {
        Py_RETURN_NONE;
    }
}
/**
 *  getTime
 *     Return the timestamp at which the note was created.
 *     A datetime object can be created from this timestamp.
 * @param self - pointer to the LogBook.Note object that is us.
 * @param closrue - unused closure parameter.
 * @return PyObject*  If there's an associated run else None.
 */
static PyObject*
PyNote_getTime(PyObject* self, void* closure)
{
    auto note = PyNote_getNote(self);
    return PyLong_FromLong(note->getNoteText().s_noteTime);
}
/**
 * getContents 
 *    Get the Text of the note (see also
 *    the substituteImages method) which takes this text and
 *    returns it with fixed up image links so that it all works.
 * 
 * @param self - pointer to the LogBook.Note object that is us.
 * @param closrue - unused closure parameter.
 * @return PyObject*  If there's an associated run else None.
 */
static PyObject*
PyNote_getContents(PyObject* self, void* closure)
{
    auto note = PyNote_getNote(self);
    return PyUnicode_FromString(note->getNoteText().s_contents.c_str());
}
/**
 * PyNote_getAuthor
 *    Return the author as a person object.
 * @param PyObject* self - pointer to this object.
 * @param closure - ignored.
 * @return PyObject* Pointer to the author LogBook.Person object.
 */
static PyObject*
PyNote_getAuthor(PyObject* self, void* closure)
{
    auto note = PyNote_getNote(self);
    int authorId = note->getNoteText().s_authorId;
    pPyNote pNote = reinterpret_cast<pPyNote>(self);
    return  PyPerson_newPerson(pNote->m_book, authorId);
}
//////////////////////////////////////////////////////////////
// LogBook.Note object methods:


/**
 *  imageCount
 *     Return the number of images the note has.
 *  @param self - pointer to our object storage.
 *  @param args - unused args as there are none.
 *  @return PyObject* integer number of images.
 */
static PyObject*
PyNote_imageCount(PyObject* self, PyObject* args)
{
    auto pNote = PyNote_getNote(self);
    return PyLong_FromLong(pNote->imageCount());
}
/**
 *   image
 *     Given a valid image index, returns an image object
 *     (LogBook.Image) for that object.
 *  @param self -pointer to our object storage.
 *  @param args - positional parameters - image index.
 *  @return PyObject* new image object.
 *
 */
static PyObject*
PyNote_image(PyObject* self, PyObject* args)
{
    // get the index and be sure it's legitimate.
    
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        return nullptr;
    }
    
    auto pNote = PyNote_getNote(self);
    if ((index < 0) || (index >= pNote->imageCount())) {
         PyErr_SetString(
            logbookExceptionObject,
            "Index out of range attempting to fetch an image in LogBook.Note.image method"
         );
         return nullptr;
    }
    pPyNote pNoteObj = reinterpret_cast<pPyNote>(self);
    return PyImage_create(pNoteObj->m_book, self, index);
}
/**
 *  substituteImages
 *     Notes are written as markdown text with explicit image inclusions.
 *     The images are stored in the database indexed to the link that created
 *     them.  If the markdown needs rendering, this code will
 *     export images and modify the raw text to refer to the exported images
 *     rather than the original image files.
 *
 *  @param  self  - pointer to note object storage.
 *  @param  args  - unsed positional parameters.
 *  @return PyObject* unicode modified note text.
 */
static PyObject*
PyNote_substituteImages(PyObject* self, PyObject* args)
{
    auto pNote = PyNote_getNote(self);
    return PyUnicode_FromString(pNote->substituteImages().c_str());
}
/////////////////////////////////////////////////////////////////////
// Tables for LogBook.Note types.

// attribute accessors.

static PyGetSetDef note_accessors[] = {
   {"id", PyNote_getid, nullptr, "Note primarykey", nullptr},
   {"run", PyNote_getRun, nullptr, "Associated run", nullptr},
   {"time", PyNote_getTime, nullptr, "Note unix timestamp", nullptr},
   {"contents", PyNote_getContents, nullptr, "Raw note text", nullptr},
   {"author", PyNote_getAuthor, nullptr, "Note author", nullptr},
   
    
    {nullptr, nullptr, nullptr, nullptr, nullptr}
};

// attribute methods:

static PyMethodDef note_methods[] = {
  {"image_count", PyNote_imageCount, METH_NOARGS, "Number of images in note"},
  {"get_image", PyNote_image, METH_VARARGS, "Get specific image object"},
  {
    "substitute_images", PyNote_substituteImages, METH_NOARGS,
    "Export and substitue image filenames"},
  
  {nullptr, nullptr, 0, nullptr}  
};


PyTypeObject PyNoteType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    /* Not supported in e.g. g++-4.9.2
    .tp_name = "LogBook.Note",
    .tp_basicsize = sizeof(PyNote),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)(PyNote_dealloc),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc  = "LogBook Note class - user notation",
    .tp_methods = note_methods,
    .tp_getset  = note_accessors,
    .tp_init = (initproc)PyNote_init,
    .tp_new  = PyNote_new  
    */
};

void PyNote_InitType()
{
  PyImage_InitType();
  PyNoteType.tp_name = "LogBook.Note";
  PyNoteType.tp_basicsize = sizeof(PyNote);
  PyNoteType.tp_itemsize = 0;
  PyNoteType.tp_dealloc = (destructor)(PyNote_dealloc);
  PyNoteType.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
  PyNoteType.tp_doc  = "LogBook Note class - user notation";
  PyNoteType.tp_methods = note_methods;
  PyNoteType.tp_getset  = note_accessors;
  PyNoteType.tp_init = (initproc)PyNote_init;
  PyNoteType.tp_new  = PyNote_new;
}
//////////////////////////////////////////////////////////////////
// Functions exported outside this module.

/**
 * PyImage_isImage
 *    @param pObject - ostensibly an image type.
 *    @return bool   - true if pObject is a Notebook.Image.
 */
bool
PyImage_isImage(PyObject* pObject)
{
    return PyObject_IsInstance(
        pObject, reinterpret_cast<PyObject*>(&PyNoteImageType)
    );
}
/**
 * PyImage_getImage
 *    Given that pObject is a note image, returns the
 *    underlying image object.
 *  @param pObject - an image object. It's the caller's responsibility
 *         to ensure that's the case else at best we'll return
 *         bullshit.
 *  @return LogBookNote::NoteIMage*  - Pointer to the underlying image
 *        object.
 */
const LogBookNote::NoteImage*
PyImage_getImage(PyObject* pObject)
{
    pPyNoteImage pImage = reinterpret_cast<pPyNoteImage>(pObject);
    pPyNote pNote       = reinterpret_cast<pPyNote>(pImage->m_note);
    return &(*pNote->m_pNote)[pImage->m_imageIndex];
}
/**
 * PyImage_create
 *     Create a new PyObject* that is a NoteBook.Image object.
 *  @param book - the logbook.
 *  @param note - the note the image is in.
 *  @param index - the index of the image in the note.
 *  @return PyObject* - Resulting new object.
 */
PyObject*
PyImage_create(PyObject* book, PyObject* note, int index)
{
    PyObject* pResult = PyObject_CallFunction(
        reinterpret_cast<PyObject*>(&PyNoteImageType),
        "OOI", book, note, index
    );
    Py_XINCREF(pResult);
    return pResult;
}
/**
 * PyNote_isNote
 *    @param pObject - an object that may or may  not be a LogBook.Note
 *    @return bool   - true if pObject is, in fact a LogBook.Note.
 */
bool
PyNote_isNote(PyObject* pObject)
{
    return PyObject_IsInstance(
        pObject, reinterpret_cast<PyObject*>(&PyNoteType)
    );
}
/**
 * PyNote_GetNote
 *    Returns the logBookNote object encapsulated by an object
 *    that's known to be a LogBook.Note.
 * @param pObject - pointer to an object that's know to be
 *                  a LogBook.Note.
 * @return LogBookNote*  - pointer to the underying note. This is
 *        only valid for the remaining lifetime of pObject.
 */
LogBookNote*
PyNote_getNote(PyObject* pObject)
{
    pPyNote pNote = reinterpret_cast<pPyNote>(pObject);
    return pNote->m_pNote;
}
/**
 * PyNote_create
 *    Create a new LogBook.Note object.
 * @param LogBook the Python LogBook object in which the note is
 *                being created.
 * @param pNote - a LogBookNote object pointer we want to encapsulate
 *            in a LogBook.Note python object.  Note that a new
 *            note object is created that also represents the
 *            note in pNote so it's not necessary to maintain
 *            the object pointed to by pNote after we return.
 *  @return PyObject* pointer to a new LogBook.Note.
 *  
 */

PyObject*
PyNote_create(PyObject* logbook, LogBookNote* pNote)
{
    PyObject* result = PyObject_CallFunction(
        reinterpret_cast<PyObject*>(&PyNoteType),
        "OI", logbook, pNote->getNoteText().s_id
    );
    Py_XINCREF(result);
    return result;
}
