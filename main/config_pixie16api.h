/*
  Extract the Pixie16 API includes and conditionalize them on 
  the API version 
*/
#if XIAAPI_VERSION >= 3

/*
  Need to include additional header for size_t because pixie16.h 
  is not inclusive as of version 3.7.0
*/
#include <cstddef>
#include <pixie16/pixie16.h>

/* 
   The transition manual says these will be deprecated but the API 
   still needs them and I'm damned if I'll use magic numbers instead so:
*/
#ifndef LIST_MODE_RUN
#define LIST_MODE_RUN 0x100
#endif

#ifndef NEW_RUN
#define NEW_RUN 1
#endif

/* Readout programs need this to decide when to start reading */
#ifndef EXTFIFO_READ_THRESH
#define EXTFIFO_READ_THRESH   1024
#endif

/*
  More things that will be deprecated but still rather avoid magic numbers:
  Below are all from SetFileWriter.cpp in xml...
*/
#ifndef N_DSP_PAR
#define N_DSP_PAR 1280
#endif

#ifndef DAC_VOLTAGE_RANGE
#define DAC_VOLTAGE_RANGE 3.0
#endif

#ifndef DSP_CLOCK_MHZ
#define DSP_CLOCK_MHZ 100
#endif

#ifndef CFDSCALE_MAX
#define CFDSCALE_MAX 7
#endif

#ifndef CFDTHRESH_MAX
#define CFDTHRESH_MAX 65535
#endif

#ifndef CFDDELAY_MIN
#define CFDDELAY_MIN 1
#endif

#ifndef CFDDELAY_MAX
#define CFDDELAY_MAX 63
#endif

#ifndef VETOSTRETCH_MIN
#define VETOSTRETCH_MIN 1
#endif

#ifndef VETOSTRETCH_MAX
#define VETOSTRETCH_MAX 4095
#endif

#ifndef EXTDELAYLEN_MIN
#define EXTDELAYLEN_MIN 0
#endif

#ifndef EXTDELAYLEN_MAX_REVBCD
#define EXTDELAYLEN_MAX_REVBCD 255
#endif

#ifndef FASTTRIGBACKDELAY_MIN
#define FASTTRIGBACKDELAY_MIN 0
#endif

#ifndef FASTTRIGBACKDELAY_MAX_REVBCD
#define FASTTRIGBACKDELAY_MAX_REVBCD 255
#endif

#ifndef FASTTRIGBACKLEN_MIN_125MHZFIPCLK
#define FASTTRIGBACKLEN_MIN_125MHZFIPCLK 2
#endif

#ifndef FASTTRIGBACKLEN_MIN_100MHZFIPCLK
#define FASTTRIGBACKLEN_MIN_100MHZFIPCLK 1
#endif

#ifndef FASTTRIGBACKLEN_MAX
#define FASTTRIGBACKLEN_MAX 4095
#endif

#ifndef CHANTRIGSTRETCH_MIN
#define CHANTRIGSTRETCH_MIN 1
#endif

#ifndef CHANTRIGSTRETCH_MAX
#define CHANTRIGSTRETCH_MAX 4095
#endif

#ifndef EXTTRIGSTRETCH_MIN
#define EXTTRIGSTRETCH_MIN 1
#endif

#ifndef EXTTRIGSTRETCH_MAX
#define EXTTRIGSTRETCH_MAX 4095
#endif

#ifndef FASTFILTER_MAX_LEN
#define FASTFILTER_MAX_LEN 127
#endif

#ifndef MIN_FASTLENGTH_LEN
#define MIN_FASTLENGTH_LEN 2
#endif

#ifndef FAST_THRESHOLD_MAX
#define FAST_THRESHOLD_MAX 65535
#endif

#ifndef SLOWFILTER_MAX_LEN
#define SLOWFILTER_MAX_LEN 127
#endif

#ifndef MIN_SLOWLENGTH_LEN
#define MIN_SLOWLENGTH_LEN 2
#endif

#ifndef MIN_SLOWGAP_LEN
#define MIN_SLOWGAP_LEN 3
#endif

#ifndef QDCLEN_MIN
#define QDCLEN_MIN 1
#endif

#ifndef QDCLEN_MAX
#define QDCLEN_MAX 32767
#endif

#ifndef TRACELEN_MIN_500MHZADC
#define TRACELEN_MIN_500MHZADC 10
#endif

#ifndef TRACELEN_MIN_250OR100MHZADC
#define TRACELEN_MIN_250OR100MHZADC 4
#endif

#ifndef TRACEDELAY_MAX
#define TRACEDELAY_MAX 1023
#endif

#else

#ifndef PLX_LINUX
#define PLX_LINUX
#endif

#include <pixie16app_common.h>
#include <pixie16app_defs.h>
#include <pixie16app_export.h>
#include <xia_common.h>

#endif
