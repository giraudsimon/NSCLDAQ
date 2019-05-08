/******************************************************************************
*
* CAEN SpA - Software Division
* Via Vetraia, 11 - 55049 - Viareggio ITALY
* +390594388398 - www.caen.it
*
***************************************************************************//**
* 
* Name      CAENComm.h
* Project   CAENComm - Communication Layer - foreign devices.
*/

/**
 * @brief  Code that is common to the SBS and VM-USB code factored out.
 */
#include <string.h>
#include <CAENComm.h>


// Below are the set of error strings we recognize.
// 

static const char* ErrorMessages[] = {
  "Success",
  "VME Bus error during cycle",
  "Communication error",
  "Unspecified Error",
  "Invalid Parameter",
  "Invalid LinkType",
  "Invalid Device Handle",
  "Communication timeout",
  "Unable to open requested device",
  "Maximum number of devices exceeded",
  "Device is already open",
  "Not supported function",
  "There aren't any boards controlled by this bridge",
  "Communication terminated by the device"

};

static const size_t nErrorMessages = sizeof(ErrorMessages)/sizeof(char*);

// Could really be C :-(

extern "C" {

/**************************************************************************//**
* \fn      void CAENComm_DecodeError(int ErrCode, char *ErrMsg)
* \brief   Decode error code
*
* \param   [IN]  ErrCode: Error code
* \param   [OUT] ErrMsg: string with the error message
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_DecodeError(int ErrCode,  char *ErrMsg)
{
  // Note that the error code is the negative of the index into the ErrorMessages
  // array:


  int idx = -ErrCode;
  if (idx < nErrorMessages) {
    strcpy(ErrMsg, ErrorMessages[idx]);
  } 
  else {
    return CAENComm_GenericError; // kind of ironic don't you think?
  }
  return CAENComm_Success;
}

}
