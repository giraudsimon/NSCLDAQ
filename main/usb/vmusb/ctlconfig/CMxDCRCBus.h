
#ifndef CMXDCRCBUS_H
#define CMXDCRCBUS_H

#include <string>
#include <cstdint>
#include <utility>
#include <CControlHardware.h>

class CVMUSB;
#include <CControlModule.h>

class CMxDCRCBus : public ::CControlHardware
{
  private:
    size_t m_maxPollAttempts;

  public:
    CMxDCRCBus();

    /**! Copy construct
     *
     */
    CMxDCRCBus(const CMxDCRCBus& rhs);

    CMxDCRCBus& operator=(const CMxDCRCBus& rhs);

    /**! Virtual Copy Constructor
     *
     */
    CControlHardware* clone() const;

    /**! \brief Add configuration parameters to CControlModule owning this
     *
     * \param config the object that owns this
     */
    virtual void onAttach(CControlModule& config);

    /**! Setup the MxDC for RC-bus operations
     *
     *  The MxDC module's NIM Busy ouput is hijacked to be used for the 
     *  RC-bus control.
     *
     *  \param ctlr the controller to communicate through
     */
    virtual void Initialize(CVMUSB& ctlr);

    /**! Does nothing but return "OK" */
    virtual std::string Update(CVMUSB& ctlr);

    /**! Write a value to a parameter address 
     *
     *  This attempts to send a write command to the device on the RC-bus. The 
     *  particular device and parameter address are identified by the what 
     *  argument. See the parseAddress method for the format accepted. It is not
     *  uncommon that the write will fail the first time so this will actually 
     *  attempt up to 4 times to reach a successful transmission. If it fails
     *  all 4 times, then an error response prefixed by "ERROR - " is returned
     *  containing an indication of what happened. Otherwise, if the
     *  transmission succeeded, the device will read back the value and check
     *  that it is the same as was written. If it differs, then an error
     *  response starting with "ERROR - " is returned indicating so.
     *
     *  \param ctlr   controller to communicate through
     *  \param what   address specifier (eg. d4a23 --> device 4, address 23)
     *  \param value  string representation of the value to write
     *
     *  \returns string indicating success or failure
     *  \retval "OK"          - success
     *  \retval "ERROR - ..." - for any failure on the RC-bus operations
     *  \throws string if communication with the VM-USB returns nonzero status
     */
    virtual std::string Set(CVMUSB& ctlr, std::string what, std::string value);

    /**! Read a value from a parameter address
     *
     * The behavior of this is similar to the Set method. It attempts 4 times to
     * read a value from an address in the specified device. If it succeeds
     * without error, the value is returned. Otherwise, if after 4 attempts it
     * has still not succeeded, the device will be returned as zero.
     *
     * \param ctlr    controller to communicate through
     * \param what    address specifier (eg. d4a23 --> device 4, address 23)
     *
     * \returns string indicating success or failure
     * \retval "OK"           - success
     *  \retval "ERROR - ..." - for any failure on the RC-bus operations
     *  \throws string if communication with the VM-USB returns nonzero status
     */
    virtual std::string Get(CVMUSB& ctlr, std::string what);


    // Getters and setters over the number of allowed poll attempts
    size_t getPollTimeout() const { return m_maxPollAttempts;}
    void   setPollTimeout(size_t value) { m_maxPollAttempts = value; }

  private:
    ///////////////////////////////////////////////////////////////////////////
    // UTILITY methods
    //

    /**! Copy the state of another instance
     *
     */
    virtual void copy(const CMxDCRCBus& rhs);

    /**! Enlist NIM Busy output on MxDC for RC-bus operations
     *
     *  \param ctlr   controller to communicate through 
     *
     *  \throws string if communicate with the VM-USB returns nonzero status
     */
    void activate(CVMUSB& ctlr);


    /**! Poll the 0x608A address until bit0 is 0
     *
     * This is called after an initiateWrite or initiateRead method to
     * wait for the device's error response.
     * 
     * \param ctlr    controller to communicate through 
     *
     * \return value read from 0x608A once bit0 becomes 0
     *
     * \throws string if communication with the VM-USB returns nonzero status
     */
    uint16_t pollForResponse(CVMUSB& ctlr); 

    /**! Read the resulting data response at 0x6088
     *
     * \param ctlr    controller to communicate through
     *
     * \return value read from 0x6088
     * \throws string if communication with the VM-USB returns nonzero status
     */
    uint16_t readResult(CVMUSB& ctlr);

    /**! Set up the commands for a write and then put value on the wire
     *
     * \param ctlr    controller to communicate through
     * \param what    parameter address (e.g. d2a42 --> device 2, address 42)
     * \param value   value to write
     *
     * \throws string if communication with the VM-USB returns nonzero status
     */
    void initiateWrite(CVMUSB& ctlr, std::string what, uint16_t value);

    /**! Set up the commands for a read and then execute the action
     *
     * \param ctlr    controller to communicate through
     * \param what    parameter address (e.g. d2a42 --> device 2, address 42)
     *
     * \throws string if communication with the VM-USB returns nonzero status
     */
    void initiateRead(CVMUSB& ctlr, std::string what);

    /**! Parse the string encoded with device number and parameter address
     *
     * The "what" parameter in the Get and Set methods contains both the device
     * number and the parameter address. The format is quite simple and is just:
     * dXaY, where X is the device number and Y is the parameter address. In 
     * RC-bus lingo the device number sometime referred to as a device address.
     * 
     * \param what    string of form dXaY where X=device number, Y=param address
     * \returns  pair with first=X, second=Y
     *
     * \throws string if unable to parse the format
     */
    std::pair<uint16_t,uint16_t> parseAddress(std::string what);

    /**! Add the commands for initiating a write to a readout list
     *
     * \param list  a readout list
     * \param addr  the parameter address struct returned from parseAddresses
     * \param value the value to write
     *
     */
    void addParameterWrite(CVMUSBReadoutList& list, 
                           std::pair<uint16_t,uint16_t> addr,
                           uint16_t value);

    /**! Add the commands for initiating a read to a readout list
     *
     * \param list  a readout list
     * \param addr  the parameter address struct returned from parseAddresses
     *
     */
    void addParameterRead(CVMUSBReadoutList& list, 
                           std::pair<uint16_t,uint16_t> addr);

    /**! Check whether error bits are set
     *
     * This is called to evaluate whether a transaction succeeded or ended
     * erroneously with a known error.
     *
     * \param datum  the word to check
     *
     * \retval true  - an error bit was set in datum
     * \retval false - no error bits were set 
     *
     */
    bool responseIndicatesError(uint16_t datum);

    /**! Return a string specifying what error occurred
     *
     * Checks the error bits to determine what type of error occurred. 
     * It is assumed that the user has already verified that an error bit is
     * set.
     *
     * \param datum   response word with error bits set
     *
     * \return string providing reason for error bit
     */
    std::string convertResponseToErrorString(uint16_t datum);
};

#endif
