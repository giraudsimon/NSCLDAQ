/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#ifndef CTransform10p0to11p0_H
#define CTransform10p0to11p0_H

namespace DAQ {

  // Fwd declarations
  namespace V10
  {
      class CRingItem;
  }

  namespace V11 {
      class CRingItem;
      class CRingScalerItem;
      class CRingStateChangeItem;
      class CPhysicsEventItem;
      class CRingPhysicsEventCountItem;
      class CRingTextItem;
      class CRingFragmentItem;
  }

  namespace Transform {


/*! \brief Transformation from 10.0 to 11.0 data.
 *
 *  The transformations defined here are the DAQ committee approved
 *  transformations for data formats 10.0 to 11.0. There are individual
 *  transformation methods that are implemented to provide the specific
 *  transforms for each data type. In general, these are all very
 *  straightforward and do what you would expect. The details of each
 *  transform of described with the specific method.
 *
 *  The output of these transforms are generic CRingItems and not references
 *  to their derived types. They contain all of the proper information, but
 *  the following code will throw a std::bad_cast exception:
 *  \code
 *      NSCLDAQ10::CRingScalerItem oldItem(32);
 *      auto newItem = CTransform10p0to11p0()(oldItem);
 *      V11::CRingScalerItem& sclr
 *           = dynamic_cast<V11::CRingScalerItem&>(newItem);
 *  \endcode
 *
 *  The better method for this is to use the
 *
 */
class CTransform10p0to11p0 
{
  public:
    typedef typename V10::CRingItem InitialType;
    typedef typename V11::CRingItem FinalType;

  public:
    /*! \brief Main entry point for any transform
     *
     *  This really is just a wrapper around the dispatch method.
     *
     * \param item  Version 10.0 item
     * \returns Version 11.0 item
     */
    FinalType operator()(InitialType& item);

    /*!
     * \brief Dispatches to the appropriate method
     *
     * \param item  Version 10.0 item
     * \return Version 11.0 item produced by transform
     */
    FinalType dispatch(InitialType& item);

   public:

    /*!
     * \brief Transform an INCREMENTAL_SCALER type to an 11.0 scaler
     *
     * The 11.0 scaler is produced from the 10.0 INCREMENTAL_SCALER type.
     * This is accomplished according to the following rules:
     *  - Size in ring item header will be recomputed.
     *  - The type will map as INCREMENTAL_SCALERS -> PERIODIC_SCALERS
     *  - No body header will be included in the 11.0 ring item
     *  - Interval start offset will remain the same
     *  - Interval end offset will remain the same
     *  - Timestamp (Unix) will remain the same
     *  - Interval Divisor will be set to 1.
     *  - Scaler Count will remain the same.
     *  - Is Incremental will be set to 1.
     *  - Scaler values will remain the same
     *
     * \param item  10.0 incremental scaler item
     * \return transformed item (PERIODIC_SCALER type)
     */
    V11::CRingScalerItem transformScaler(InitialType& item);

    /*!
     * \brief Transforms 10.0 State Change to an 11.0 State Change
     *
     *  The rules for this transform are as follows:
     *  - Size in ring item header will be recomputed.
     *  - The types will map as:
     *    - BEGIN_RUN  -> BEGIN_RUN
     *    - END_RUN    -> END_RUN
     *    - PAUSE_RUN  -> PAUSE_RUN
     *    - RESUME_RUN -> RESUME_RUN
     *  - No body header will be included in the resulting 11.0 ring item.
     *  - The run number will transform unchanged.
     *  - The time offset will transform unchanged.
     *  - The timestamp (Unix) will transform unchanged.
     *  - The offset divisor will be set to 1.
     *  - The title will transform unchanged
     *
     * \param item 10.0 state change type
     * \return transformed item in 11.0 format
     */
    V11::CRingStateChangeItem transformStateChange(InitialType& item);

    /*!
     * \brief Transform a physics event between 10.0 and 11.0 formats
     *
     *  The rules for this transform are as follows:
     *  - Size in ring item header will be recomputed.
     *  - The type will map as PHYSICS_EVENT -> PHYSICS_EVENT
     *  - No body header will be created in the 11.0 ring item.
     *  - Body data will transform unchanged
     *
     * \param item  10.0 physics event item
     * \return transformed 11.0 physics event
     */
    V11::CPhysicsEventItem transformPhysicsEvent(InitialType& item);

    /*!
     * \brief Transform Physics Event Counts from 10.0 to 11.0
     *
     * The rules for this transform are as follows:
     *  - Size in ring item header will be recomputed.
     *  - The type will map as PHYSICS_EVENT_COUNT -> PHYSICS_EVENT_COUNT
     *  - No body header will be included in the 11.0 ring item.
     *  - Time offset will remain the same.
     *  - Offset divisor will be set to 1.
     *  - Timestamp (unix) will remain the same.
     *  - Event count will remain the same.
     * \param item
     * \return
     */
    V11::CRingPhysicsEventCountItem transformPhysicsEventCount(InitialType& item);

    /*!
     * \brief Transform a text item between 10.0 and 11.0 formats
     *
     * The rules for this transform are as follows:
     * - Size in ring item header will be recomputed.
     * - The type will transform as :
     *   - PACKET_TYPES  -> PACKET_TYPES
     *   - MONITORED_VARIABLES  -> MONITORED_VARIABLES
     * - No body header will be included in the 11.0 ring item.
     * - Time offset will transform unchanged.
     * - Timestamp (Unix) will transform unchanged.
     * - String count will transform unchanged.
     * - The offset divisor will be set to 1.
     * - The strings will transform unchanged.
     *
     * \param item 10.0 text item type
     * \return transformed 11.0 item
     */
    V11::CRingTextItem transformText(InitialType& item);

    /*!
     * \brief Transform timestamped non-incr scaler (aka. timestamped running sclr)
     *
     * The rules for this transform are as follows:
     * - Size in ring item header will be recomputed.
     * - Type will map as TIMESTAMPED_NONINCR_SCALERS -> PERIODIC_SCALERS
     * - No body header will be created in the 11.0 ring item.
     * - Interval start offset will remain the same
     * - Interval end offset will remain the same
     * - Interval divisor will be set to 1.
     * - Timestamp (Unix) will be become timestamp (unix).
     * - Scaler count will transform unchanged.
     * - Is Incremental will be set to 0.
     * - Scaler values will transform unchanged.
     * - Event timestamp will be discarded.
     *
     * \param item 10.0 non-incr scaler item
     * \return transformed 11.0 item
     */
    V11::CRingScalerItem transformNonIncrScaler(InitialType& item);

    /*!
     * \brief Transform a 10.0 Fragment to an 11.0 Fragment
     *
     *  This should not be confused with transforming 11.0 EVB::fragment
     *  types. Rather it is just a conversion between the CRingFragmentItem
     *  types in 10.0 and 11.0.
     *
     *  The rules are as follow:
     * - Size in ring item header will be recomputed.
     * - Type will map as EVB_FRAGMENT -> EVB_FRAGMENT
     * - Body header will be created in the 11.0 ring item.
     *   - Body timestamp will be used for body header timestamp
     *   - Body source id will be used for body header source id
     *   - Payload size will be discarded. The value can easily be calculated
     *     from the size value in the RingItemHeader and the size of the body
     *     header.
     *   - Body barrier type will be used for body header barrier type
     * - Payload will remain the same.
     *
     * Note that the payload may consist of fragments in a given format and
     * this is not attempting to convert them. The result will be that the top
     * level format of the built ring item will correspond to version 11.0, but
     * the payload will contain fragments in the 10.0 format.
     *
     * \param item the 10.0 item
     * \return the transformed 11.0 item
     */
    V11::CRingFragmentItem transformFragment(InitialType& item);
};

  } // end Transform
} // end DAQ

#endif
