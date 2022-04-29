/*
 Extract the Pixie16 API includes and conditionalize them on 
the API version 
*/
#ifdef USING_DDAS

#if XIAAPI_VERSION >= 3
/*
Need to include additional header for size_t because pixie16.h 
is not inclusive as of version 3.4.0
*/
#include <cstddev>
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

/*  Readout programs need this to decide when to start reading */
#ifndef EXTFIFO_READ_THRESH
#define EXTFIFO_READ_THRESH   1024
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

#endif

#endif
