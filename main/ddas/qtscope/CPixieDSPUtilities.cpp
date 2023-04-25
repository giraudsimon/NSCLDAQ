/**
 * @file CPixieDSPUtilities.cpp
 * @brief Implement the class to read and write settings to XIA Pixie modules.
 */

#include "CPixieDSPUtilities.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <bitset>
#include <string>

#include <config.h>
#include <config_pixie16api.h>

/**
 * @brief Constructor.
 */
CPixieDSPUtilities::CPixieDSPUtilities() {}

/**
 * @brief Destructor.
 */
CPixieDSPUtilities::~CPixieDSPUtilities() {}

/**
 * @brief Adjust DC offsets of all channels on a given module.
 *
 * @param module  Module number.
 *
 * @return int  0 on success, XIA API error code on fail.
 */
int
CPixieDSPUtilities::AdjustOffsets(int module)
{
  int retval = Pixie16AdjustOffsets(module);

  if (retval < 0) {
    std::cerr << "CPixieDSPUtilities::AdjustOffsets() failed to adjust offsets in module: " << module << " with retval " << retval;
  }

  return retval;
}

/**
 * @brief Write a Parameter parameter to a single channel. 
 *
 * Channel parameters are doubles. For a list of parameters and their units, 
 * see the Pixie-16 Programmers Manual, pgs. 60-61.
 *
 * @param module     Module number.
 * @param channel    Channel number on module.
 * @param paramName  XIA API chanel parameter name.
 * @param value      Parameter value to write.
 *
 * @return int  0 on success, XIA API error code on fail.
 */
int
CPixieDSPUtilities::WriteChanPar(int module, int channel, char* paramName, double value)
{
  int retval = Pixie16WriteSglChanPar(paramName, value, module, channel);
  
  if (retval < 0) {
    std::cerr << "CPixieDSPUtilities::WriteChanPar() failed to write parameter "
	      << paramName << " to module " << module << " channel "
	      << channel << " with retval " << retval << std::endl;
  }
    
  return retval;
}

/**
 * @brief Read a Parameter parameter from a single channel.
 *
 * Channel parameters are doubles. For a list of parameters and their units, 
 * see the Pixie-16 Programmers Manual, pgs. 60-61.
 *  
 * @param[in] module     Module number.
 * @param[in] channel    Channel number on module.
 * @param[in] paramName  XIA API channel parameter name.
 * @param[in,out] value  Reference to read parameter value.
 *
 * @return int  0 on success, XIA API error code on fail.
 */
int
CPixieDSPUtilities::ReadChanPar(int module, int channel, char* paramName, double& value)
{
  int retval = Pixie16ReadSglChanPar(paramName, &value, module, channel);
  
  if (retval != 0) {
    std::cerr << "CPixieDSPUtilities::ReadChanPar() failed to read parameter "
	      << paramName << " from module " << module << " channel "
	      << channel << " with retval " << retval << std::endl;
  }  
  
  return retval;
}

/**
 * WriteModPar
 *
 * @brief Write a module-level Parameter parameter to a single module. 
 *
 * Module parameters are unsigned ints. For a list of parameters and their 
 * units, see the Pixie-16 Programmers Manual, pgs. 62-63. 
 *  
 * @param module     Module number.
 * @param paramName  XIA API chanel parameter name.
 * @param value      Parameter value to write.
 *
 * @return int  0 on success, XIA API error code on fail.
 */
int
CPixieDSPUtilities::WriteModPar(int module, char* paramName, unsigned int value)
{
  int retval = Pixie16WriteSglModPar(paramName, value, module);
  
  if (retval < 0) {
    std::cerr << "CPixieDSPUtilities::WriteModPar() failed to write parameter "
	      << paramName << " to module " << module << " with retval "
	      << retval << std::endl;
  } 
  
  return retval;
}

/**
 * @brief Read a module-level Parameter parameter to a single module. 
 *
 * Module parameters are unsigned ints. For a list of parameters and their 
 * units, see the Pixie-16 Programmers Manual, pgs. 62-63. 
 * 
 * @param[in] module     Module number.
 * @param[in] paramName  XIA API chanel parameter name.
 * @param[in,out] value  Reference to read parameter value.
 *
 * @return int  0 on success, XIA API error code on fail.
 */
int
CPixieDSPUtilities::ReadModPar(int module, char* paramName, unsigned int& value)
{
  int retval = Pixie16ReadSglModPar(paramName, &value, module);
  
  if (retval != 0) {
    std::cerr << "CPixieDSPUtilities::ReadModPar() failed to read parameter "
	      << paramName << " from module " << module << " with retval "
	      << retval << std::endl;
  }
 
  return retval;
}
