
#ifndef CLeCroy4448_H
#define CLeCroy4448_H

#include "CReadoutHardwareT.h"
#include <CReadoutModule.h>

// forward declaration of the class
template<class Controller, class RdoList> class CLeCroy4448;

    
/**! \brief A hybrid camac driver for the LeCroy4448 scaler 
*
*   Depending on the template parameters, an object of this class 
*   function on either a CBD8210 branch or within a crate controlled 
*   by a CCUSB. In the context of the CBD8210, an object of this type
*   must be registered to a camac crate. It only makes use of basic 
*   camac commands such as 16- and 24-bit reads/writes and control operations.   
*   
*/
template<class Controller, class RdoList>
class CLeCroy4448 : public CReadoutHardwareT<Controller,RdoList>
{

    private:
    CReadoutModule* m_pConfig;  ///! The parent object

    public:
    /**! \brief Default constructor
    *  
    *   Initializes the m_pConfig to zero. 
    */
    CLeCroy4448();

    /**! \brief Copy constructor
    *   
    *   Calls the base class constructor and nothing else. This
    *   does not copy thepointer to the parent. In essence the 
    *   a new object is produced without any owner. 
    *
    *   \param the object to copy
    */
    CLeCroy4448(const CLeCroy4448& rhs);


    /**! \brief Assignment operator 
    *   
    *   Does nothing. 
    *
    *   \param the object to copy
    *   
    *   \returns a reference to this
    */
    CLeCroy4448& operator=(const CLeCroy4448& rhs);

    /**! \brief Destructor
    *
    *   Does nothing. This object doesn't own anything.    
    */
    virtual ~CLeCroy4448();

    /**! \brief Set up the configuration parameters
    *
    *   Adds the -slot configuration flag. It defaults to a
    *   value of 0.
    *
    *   \param config SHOULD be a reference to the parent 
    */
    void onAttach(CReadoutModule& config);

    /**! \brief Initialization routine 
    *
    *   Clears the scaler module and disables auxiliary bus readout.
    *
    *   \param controller an abstract camac crate controller
    */
    void Initialize(Controller& controller);

    /**! \brief Readout routine 
    *
    *   Appends the readout routine for this module to the readoutlist.
    *   this readout list will eventuially be loaded as a stack in the
    *   true physical controller.
    *
    *   Readout routine:
    *   1. Reads all 32 channels of scalers 
    *   2. Clears the module.
    *
    *   \param rdolist the readout list that represents the stack 
    */
    void addReadoutList(RdoList& rdolist);

    /**! \brief onEndRun routine 
    *
    *   This is a no-op 
    *
    *   \param controller an abstract camac crate controller
    */
    void onEndRun(Controller& controller) {}

    /**! \brief Virtual copy constructor
    *
    *   Copies the object in a polymorphic manner.
    *
    *   \return a dynamically allocated copy of this object
    */
    CLeCroy4448* clone() const {
        return new CLeCroy4448(*this);
    }

    /**! \brief Clear the device 
    *
    *   \param ctlr the controller 
    */
    void Clear(Controller& ctlr);

    /**! \brief Add clear routine to readoutlist
    *
    *   \param list the readout list that represents the stack
    */
    void sClear(RdoList& list);

    /**! \brief Add single channel read routine to readoutlist
    *
    *   \param list the readout list that represents the stack
    *   \param a the channel to read out 
    */
    void sRead(RdoList& list, int reg);

};

#include "CLeCroy4448.hpp"

#endif
