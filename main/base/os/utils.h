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

/** @file:  utils.h
 *  @brief: Little utility methods/functions
 */
#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
uint32_t swal(uint32_t l);       // Swap bytes in long word
uint64_t    swaq(uint64_t aquad);
uint16_t    swaw(uint16_t aword);
#ifdef __cplusplus
} 
#endif

#endif