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

/** @file:  PyLogBookRun.cpp
 *  @brief: Implement the Python type that encapsulates LogBookRun.
 */
#include "PyLogBookRun.h"
#include "PyLogbook.h"
#include "PyLogBookShift.h"
#include "LogBook.h"
#include "LogBookPerson.h"
#include "LogBookShift.h"
#include "LogBookRun.h"


// Support python 3 date/time.

#include <datetime.h>
///////////////////////////////////////////////////////////////////
// Exported utilities:

/**
 * PyRun_isRun
 *    @param pObject - some object to test.
 *    @return bool  - true if pObject is a LogBook.Run
 */
bool
PyRun_isRun(PyObject* pObject)
{
    return PyObject_IsInstance(
        pObject, reinterpret_cast<PyObject*>(&PyLogBookRunType)
    );
}
/**
 * PyRun_getRun
 *    Return the LogBookRun* for a run python object.
 * @param pObject - the run object.
 * @return LogBookRun*
 */
LogBookRun*
PyRun_getRun(PyObject* pObject)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(pObject);
    return pThis->m_pRun;
}
/**
 * PyRun_newRun
 *    Make a run python object from a logbook run and the logbook that
 *    python object that spawned it:
 * @param logBook - LogBook object the run is in.
 * @param pRun    - LogBook* to encapsulate.
 * @return PyObject* - resulting LogBook.Run object.
 */
PyObject*
PyRun_newRun(PyObject* logbook, LogBookRun* pRun)
{
    int number = pRun->getRunInfo().s_number;
    PyObject* result = PyObject_CallFunction(
        reinterpret_cast<PyObject*>(&PyLogBookRunType),
        "Oi", logbook, number
    );
    Py_XINCREF(result);
    return result;
}
/**
 * PyRun_TupleFromVector
 *    Returns a tuple of run objects from a vector of LogBookRun
 *    objects
 * @param        logbook - the logbook containing the runs.
 * @param        runs    - vector of runs.
 * @return The tuple, nullptr on error.
 */
PyObject*
PyRun_TupleFromVector(
    PyObject* logbook,
    const std::vector<LogBookRun*>& runs
)
{
    PyObject* result = PyTuple_New(runs.size());
    if (!result) return nullptr;
    
    for (int i = 0; i < runs.size(); i++) {
        PyObject* pRun = PyRun_newRun(logbook, runs[i]);
        if (!pRun) {
            freeTuple(result);
            return nullptr;
        }
        PyTuple_SET_ITEM(result, i, pRun);
    }
    
    return result;
}


/**
 *  The Transition type only stands in the presence of a run object.
 *  It's used to provide information about each transition.
 */
// Canonicals for transitions:

/**
 * PyTransition_new
 *    Allocator
 *  @param type - pointer to the type object
 *  @param args, kwargs - ignored parameters
 *  @return PyObject* new object but not yet filled  in (see PyTransition_init)
 */
static PyObject*
PyTransition_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    pPyRunTransition self = reinterpret_cast<pPyRunTransition>(type->tp_alloc(type, 0));
    if (self) {
        self->m_book = nullptr;
        self->m_pRun = nullptr;
        self->m_nIndex = -1;           // Illegal
    }
    return reinterpret_cast<PyObject*>(self);
}
/**
 * PyTransition_init
 *   Initialize transition storage. We're passed the following
 *   parameters in order:
 *   -  'logbook'
 *   -  'run'  - PyRun object.
 *   -  'index' - Transition number (Must be valid)
 *  @param self - pointer to the object to initialize.
 *  @param args - Positional arguments.
 *  @param kwargs - Keyword arguments.
 *  @return int 0 for success, -1 on failure.
 */
static int
PyTransition_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    PyObject* pRun(nullptr);
    PyObject* pBook(nullptr);
    int       index;
    const char* keywords[] = {"logbook", "run", "index", nullptr};
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|OOI", const_cast<char**>(keywords),
        &pBook, &pRun, &index
    )) {
        return -1;
    }
    // Must have all objects:
    
    if (!(pRun && pBook)) {
        PyErr_SetString(
            logbookExceptionObject,
            "Missing either a required logbook or run object constructing a transition"
        );
        return -1;
    }
    
    // Must have a logbook:
    
    if (!PyLogBook_isLogBook(pBook)) {
        PyErr_SetString(
            logbookExceptionObject,
            "The object that's supposed to be a logbook isn't when constructing a transition object"
        );
        return -1;
    }
    
    // Must have a logbook run and the index must be valid:
    
    if (!PyRun_isRun(pRun)) {
        PyErr_SetString(
            logbookExceptionObject,
            "Need a run object when constructing a transition"
        );
        return -1;
    }
    pPyRunTransition pThis = reinterpret_cast<pPyRunTransition>(self);
    if ((index < 0) || (index > PyRun_getRun(pRun)->numTransitions())) {
        PyErr_SetString(
            logbookExceptionObject,
            "Transition index out of range in transition constructor"
        );
        return -1;
    }
    Py_INCREF(pBook);
    Py_INCREF(pRun);              // Hold a reference to the run .
    pThis->m_book = pBook;
    pThis->m_pRun = pRun;
    pThis->m_nIndex = index;
    return 0;
}
/**
 * PyTransition_dealloc
 *    Free transition storage.
 * @param self - pointer the transition.
 */
static void
PyTransition_dealloc(PyObject* self)
{
    pPyRunTransition pThis = reinterpret_cast<pPyRunTransition>(self);
    Py_XDECREF(pThis->m_book);
    Py_XDECREF(pThis->m_pRun);
    Py_TYPE(self)->tp_free(self);
}

/////////  Transition getters.

/**
 * get_transitionid
 *    Get the primary key of this transition.
 *
 *  @param self - pointer to the RunTranstion object data.
 *  @param closure - unused void pointer.
 *  @return PyObject* integer id.
 * 
 */
static PyObject*
get_transitionid(PyObject* self, void* closure)
{
    pPyRunTransition pThis = reinterpret_cast<pPyRunTransition>(self);
    LogBookRun* pRun = PyRun_getRun(pThis->m_pRun);
    int id = (*pRun)[pThis->m_nIndex].s_id;
    return PyLong_FromLong(id);
}
/**
 * get_transition
 *    Get the transition code.  This is an integer that is the
 *    primary key of the transition name in the valid_transitions
 *    table. Normally,  you'll really want the transition name.
 *  @param self - pointer to the RunTranstion object data.
 *  @param closure - unused void pointer.
 *  @return PyObject* integer id.
 * 
 */
static PyObject*
get_transition(PyObject* self, void* closure)
{
    pPyRunTransition pThis = reinterpret_cast<pPyRunTransition>(self);
    LogBookRun* pRun = PyRun_getRun(pThis->m_pRun);
    int id = (*pRun)[pThis->m_nIndex].s_transition;
    return PyLong_FromLong(id);
}
/**
 * get_transitionname
 *    Returns the textual name of the transition this represents
 *    for example 'BEGIN'.
 *  @param self - pointer to the RunTranstion object data.
 *  @param closure - unused void pointer.
 *  @return PyObject* Unicode string.
 * 
 */
static PyObject*
get_transitionname(PyObject* self, void* closure)
{
    pPyRunTransition pThis = reinterpret_cast<pPyRunTransition>(self);
    auto pRun = PyRun_getRun(pThis->m_pRun);
    auto transition = (*pRun)[pThis->m_nIndex].s_transitionName;
    return PyUnicode_FromString(transition.c_str());
}

/**
 * get_transitiontime
 *   Returns a date-time object that gets  the time the transition
 *   occured.  Note the resulting time is really not aware of the timezone.
 * 
 *  @param self - pointer to the RunTranstion object data.
 *  @param closure - unused void pointer.
 *  @return PyObject* integer timestamp that can be passed to a
 *               datetime.fromtimestamp to get a date time.
 * 
 */
static PyObject*
get_transitiontime(PyObject* self, void * closure)
{
    pPyRunTransition pThis = reinterpret_cast<pPyRunTransition>(self);
    LogBookRun* pRun = PyRun_getRun(pThis->m_pRun);
    int stamp = (*pRun)[pThis->m_nIndex].s_transitionTime;
    
    // Make the date-time value and return it:
        
    
    return PyLong_FromLong(stamp);
}
/**
 * get_transitioncomment
 *   Return the comment associated with a run's state transition.
 *  @param self - pointer to the RunTranstion object data.
 *  @param closure - unused void pointer.
 *  @return PyObject* Unicode comment.
 * 
 */
static PyObject*
get_transitioncomment(PyObject* self, void* closure)
{
    pPyRunTransition pThis = reinterpret_cast<pPyRunTransition>(self);
    auto pRun = PyRun_getRun(pThis->m_pRun);
    auto comment = (*pRun)[pThis->m_nIndex].s_transitionComment;
    return PyUnicode_FromString(comment.c_str());
}
/**
 * get_transitionshift
 *    Return the shift object from the transition.
 *  @param self - pointer to the RunTranstion object data.
 *  @param closure - unused void pointer.
 *  @return PyObject* LogBook.Shift object.
 * 
 */
static PyObject*
get_transitionshift(PyObject* self, void* closure)
{
    pPyRunTransition pThis = reinterpret_cast<pPyRunTransition>(self);
    auto pRun = PyRun_getRun(pThis->m_pRun);
    LogBookShift* pShift = (*pRun)[pThis->m_nIndex].s_onDuty;
    return PyShift_newShift(pThis->m_book, pShift);
}



/////////// Transition tables:

//  Getters:
static PyGetSetDef PyTransitionGetters[] = {
    //name  get     set        docstring                    closure
    {"id", get_transitionid, nullptr, "Get transition primary key", nullptr},
    {"transition", get_transition, nullptr, "Get transition type id", nullptr},
    {"transition_name", get_transitionname, nullptr, "Get textual transition type"},
    {"time", get_transitiontime, nullptr, "Get transition timestamp", nullptr},
    {"comment", get_transitioncomment, nullptr, "Get transition comment", nullptr},
    {"shift", get_transitionshift, nullptr, "Get shift on duty for transition"},
    
    
    // End sentinel:
    
    {NULL, NULL, NULL, NULL, NULL}
};

   // Type struct:
   
PyTypeObject PyRunTransitionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    /* Can't do this in g++ 4.9.2 e.g.
    .tp_name = "LogBook.transition",
    .tp_basicsize = sizeof(PyRunTransition),
    .tp_itemsize = 0,
    .tp_dealloc  = PyTransition_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc   = "Run state transition",
    .tp_methods = nullptr,
    .tp_getset = PyTransitionGetters,
    .tp_init   = (initproc)PyTransition_init,
    .tp_new    = PyTransition_new
    */
};
static void PyTransitionType_Init()
{
  PyRunTransitionType.tp_name      = "LogBook.transition";
  PyRunTransitionType.tp_basicsize = sizeof(PyRunTransition);
  PyRunTransitionType.tp_itemsize  = 0;
  PyRunTransitionType.tp_dealloc   = PyTransition_dealloc;
  PyRunTransitionType.tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
  PyRunTransitionType.tp_doc       = "Run state transition";
  PyRunTransitionType.tp_methods   = nullptr;
  PyRunTransitionType.tp_getset    = PyTransitionGetters;
  PyRunTransitionType.tp_init      = (initproc)PyTransition_init;
  PyRunTransitionType.tp_new       = PyTransition_new;
							  
}

/////////////////////////////////////////////////////////////////////////////
// LogBook.Run canonicals:

/**
 * PyRun_new
 *    Allocate storage for a new LogBook.Run object.
 *
 *  @param type - pointer to type object.
 *  @param args, kwargs - positional and keyword parameters we don't care about
 *  @return PyObject* allocated strorage.nullptr if failed with an exception
 *                raised.
 */
static PyObject*
PyRun_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
        // Allocate the storage and fill in the data pointers with
        // nullptr so Py_XDECREF and delete won't choke.
        
        pPyLogBookRun self = reinterpret_cast<pPyLogBookRun>(type->tp_alloc(type, 0));
        if (self) {
            self->m_book = nullptr;
            self->m_pRun = nullptr;
        }
        return reinterpret_cast<PyObject*>(self);
}
/**
 * PyRun_init
 *    Initialize storage of a PyLogBookRun object given the constructor
 *    parameters.
 * @param self   - pointer to the PyLogBookRun storage.
 * @param args   - Positional parameters.
 * @param kwargs - Keyword arguments.
 *   Parmaters are, order 'logbook', 'run' the logbook object and the run number
 *   of the run respectively.
 * @return int  0 for success, and -1 for failure with an exception raised.
 */
static int
PyRun_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    PyObject* pBook;
    int       run;
    const char* keywords[] = {"logbook", "run", nullptr};
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|Oi", const_cast<char**>(keywords),
        &pBook, &run
    )) {
        return -1;    
    }
    // pBook must be a logbook:
    
    if (!PyLogBook_isLogBook(pBook)) {
        PyErr_SetString(
            logbookExceptionObject,
            "Construction logbook object was not a logbook"
        );
        return -1;
    }
    // Construct the underlying LogBookRun object:
    // and fill in the storage.
    
    LogBookRun* pRun(nullptr);
    LogBook*    book = PyLogBook_getLogBook(pBook);
    try {
        pRun = book->findRun(run);
        
        // Success:
        
        Py_INCREF(pBook);
        pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
        pThis->m_book = pBook;
        pThis->m_pRun = pRun;
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        return -1;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "An unanticipated exception type was caught in PyRun_init"
        );
        return -1;
    }
    return 0;                  // Success   
}
/**
 * PyRun_dealloc
 *   Destructor for run objects.
 *
 * @param self - pointer to PyLogBookRun storage.
 */
static void
PyRun_dealloc(PyObject* self)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    Py_XDECREF(pThis->m_book);
    delete pThis->m_pRun;
    Py_TYPE(self)->tp_free(self);
}


/////////////////////////////////////////////////////////////
// Log book Run attribute getters.

/**
 * get_id
 *    Get the primary key of the run (in the run table).
 * @param self - pointer to PyLogBookRun struct.
 * @param closure - unused additional data pointer.
 * @return PyObject* - integer primary key.
 */
static PyObject*
get_runid(PyObject* self, void* closure)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    return PyLong_FromLong(pThis->m_pRun->getRunInfo().s_id);
}


/**
 * get_number
 *     Get the run number.
 * @param self pointer to our PyLogBookRun struct.
 * @param closure - unused.
 * @return PyObject* (integer)
 */
static PyObject*
get_runnumber(PyObject* self, void* closure)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    int run = pThis->m_pRun->getRunInfo().s_number;
    return PyLong_FromLong(run);
}
/**
 *  get_title
 *     Get the run title string
 *  @param self - pointer to the PyLogBookRun struct for this object.
 *  @param closure (unused)]
 *  @return PyObject* unicode title string.
 */
static PyObject*
get_runtitle(PyObject* self, void* closure)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    std::string title = pThis->m_pRun->getRunInfo().s_title;
    return PyUnicode_FromString(title.c_str());
}


/////////////////////////////////////////////////////////////
// Log Book Run instance methods.
//    Note - future versions may support the Python sequence
//           protocol to fish transition objects.\\\


/**
 *   numTransitions
 *      Return the number of transitions (could become len(run if
 *      sequence protocol is implemented).
 *  @param PyObject* self - pointer to the storage associated with this
 *                          object.
 *  @param unused - args which don't exist.
 *  @return PyObject* (integer)
 */
static PyObject*
numTransitions(PyObject* self, PyObject* unused)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    int n = pThis->m_pRun->numTransitions();
    return PyLong_FromLong(n);
}
/**
 *  getTransition
 *     Indexing to return a specific transition object.  This could
 *     be replaced in the future by indexing the run as a sequence
 *     of transitions suporting the sequence protocol
 * @param self - pointer to the storage associated with this object.
 * @param args - contains an integer offset.
 * @return PyObject* - Transition objecst on success else raises
 *                      LogBook.error exception.
 */
static PyObject*
getTransition(PyObject* self, PyObject* args)
{
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        return nullptr;
    }
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    PyObject* result;
    
    result = PyObject_CallFunction(
        reinterpret_cast<PyObject*>(&PyRunTransitionType),
        "OOi", pThis->m_book, self, index
    );
    Py_XINCREF(result);
    return result;
    
}
/**
 *  isCurrent
 *    Determine if the run is the current run.
 *
 * @param self - pointer to object storage.
 * @param unused - Args are not used.
 * @return PyObject* bool
 */
static PyObject*
isCurrent(PyObject* self, PyObject* unused)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    auto pBook = PyLogBook_getLogBook(pThis->m_book);
    auto currentRun = pBook->currentRun();
    
    // This run is curent if it's  id matches the returned one:
    
    bool bResult =
        currentRun && (currentRun->getRunInfo().s_id == pThis->m_pRun->getRunInfo().s_id);
    delete currentRun;
    
    if (bResult) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}
/**
 *  lastTransitionType
 *     Return the code of the last transition.
 *  @param self - pointer to our storage.
 *  @param unused - unused parameter.
 *  @return PyObject* integer, see last Transition below.
 */
static PyObject*
lastTransitionType(PyObject* self, PyObject* unused)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    return PyLong_FromLong(pThis->m_pRun->lastTransitionType());
}

/**
 *  lastTransition
 *     Return the string that describes the last transition.
 * @param self
 * @param unused
 * @return PyObject* Unicode object.
 */
static PyObject*
lastTransition(PyObject* self, PyObject* unused)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    return PyUnicode_FromString(pThis->m_pRun->lastTransition());
}
/**
 * isActive
 *    Determines if the run is active (that is not ended.)
 * @param self
 * @param unused
 * @return PyObject* boolean.
 */
static PyObject*
isActive(PyObject* self, PyObject* unused)
{
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    if (pThis->m_pRun->isActive()) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}
/**
 * end
 *    End the run.
 * @param self  - pointer to our object storage.
 * @param args  - One optional parameter - the end of run comment.
 * @return PyObject* self - to support chaining.
 * @note the underlying run can be changed by this.
 */
static PyObject*
end(PyObject* self, PyObject* args)
{
    const char* comment(nullptr);
    if (!PyArg_ParseTuple(args, "|s", &comment)) {
        return nullptr;
    }
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    auto pBook = PyLogBook_getLogBook(pThis->m_book);
    
    try {
        pBook->end(pThis->m_pRun, comment);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        return nullptr;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "An unanticipated Exception was caught in LogBook.run.end()"
        );
        return nullptr;
    }
    Py_XINCREF(self);
    return reinterpret_cast<PyObject*>(self);
}
/**
 * pause
 *    Pause this run. ... see end above for usage.
 */
static PyObject*
pause(PyObject* self, PyObject* args)
{
    const char* comment(nullptr);
    if (!PyArg_ParseTuple(args, "|s", &comment)) {
        return nullptr;
    }
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    auto pBook = PyLogBook_getLogBook(pThis->m_book);
    try {
        pBook->pause(pThis->m_pRun, comment);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        return nullptr;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "An unanticipated Exception was caught in LogBook.run.pause()"
        );
        return nullptr;
    }
    Py_XINCREF(self);
    return reinterpret_cast<PyObject*>(self);
}
/**
 * resume
 */
static PyObject*
resume(PyObject* self, PyObject* args)
{
    const char* comment(nullptr);
    if (!PyArg_ParseTuple(args, "|s", &comment)) {
        return nullptr;
    }
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    auto pBook = PyLogBook_getLogBook(pThis->m_book);
    try {
        pBook->resume(pThis->m_pRun, comment);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        return nullptr;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "An unanticipated Exception was caught in LogBook.run.resume()"
        );
        return nullptr;
    }
    Py_XINCREF(self);
    return reinterpret_cast<PyObject*>(self);
}
/**
 * emergencyEnd
 */
static PyObject*
emergencyEnd(PyObject* self, PyObject* args)
{
    const char* comment(nullptr);
    if (!PyArg_ParseTuple(args, "|s", &comment)) {
        return nullptr;
    }
    pPyLogBookRun pThis = reinterpret_cast<pPyLogBookRun>(self);
    auto pBook = PyLogBook_getLogBook(pThis->m_book);
    try {
        pBook->emergencyStop(pThis->m_pRun, comment);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        return nullptr;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "An unanticipated Exception was caught in LogBook.run.emerbency_end()"
        );
        return nullptr;
    }
    Py_XINCREF(self);
    return reinterpret_cast<PyObject*>(self);
}




///////////////////////////////////////////////////////////
// Tables for logBook.Run type:

// Setters and getters:

static PyGetSetDef PyRunGetters[] = {
    
    {"id", get_runid, nullptr, "Run primary database key", nullptr},
    {"number", get_runnumber, nullptr, "Run Number", nullptr},
    {"title", get_runtitle, nullptr, "Run title", nullptr},
    
    // end setinell
    // name     get     set      doc      closure
    {nullptr, nullptr, nullptr, nullptr, nullptr}
};
// Methods;

static PyMethodDef PyRun_methods [] = {
    {
        "transition_count", numTransitions, METH_NOARGS,
        "Number of state transitions"
    },
    {"get_transition", getTransition, METH_VARARGS, "Get a transition"},
    {"is_current", isCurrent, METH_NOARGS, "Test if run is current"},
    {"last_transitionid", lastTransitionType, METH_NOARGS, "Get last transition code"},
    {"last_transition", lastTransition, METH_NOARGS, "Get last transition textually"},
    {"is_active", isActive, METH_NOARGS, "Is this run active?"},
    {"end", end, METH_VARARGS, "End this run nomrally"},
    {"pause", pause, METH_VARARGS, "Pause this run"},
    {"resume", resume, METH_VARARGS, "Resume this run"},
    {"emergency_end", emergencyEnd, METH_VARARGS, "Emergency end to this run"},
    
    
    // End sentinell    
    // name function, flags docstring
    {NULL, NULL,  0,  NULL}    
};

//  Type definition table:

PyTypeObject PyLogBookRunType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    /* Not allowed in g++ 4.9.2 e.g.
    .tp_name = "LogBook.Run",
    .tp_basicsize = sizeof(PyLogBookRun),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)(PyRun_dealloc),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc  = "LogBook Run class; data taking run and its transitions",
    .tp_methods = PyRun_methods,
    .tp_getset  = PyRunGetters,
    .tp_init = (initproc)PyRun_init,
    .tp_new  = PyRun_new    
    */
};
void PyRun_InitType()
{
  PyTransitionType_Init();
  PyLogBookRunType.tp_name = "LogBook.Run";
  PyLogBookRunType.tp_basicsize = sizeof(PyLogBookRun);
  PyLogBookRunType.tp_itemsize = 0;
  PyLogBookRunType.tp_dealloc = (destructor)(PyRun_dealloc);
  PyLogBookRunType.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
  PyLogBookRunType.tp_doc  = "LogBook Run class; data taking run and its transitions";
  PyLogBookRunType.tp_methods = PyRun_methods;
  PyLogBookRunType.tp_getset  = PyRunGetters;
  PyLogBookRunType.tp_init = (initproc)PyRun_init;
  PyLogBookRunType.tp_new  = PyRun_new;
}
