

#ifndef ABNORMALENDRUNFILTERHANDLER_H
#define ABNORMALENDRUNFILTERHANDLER_H

#include <CFilter.h>

class CDataSink;
class CRingItem;
class CRingStateChangeItem;
class CRingScalerItem;
class CPhysicsEventItem;
class CRingPhysicsEventCountItem;
class CRingFragmentItem;
class CRingTextItem;


/**! \brief Filter providing logic for handling ABNORMAL_ENDRUN items
 *
 * The ABNORMAL_ENDRUN item is supposed to be outputted when something
 * bad has happened. Its purpose is the flush through the data stream
 * and kill off every process it encounters. So the unique thing about this
 * is that once observed, it must be passed on and only then can the
 * process exit. THis does essentially that. It is kind of specialized
 * because it takes matters into its own hands by performing the write
 * to the data sink itself. The user should set this up by doing the 
 * following in their main:
 *
 * #include <CAbnormalEndRunFilterHandler.h>
 * #include <CMediator.h>
 *
 * int main(int argc, char* argv[]) {
 *  // ...
 * CFilterMain theApp(argc,argv);
 * CAbnormalEndRunFilterHandler abnHandler(*(theApp.getMediator()->getDataSink()));
 * main.registerFilter(&abnHandler);
 *  // ...
 * }
 */
class CAbnormalEndRunFilterHandler : public CFilter 
{

  private:
    CDataSink& m_sink;

  public:
    CAbnormalEndRunFilterHandler(CDataSink& sink ) 
       : m_sink(sink) {}

    CAbnormalEndRunFilterHandler(const CAbnormalEndRunFilterHandler& rhs);

    CAbnormalEndRunFilterHandler* clone() const {
      return new CAbnormalEndRunFilterHandler(*this);
    }

  private:
    CAbnormalEndRunFilterHandler& operator=(const CAbnormalEndRunFilterHandler& rhs);

    /*! \brief Checks for ABNORMAL_ENDRUN presence
     *
     * If the ring item is an ABNORMAL_ENDRUN, it sets a flag to ensure
     * that an exception is thrown on the next iteration. Note that 
     * the next iteration may not come... in which case this would probably
     * just exit normally.
     *
     */
    CRingItem* handleRingItem(CRingItem* item);


    // these are all just wrappers around the handleRingItem method. They
    // simply do the following:
    //
    //   return handleRingItem(item);
    //
    CRingItem* handleStateChangeItem(CRingStateChangeItem* item);
    CRingItem* handleScalerItem(CRingStateChangeItem* item);
    CRingItem* handleTextItem(CRingTextItem* item);
    CRingItem* handlePhysicsEventItem(CPhysicsEventItem* item);
    CRingItem* handlePhysicsEventCountItem(CRingPhysicsEventCountItem* item);
    CRingItem* handleFragmentItem(CRingFragmentItem* item);

};

#endif
