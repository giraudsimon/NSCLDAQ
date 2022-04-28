
#ifndef WienerMDGG16Register_H
#define WienerMDGG16Register_H

#include <stdint.h>

/**! This file defines a structured namespace to describe the registers
 * of the MDGG-16 device. It is not an exhaustive list but provides the 
 * registers that are currently operated on in the device by other classes
 * such as the WienerMDGG16::CDeviceDriver.
 *
 */
namespace WienerMDGG16 
{
  // register address offsets
  namespace Regs {
    const uint32_t FirmwareID    = 0x0000;
    const uint32_t Global        = 0x0004;
    const uint32_t Auxiliary     = 0x0008;
    const uint32_t ECL_Output    = 0x000c;
    const uint32_t LEDNIM_Output    = 0x00d0;
    const uint32_t Logical_OR_AB     = 0x00b8;
    const uint32_t Logical_OR_CD     = 0x00bc;
  }

  // Bit meanings/offsets for ECL_Output register
  namespace ECL_Output {
    const uint32_t ECL9_Offset  = 0;
    const uint32_t ECL10_Offset = 4;
    const uint32_t ECL11_Offset = 8;
    const uint32_t ECL12_Offset = 12;
    const uint32_t ECL13_Offset = 16;
    const uint32_t ECL14_Offset = 20;
    const uint32_t ECL15_Offset = 24;
    const uint32_t ECL16_Offset = 28;

    const uint32_t Disable    = 0;
    const uint32_t DGG        = 1;
    const uint32_t ECL_Input  = 2;
    const uint32_t Logical_OR = 3;
  }

  // Bit meanings/offsets for Logical_OR register
  namespace Logical_OR {
    const uint32_t A_Offset = 0;
    const uint32_t B_Offset = 16;
    const uint32_t C_Offset = 0;
    const uint32_t D_Offset = 16;
  }

  // Bit meanings/offsets for LED/NIM register
  namespace LEDNIM_Output {
    // shifts for each set of bits
    const uint32_t LEDGreen_Lt_Shift = 0;
    const uint32_t LEDGreen_Rt_Shift = 4;
    const uint32_t LEDYellow_Lt_Shift = 8;
    const uint32_t LEDYellow_Rt_Shift = 12;
    const uint32_t NIM1_Shift = 16;
    const uint32_t NIM2_Shift = 20;
    const uint32_t NIM3_Shift = 24;
    const uint32_t NIM4_Shift = 28;

    // common mask for all of the sets of bits...
    // b/c they are all 4 bits wide.
    const uint32_t Bit_Mask = 0xf;

    const uint32_t LED_ECL_OR_1234 = 0;
    const uint32_t LED_ECL_OR_12 = 1;
    const uint32_t LED_ECL_OR_23 = 2;
    const uint32_t LED_ECL_OR_34 = 3;
    const uint32_t LED_ECL_OUT1 = 4;
    const uint32_t LED_ECL_OUT2 = 5;
    const uint32_t LED_ECL_OUT3 = 6;
    const uint32_t LED_ECL_OUT4 = 7;

    // values for the NIM bits
    const uint32_t NIM_None = 0;
    const uint32_t NIM_ECL1 = 1;
    const uint32_t NIM_ECL2 = 2;
    const uint32_t NIM_ECL3 = 3;
    const uint32_t NIM_ECL4 = 4;
    const uint32_t NIM_NIMIn1 = 5;
    const uint32_t NIM_Logical_OR = 6;
  }

} // end namespace

#endif

