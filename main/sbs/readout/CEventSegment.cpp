/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include <config.h>
#include "CEventSegment.h"
#include <CExperiment.h>
#include <CReadoutMain.h>

/*!
   Concrete classes are expected to override this
   method by providing code that does one-time star to data taking
   initialization of the data taking modules read by this segment.
*/
void
CEventSegment::initialize()
{
}

/*!
   concrete classes are expected to override this method by
   providing code that clears pending data latched in the module.
   Note that this is called after reading each event, and is intended
   to be used for modules for which readout is non-destructive.
*/
void
CEventSegment::clear()
{
}
/*!
   Concreate sub classes are expected to override this method and
   provide code that disables the module as data taking is turned off.
*/
void
CEventSegment::disable()
{
}
//
// The C++ standards provides for the implementation of abstract members.
// we use this to produce a doxygen description of the expectations
// for this member.
/*!
   Concrete subclasses are expecdted to read out the hardware in response to
   an event ttrigger inthis function.
   \param pBuffer - points to ordinary storage into which the method should put
                    its data.
   \parm maxwords - Is the maximum number of uint16_t's that can be read into
                    pBuffer without risking an overrun.
 
   \return size_t
   \retval The number of 16 (uint16_t e.g.) values inserted in pBuffer.

   \throw  CRangeError  if the function detects that it must insert more than maxwords
           into pBuffer.
*/
size_t
CEventSegment::read(void* pBuffer, size_t maxwords)
{
  return 0;
}

/**
 * onBegin
 *   Invoked just prior to starting data taking at a begin run.  At
 *   the point when called, the trigger thread is not yet started.
 *
 */
void 
CEventSegment::onBegin()
{}

/**
 * onEnd
 *   Invoked just prior to emitting the end of run event.
 *   At this time the trigger thread has stopped (or at least
 *   won't be processing any more triggers)  but the end run event has not
 *   yet been emitted.
 */
void
CEventSegment::onEnd()
{}
/**
 * onPause
 *   Invoked just prior to emitting  a pause event. At this time
 *   the trigger thread has stopped (or at least won't process any
 *   more triggers).
 */
void
CEventSegment::onPause()
{
}

/**
 * onResume
 *    Invoked just after emitting a resume event. This is done before
 *    any triggers are processed.
 */
void
CEventSegment::onResume()
{
}

/*!
   Event segments are not composites:
*/
bool
CEventSegment::isComposite() const
{
  return false;
}
/*----------------------------------------------------
 * non virtual publics:
 */


/**
 * reject
 * 
 * Called to indicate that after all event segments have been read
 * the event should not be kept.
 *
 */
void 
CEventSegment::reject()
{
  m_accept = Reject;
}
/**
 * rejectImmediately
 *
 *  Called to indicate that after _this_ event segment returns,
 *  all event processing will end and the event will be rejected.
 *
 */
void
CEventSegment::rejectImmediately()
{
  m_accept = RejectImmediately;
}
/**
 * keep
 *
 *  Called to indicate the event segment wants to keep the event
 *  Normally you don't need to call this unless your event segment
 *  has changed its mind about what to do because
 *  CExperiment will call this prior to running your event segment.
 */
void
CEventSegment::keep()
{
  m_accept = Keep;
}
/**
 * getAcceptState
 *
 *   Returns the current value of the acceptance state.
 *
 * @return CEventSegment::AcceptState
 * @retval keep - keep the event
 * @retval reject - reject the event but process the rest of the event segments.
 * @retval rejecteimmediatly - reject the event and don't process the rest of the
 *                             event segments.
 */
CEventSegment::AcceptState
CEventSegment::getAcceptState() const
{
  return m_accept;
}
/**
 * setTimestamp
 *
 *   Set the event timestamp for the event.
 *   If called multiple times, the last one rules.
 *
 *   @param stamp - the timestamp.
 */
void
CEventSegment::setTimestamp(uint64_t stamp)
{
    CExperiment* p = CReadoutMain::getExperiment();
    p->setTimestamp(stamp);
}
/**
 * setSourceId
 *
 *  Sets the event source id.  If called multiple times, the last one rules.
 *  @param id
 */
void
CEventSegment::setSourceId(uint32_t id)
{
    CExperiment* p = CReadoutMain::getExperiment();
    p->setSourceId(id);
}
