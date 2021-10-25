/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CTclRingCommand.cpp
# @brief  Implement the ring Tcl command.
# @author <fox@nscl.msu.edu>
*/

#include "CTclRingCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CRingBuffer.h>
#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CDataFormatItem.h>
#include <CRingFragmentItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CGlomParameters.h>

#include <CRemoteAccess.h>
#include <CAllButPredicate.h>
#include <CDesiredTypesPredicate.h>
#include <CRingItemFactory.h>
#include <CAbnormalEndItem.h>
#include <CTimeout.h>

#include <tcl.h>

#include <limits>
#include <chrono>
#include <thread>
#include <iostream>

using namespace std;

/**
 * construction
 *   @param interp - the interpreter on which we are registering the 'ring'
 *                   command.
 */
CTclRingCommand::CTclRingCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "ring", true) {}
    
/**
 * destruction:
 *    Kill off all the CRingItems in the m_attachedRings map.
 */
CTclRingCommand::~CTclRingCommand()
{
    while(! m_attachedRings.empty()) {
        CRingBuffer* pRing = (m_attachedRings.begin())->second;    // First item.
        delete pRing;
        m_attachedRings.erase(m_attachedRings.begin());
    }
}

/**
 * operator()
 *   Gains control when the command is executed.
 *
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @return int
 *   @retval TCL_OK - Normal return, the command was successful..
 *   @retval TCL_ERROR - Command completed in error.
 */
int
CTclRingCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // Put everything in a try/catch block. Errors throw strings that get put
    // in result:
    
    try {
        bindAll(interp, objv);
        requireAtLeast(objv, 2, "Insufficient parameters");
        
        std::string subcommand = objv[1];
        if(subcommand == "attach") {
            attach(interp, objv);
            
        } else if (subcommand == "detach") {
            detach(interp, objv);
        } else if (subcommand == "get") {
            get(interp, objv);
        } else {
            throw std::string("bad subcommand");
        }
        
    } catch(std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*----------------------------------------------------------------------------
 * Protected members (subcommand handlers)
 */

/**
 * attach
 *    Execute the ring attach command which attaches to a ring.
 *    *  Ensure there's a URI parameter.
 *    *  Connect to the ring creating a CRingBuffer object.
 *    *  Put the ring buffer object in the m_attachedRings map indexed by the
 *    *  ring URI.
 *
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @throw std::string error message to put in result string if TCL_ERROR
 *          should be returned from operator().
 */
void
CTclRingCommand::attach(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // need a URI:
    
    requireExactly(objv, 3, "ring attach needs a ring URI");
    std::string uri = objv[2];
    CRingBuffer* pRing(0);
    try {
        if (m_attachedRings.find(uri) != m_attachedRings.end()) {
            throw std::string("ring already attached");
        }
        pRing = CRingAccess::daqConsumeFrom(uri);
        m_attachedRings[uri] = pRing;
    }
    catch(std::string) {
        throw;
    }
    catch (...) {
        throw std::string("Failed to attach ring");
    }
}

/**
 * detach
 *    Execute the ring detach command:
 *    *  Ensure there's a URI parameter
 *    *  Use it to look up the ring in the m_attachedRings map (error if no match).
 *    *  delete the ring buffer object -- which disconnects from the ring.
 *    *  remove the map entry.
 *
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @throw std::string error message to put in result string if TCL_ERROR
 *          should be returned from operator().
 */
void
CTclRingCommand::detach(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "ring detach needs a URI");
    
    std::string uri = objv[2];
    std::map<std::string, CRingBuffer*>::iterator p = m_attachedRings.find(uri);
    if (p == m_attachedRings.end()) {
        throw std::string("ring is not attached");
    }
    CRingBuffer* pRing = p->second;
    m_attachedRings.erase(p);
    delete pRing;
}                             

/**
 * get
 *   Execute the ring get command (blocks until an item is available);
 *    * Ensure there's a ring URI parameter
 *    * Looks up the CRingBuffer in the map (error if no match).
 *    * Gets a CRingItem from the ring with the appropriate filter.
 *    * Produces a dict whose keys/contents will depend on the item type
 *      (which will always be in the -type key).  See the private formattting
 *      functions for more on what's in each dict.
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @throw std::string error message to put in result string if TCL_ERROR
 *          should be returned from operator().
 */
void
CTclRingCommand::get(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireAtLeast(objv, 3, "ring get needs a URI");
    requireAtMost(objv, 6, "Too many command parameters");

    CAllButPredicate all;
    CDesiredTypesPredicate some;
    CRingSelectionPredicate* pred;
    pred = &all;
    
    // If there's a 4th parameter it must be a list of item types to select
    // from

    unsigned long timeout = std::numeric_limits<unsigned long>::max();

    size_t paramIndexOffset = 0;
    if (std::string(objv[2]) == "-timeout") {
        if (objv.size() >= 4) {
            CTCLObject object = objv[3];
            timeout = int(object.lindex(0));
        } else {
            throw std::string("Insufficient number of parameters");
        }
        paramIndexOffset = 2;
    }

    std::string uri  = std::string(objv[2+paramIndexOffset]);
    std::map<std::string, CRingBuffer*>::iterator p =  m_attachedRings.find(uri);
    if (p == m_attachedRings.end()) {
        throw std::string("ring is not attached");
    }

    if (objv.size() == 4+paramIndexOffset) {
        CTCLObject types = objv[3+paramIndexOffset];
        for (int i = 0; i < types.llength(); i++) {
            int type = int(types.lindex(i));
            some.addDesiredType(type);
        }
        pred = &some;
    }
    
    // Get the item from the ring.
    
    
    CRingBuffer* pRing = p->second;
    auto pSpecificItem = getFromRing(*pRing, *pred, timeout);

    if (pSpecificItem == nullptr) {
        // oops... we timed out. return an empty string
        CTCLObject result;
        result.Bind(&interp);
        interp.setResult(result);
        return;
    }
    
    // Actual upcast depends on the type...and that describes how to format:
    
    
    switch(pSpecificItem->type()) {
        case BEGIN_RUN:
        case END_RUN:
        case PAUSE_RUN:
        case RESUME_RUN:
            formatStateChangeItem(interp, pSpecificItem);
            break;
        case PERIODIC_SCALERS:
            formatScalerItem(interp, pSpecificItem);
            break;
        case PACKET_TYPES:
        case MONITORED_VARIABLES:
            formatStringItem(interp, pSpecificItem);
            break;
        case RING_FORMAT:
            formatFormatItem(interp, pSpecificItem);
            break;
        case PHYSICS_EVENT:
            formatEvent(interp, pSpecificItem);
            break;
        case EVB_FRAGMENT:
        case EVB_UNKNOWN_PAYLOAD:
            formatFragment(interp, pSpecificItem);
            break;
        case PHYSICS_EVENT_COUNT:
            formatTriggerCount(interp, pSpecificItem);
            break;
        case EVB_GLOM_INFO:
            formatGlomParams(interp,  pSpecificItem);
            break;
        case ABNORMAL_ENDRUN:
            formatAbnormalEnd(interp, pSpecificItem);
        default:
            break;;
            // TO DO:
    }
    
    
    delete pSpecificItem;
}

/*-----------------------------------------------------------------------------
 * Private utilities
 */


/**
 *  formatBodyHeader
 *  
 * Given that the object has a body header, this function creates the body header
 * dict for it
 *
 * @param interp - The interpreter to use when building the dict.
 * @parma p      - Pointer to the item.
 * @return CTCLObject - Really a list but in a format that can shimmer to a dict.
 */
CTCLObject
CTclRingCommand::formatBodyHeader(CTCLInterpreter& interp, CRingItem* p)
{
    CTCLObject subDict;
    subDict.Bind(interp);
    Tcl_Obj* tstamp = Tcl_NewWideIntObj(p->getEventTimestamp());
    subDict += "timestamp";
    subDict += CTCLObject(tstamp);
    
    subDict += "source";
    subDict += static_cast<int>(p->getSourceId());
    
    subDict += "barrier";
    subDict += static_cast<int>(p->getBarrierType());
    
    return subDict;        
}

/**
 *  format a ring state change item.  We're going to use the trick that a dict
 *  has a list rep where even elements are keys and odd elements their values.
 *  Users of the dict will shimmer into its dict rep. at first access.
 *
 *  @param interp - the interpreter.
 *  @param pItem  - Actually a CRingStateChangeItem.
 *
 *  Result is set with a dict with the keys:
 *
 *      type       - ring item type (textual)
 *      run        - run number
 *      timeoffset - Time offset in seconds.
 *      realtime   - Time of day of the ring item (can use with [time format])
 *      title      - The run title.
 *      bodyheader - only if there is a body header in the item.  That in turn is a dict with
 *                   timestamp, source, and barrier keys.
 */
 
void
CTclRingCommand::formatStateChangeItem(CTCLInterpreter& interp, CRingItem* pItem)
{
    CRingStateChangeItem* p = reinterpret_cast<CRingStateChangeItem*>(pItem);
    CTCLObject result;
    result.Bind(interp);
    
    // type:
    
    result += "type";
    result += p->typeName();
    
    result += "run";
    result += static_cast<int>(p->getRunNumber());
    
    result += "timeoffset";
    result += static_cast<int>(p->getElapsedTime());
    
    result += "realtime";
    result += static_cast<int>(p->getTimestamp());
    
    result += "title";
    result += p->getTitle();
    
    
    if (p->hasBodyHeader()) {
        result += "bodyheader";
        result += formatBodyHeader(interp, p);        
    }
    
    interp.setResult(result);
    
    
}
/**
 * formatScalerItem
 *    Formats a scaler item.  This creates a list that can be shimmered into a
 *    dict with the keys:
 *    - type - going to be Scaler
 *    - start - When in the run the start was.
 *    - end   - When in the run the end of the period was.
 *    - realtime  Time emitted ([clock format] can take this)
 *    - divisor - What to divide start or end by to get seconds.
 *    - incremental - Bool true if this is incremental.
 *    - scalers     - List of scaler values.
 *
 *    If there is a body header, the bodyheader key will be present and will be
 *    the body header dict.
 *
 *    @param interp - the intepreter object whose result will be set by this.
 *    @param pSpecificItem - Actually a CRingScalerItem pointer.
 */
void
CTclRingCommand::formatScalerItem(CTCLInterpreter& interp, CRingItem* pSpecificItem)
{
    CRingScalerItem* pItem = reinterpret_cast<CRingScalerItem*>(pSpecificItem);
    
    // Stuff outside the body header
   
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += pItem->typeName();
    
    result += "start";
    result += static_cast<int>(pItem->getStartTime());
    
    result += "end";
    result += static_cast<int>(pItem->getEndTime());
    
    result += "realtime";
    result += static_cast<int>(pItem->getTimestamp());
    
    result += "divisor";
    result += static_cast<int>(pItem->getTimeDivisor());
    
    result += "incremental";
    result += static_cast<int>(pItem->isIncremental() ? 1 : 0);
    
    // Now the scaler vector itself:
    
    std::vector<uint32_t> scalerVec = pItem->getScalers();
    CTCLObject scalerList;
    scalerList.Bind(interp);
    for (int i=0; i < scalerVec.size(); i++) {
        scalerList += static_cast<int>(scalerVec[i]);
    }
    result += "scalers";
    result += scalerList;
    
    
    // If there is a body header add it too.
    
    if (pItem->hasBodyHeader()) {
        result += "bodyheader";
        result += formatBodyHeader(interp, pItem);
    }
    
    
    interp.setResult(result);
}
/**
 * formatStringItem:
 *    Formats a ring item that contains a list of strings.   This produces a dict
 *    with the following keys:
 *    -   type - result of typeName.
*     -  timeoffset will have the offset time.
*     -  divisor will have the time divisor to get seconds.
*     -  realtime will have something that can be given to [clock format] to get
*        when this was emitted
*     -  strings - list of strings that are in the ring item.
*     -  bodyheader - if the item has a body header.
*     @param interp - the intepreter object whose result will be set by this.
*     @param pSpecificItem - Actually a CRingTextItem pointer.
*     */
void
CTclRingCommand::formatStringItem(CTCLInterpreter& interp, CRingItem* pSpecificItem)
{
    CRingTextItem* p = reinterpret_cast<CRingTextItem*>(pSpecificItem);
    
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += p->typeName();
    
    result += "timeoffset";
    result += static_cast<int>(p->getTimeOffset());
    
    result += "divisor";
    result += static_cast<int>(p->getTimeDivisor());
    
    result += "realtime";
    result += static_cast<int>(p->getTimestamp());
    
    CTCLObject stringList;
    stringList.Bind(interp);
    std::vector<std::string> strings = p->getStrings();
    for (int i =0; i < strings.size(); i++){
        stringList += strings[i];
    }
    result += "strings";
    result += stringList;
    
    if (p->hasBodyHeader()) {
        result += "bodyheader";
        result += formatBodyHeader(interp, p);
    }
    
    interp.setResult(result);
}
/**
 * formatFormatitem
 *
 *    Formats a ring format item.  This will have the dict keys:
 *    *  type - what comes back from typeName()
 *    *  major - Major version number
 *    *  minor - minor version number.
 *
 *    @param interp - the intepreter object whose result will be set by this.
 *    @param pSpecificItem - Actually a CDataFormatItem pointer.
*/ 
void
CTclRingCommand::formatFormatItem(CTCLInterpreter& interp, CRingItem* pSpecificItem)
{
    CDataFormatItem* p = reinterpret_cast<CDataFormatItem*>(pSpecificItem);
    
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += p->typeName();

    result += "major";
    result += static_cast<int>(p->getMajor());
    
    result += "minor";
    result += static_cast<int>(p->getMinor());
    
    
    interp.setResult(result);
}
/**
 * formatEvent
 *    Formats a physics event.  This is going ot have:
 *    *  type - "Event"
 *    *  bodyheader if one exists.
 *    *  size - number of bytes in the event
 *    *  body - byte array containing the event data.
 *    @param interp - the intepreter object whose result will be set by this.
 *    @param pSpecificItem - The ring item.
*/ 

void
CTclRingCommand::formatEvent(CTCLInterpreter& interp, CRingItem* pSpecificItem)
{
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += pSpecificItem->typeName();
    
    result += "size";
    result += static_cast<int>(pSpecificItem->getBodySize());
    
    result += "body";
    Tcl_Obj* body = Tcl_NewByteArrayObj(
        reinterpret_cast<const unsigned char*>(pSpecificItem->getBodyPointer()),
        static_cast<int>(pSpecificItem->getBodySize()));
    CTCLObject obj(body);
    obj.Bind(interp);
    result += obj;
    
    if (pSpecificItem->hasBodyHeader()) {
        result += "bodyheader";
        result += formatBodyHeader(interp, pSpecificItem);
    }
    
    interp.setResult(result);
}

/**
 * formatFragment
 *
 *   Format an EVB_FRAGMENT ring item.
 *    *   type - "Event fragment"
 *    *   - timestamp - the 64 bit timestamp.
 *    *   - source    - The source id.
 *    *   - barrier   - The barrier type.
 *    *   - size      - payload size.
 *    *   - body      - Byte array containing the body.
 *
 *    @param interp - the intepreter object whose result will be set by this.
 *    @param pSpecificItem - The ring item.
 */
void
CTclRingCommand::formatFragment(CTCLInterpreter& interp, CRingItem* pSpecificItem)
{
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += pSpecificItem->typeName();
    
    CRingFragmentItem* p = reinterpret_cast<CRingFragmentItem*>(pSpecificItem);
    
    result += "timestamp";
    Tcl_Obj* pTs = Tcl_NewWideIntObj(static_cast<Tcl_WideInt>(p->timestamp()));
    CTCLObject stamp(pTs);
    stamp.Bind(interp);
    result += stamp;
    
    result += "source";
    result += static_cast<int>(p->source());
    
    result += "barrier";
    result += static_cast<int>(p->barrierType());
    
    result += "size";
    result += static_cast<int>(p->payloadSize());
    
    result += "body";
    pEventBuilderFragment pFrag =
        reinterpret_cast<pEventBuilderFragment>(p->payloadPointer());
    Tcl_Obj* pBody =Tcl_NewByteArrayObj(
        pFrag->s_body,
        static_cast<int>(p->payloadSize()));
    CTCLObject body(pBody);
    body.Bind(interp);
    result += body;
    
    
    interp.setResult(result);
}
/**
 * formatTriggerCount
 *
 *   Format dicts for PHYSICS_EVENT_COUNT items.
 *   this dict has:
 *
 *    *   type : "Trigger count"
*     *   bodyheader if the item has a body header present.
*     *   timeoffset - 123 (offset into the run).
*     *   divisor    - 1   divisor needed to turn that to seconds.
*     *   triggers   - 1000 Number of triggers (note 64 bits).
*     *   realtime   - 0 time of day emitted.
*/
    
void
CTclRingCommand::formatTriggerCount(CTCLInterpreter& interp, CRingItem* pSpecificItem)
{
    CRingPhysicsEventCountItem* p =
        reinterpret_cast<CRingPhysicsEventCountItem*>(pSpecificItem);
    
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += p->typeName();
    
    result += "timeoffset";
    result += static_cast<int>(p->getTimeOffset());
    
    result += "divisor";
    result += static_cast<int>(p->getTimeDivisor());
    
    result += "realtime";
    result += static_cast<int>(p->getTimestamp());
    
    uint64_t ec = p->getEventCount();
    Tcl_Obj* eventCount = Tcl_NewWideIntObj(static_cast<Tcl_WideInt>(ec));
    CTCLObject eventCountObj(eventCount);
    eventCountObj.Bind(interp);
    result += "triggers";
    result += eventCountObj;
    
    if (p->hasBodyHeader()) {
        result += "bodyheader";
        result += formatBodyHeader(interp, pSpecificItem);
    }
    
    
    interp.setResult(result);
}
/**
 * formatGlomParams
 *     Format a glom parameters item.  This dict will have:
 *
 *     *  type - "Glom Parameters"
 *     *  bodyheader - never has this
 *     *  isBuilding - boolean True if building false otherwise.
 *     *  coincidenceWindow - Number of ticks in the coincidence window.
 *     *  timestampPolicy - one of "first", "last" or "average" indicating
 *        how the timestamp for the items are derived fromt he timestamps
 *        of their constituent fragments.
*/
void CTclRingCommand::formatGlomParams(CTCLInterpreter& interp, CRingItem* pSpecificItem)
{
    CGlomParameters *p = reinterpret_cast<CGlomParameters*>(pSpecificItem);
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += p->typeName();
    
    result += "isBuilding";
    result += (p->isBuilding() ? 1 : 0);
    
    result += "coincidenceWindow";
    Tcl_Obj* dTicks = Tcl_NewWideIntObj(
        static_cast<Tcl_WideInt>(p->coincidenceTicks())
    );
    CTCLObject window(dTicks);
    window.Bind(interp);
    result += window;
    
    CGlomParameters::TimestampPolicy policy = p->timestampPolicy();
    result += "timestampPolicy";
    if (policy == CGlomParameters::first) {
        result += "first";
    } else if (policy == CGlomParameters::last) {
        result += "last";
    } else {
        result += "average";
    }
    
    
    interp.setResult(result);
    
}
/**
 * formatAbnormalEnd
 *   We only provide  the type (Abnormal End).
 */
void
CTclRingCommand::formatAbnormalEnd(CTCLInterpreter& interp, CRingItem* pSpecificItem)
{

    CTCLObject result;
    result.Bind(interp);
    result += "type";
    result += pSpecificItem->typeName();
    interp.setResult(result);
}

CRingItem*
CTclRingCommand::getFromRing(CRingBuffer &ring, CRingSelectionPredicate &predicate,
                             unsigned long timeout)
{

    CTimeout timer(timeout);

    CRingItem* pItem = nullptr;

    do {
        pItem = getFromRing(ring, timer);

        if (pItem) {
            if (!predicate.selectThis(pItem->type())
                    || (predicate.getNumberOfSelections() == 0) ) {
                break;
            }

        }

        delete pItem;
        pItem = nullptr;
    } while (! timer.expired());

    return pItem;
}

CRingItem*
CTclRingCommand::getFromRing(CRingBuffer &ring, CTimeout& timer)
{
    using namespace std::chrono;

    // look at the header, figure out the byte order and count so we can
    // create the item and fill it in.
    RingItemHeader header;

    // block with a timeout
    while ((ring.availableData() < sizeof(header)) && !timer.expired()) {
        std::this_thread::sleep_for(milliseconds(200));
    }

    // stop if we have no data and we timed out...
    // if we have timed out but there is data, then try to continue.
    if (timer.expired() && (ring.availableData() <= sizeof(header))) {
        return nullptr;
    }

    ring.peek(&header, sizeof(header));
    bool otherOrder(false);
    uint32_t size = header.s_size;
    if ((header.s_type & 0xffff0000) != 0) {
      otherOrder = true;
      size = swal(size);
    }

    // prevent a thrown range error caused by attempting to read more data
    // than is in the buffer.
    if (ring.availableData() < size) {
        return nullptr;
    }


    std::vector<uint8_t> buffer(size);
    size_t gotSize    = ring.get(buffer.data(),
                                 buffer.size(),
                                 buffer.size(),
                                 timer.getRemainingSeconds());// Read the item from the ring.
    if(gotSize  != buffer.size()) {
      if (gotSize == 0) {
        // operation timed out
        return nullptr;
      }

      std::cerr << "Mismatch in CRingItem::getItem required size: sb " << size << " was " << gotSize
                << std::endl;
    }

    return CRingItemFactory::createRingItem(buffer.data());
}



uint32_t CTclRingCommand::swal(uint32_t value)
{
    union {
        uint32_t asValue;
        char asBytes[sizeof(uint32_t)];
    } value1, value2;

    value1.asValue = value;

    for (int fwIndex=0, bkwdIndex=sizeof(uint32_t);
         fwIndex<sizeof(uint32_t) && bkwdIndex>=0;
         fwIndex++, bkwdIndex--) {
        value2.asBytes[fwIndex] = value1.asBytes[bkwdIndex];
    }

    return value2.asValue;

}


/*-------------------------------------------------------------------------------
 * Package initialization:
 */

extern "C" 
int Tclringbuffer_Init(Tcl_Interp* pInterp)
{
    Tcl_PkgProvide(pInterp, "TclRingBuffer", "1.0");
    CTCLInterpreter* interp = new CTCLInterpreter(pInterp);
    CTclRingCommand* pCommand = new CTclRingCommand(*interp);
    
    return TCL_OK;
}


int gpTCLApplication = 0;
