/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   testDataSource.cpp
# @brief  Provide data that looks like it comes from nscldaq-8.x
# @author Ron Fox <fox@nscl.msu.edu>
*/


#include "buffer.h"
#include "buftypes.h"
#include "test.h"

#include <stdlib.h>
#include <iostream>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <io.h>
#include <stdlib.h>

// File scoped variables:


// Configuration

unsigned    runNumber(0);
const char* title("");
int         buffersLeft(1000);
unsigned    eventsPerScaler(1000);
unsigned    numScalers(32);

// What an event looks like:

struct event {
    uint16_t nWds;
    uint16_t tstamp[4];         // Little endian timestamp.
    uint16_t body[16];
};

// Output Buffer and book keeping:

uint8_t buffer[8192];
struct bheader* pHeader = reinterpret_cast<struct bheader*>(buffer);
void*           pBody   = reinterpret_cast<void*>(&(pHeader[1]));
struct event*   pEventCursor = reinterpret_cast<struct event*>(pBody);
unsigned        nWordsRemaining(8192 - sizeof(struct bheader)/sizeof(uint16_t));
uint64_t        timestamp(0);
uint32_t        sequence(0);

// Simulated elapsed time

time_t simulatedTod;
unsigned secsPerScaler(2);
unsigned runTime(0);

/**
 * initHeader
 *   Initializes the buffer header with as much information as we can.
 *   (nevt and nwds have to be set later).
 *  @param type - buffer type.
 */
void initHeader(int type) {
    pHeader->nwds    = 0;
    pHeader->type    = type;
    pHeader->cks     = 0; // unused
    pHeader->run     = runNumber;
    pHeader->seq     = sequence++;
    pHeader->nevt    = 0;
    pHeader->nlam    = 0;
    pHeader->cpu     = 1;
    pHeader->nbit    = 0;
    pHeader->buffmt  = BUFFER_REVISION;
    pHeader->ssignature = 0x0102;
    pHeader->lsignature = 0x01020304;
    pHeader->unused[0]  = 0;          // could leave this uninitialized.
    pHeader->unused[1]  = 0;
    
}
/**
 * makeTod
 *  Turns the current simulatedTod into a buftime
 *  @param pTime - Points to the buffer into which the time will be encoded.
 */
void makeTod(struct bftime* pTime) {
    struct tm timeFields;
    localtime_r(&simulatedTod, &timeFields);
    
    pTime->month  = timeFields.tm_mon + 1;
    pTime->day    = timeFields.tm_mday + 1;
    pTime->year   = timeFields.tm_year;
    pTime->hours  = timeFields.tm_hour;
    pTime->min    = timeFields.tm_min;
    pTime->sec    = timeFields.tm_sec;
    pTime->tenths = 0;
    
    
}
/**
 *  writeBuffer
 *    Write the current buffer to stdout.
 */
void writeBuffer() {
    std::cerr << "Write buffer " << pHeader->type << std::endl;
    io::writeData(STDOUT_FILENO, buffer, sizeof(buffer));
    buffersLeft--;
}

/**
 * stateChange
 *   emit a state change buffer.
 *
 *   @param type - the type of buffer (e.g. BEGRUNBF).
 */
static void stateChange(uint16_t type)
{
    initHeader(type);
    struct ctlbody* psBody = reinterpret_cast<struct ctlbody*>(pBody);
    memset(psBody->title, 0, 80);
    strcpy(psBody->title, title);
    psBody->sortim = runTime;
    makeTod(&psBody->tod);
    pHeader->nwds = (sizeof(struct bheader) + sizeof(struct ctlbody))/sizeof(uint16_t);
    
    writeBuffer();
}

/**
 * beginRun
 *   Emit a begin run buffer.
 */
static void beginRun()
{
    stateChange(BEGRUNBF);
}
/**
 * endRun
 *  Emit an end run buffer
 */
static void endRun()
{
    std::cerr << "End run\n";
    stateChange(ENDRUNBF);
}

static void initPhysHeader()
{
    initHeader(DATABF);
    pHeader->nwds = sizeof(struct bheader)/sizeof(uint16_t);
    
}
/**
 * flushEvents
 *    Flush an event buffer.
 *    * Compute the number of words based on the cursor
 *    * (nevt is a running count).
 *    * write the buffer.
 *    * reset the event cursor.
 *  @note this is a no-op if there are no events.
 */
void flushEvents()
{
    std::cerr << "Flushing events\n";
    if(pHeader->nevt > 0) {
        writeBuffer();
        pEventCursor = reinterpret_cast<struct event*>(pBody);
    }
    // Prep for next event buffer:
    
    initPhysHeader();    
}

/**
 * scaler
 *   Emit a scaler buffer.  The scalers themselves will have random values
 *   from 100-1000 and will represent a 2 second interval in the run. this means
 *   this function will have the side effect of updating runTime and the simulatedRunTime.
 *
 */
static void scaler()
{
    // Format the header:
    std::cerr << "Scaler buffer\n";
    initHeader(SCALERBF);
    pHeader->nwds = (sizeof(struct bheader) + sizeof(struct sclbody) +
        (numScalers-1)*sizeof(uint32_t))/sizeof(uint16_t);
    pHeader->nevt = numScalers;
    struct sclbody* pSBody = reinterpret_cast<struct sclbody*>(pBody);
    
    // The scaler values.
    
    uint32_t* pScalers = reinterpret_cast<uint32_t*>(pSBody->scalers);
    for (int i =0; i < numScalers; i++) {
        *pScalers++ = static_cast<uint32_t>(100.0 + drand48()*900.0);
    }
    
    // interval information:
    
    pSBody->btime = runTime;
    runTime      += secsPerScaler;
    pSBody->etime = runTime;
    pSBody->unused2[2] = 0;
    pSBody->unused2[1] = 1;

    
    writeBuffer();
    simulatedTod += secsPerScaler;
}
/**
 * emitEvent
 *    * If the buffer is empty, initialize the header.
 *    * If adding an event to the buffer would fill the buffer flush it.
 *    * Add an event to the buffer, adjust the header fields and the book keeping
 *      pointers.
 */
static void emitEvent()
{
    // If the buffer is empty initialze the header this is done for the
    // first time an event buffer is being formatted after another buffer type.
    
    if (pBody == reinterpret_cast<void*>(pEventCursor)) {
        initPhysHeader();
    }
    // If adding an event to the buffer will over fill flush
    // ..that will also re-init the event header.
    
    uint16_t newNwds = pHeader->nwds + sizeof(struct event)/sizeof(uint16_t);
    if (newNwds >= sizeof(buffer)/sizeof(uint16_t)) {
        flushEvents();
        newNwds = pHeader->nwds + sizeof(struct event)/sizeof(uint16_t);
    }
    // Add the event to the buffer at pEventCursor and advance the cursor.
    
    pEventCursor->nWds = sizeof(struct event)/sizeof(uint16_t);  // self inclusive size.
    uint64_t stamp = timestamp++;              // Put the timestamp in.
    for (int i = 0; i < 4; i++) {
        pEventCursor->tstamp[i] = stamp & 0xffff;
        stamp = stamp >> 16;
    }
    for (int i =0; i < 16; i++) {             // Put the data in:
        pEventCursor->body[i] = i;
    }
    pEventCursor++;                          //Location of next event.
    pHeader->nevt++;                         // Count the event.
    pHeader->nwds = newNwds;
    
    
}

/**
 *  Entry point
 *    * Parse the parameters
 *    * Main loop to output data.
 * @param argc - Number of args.
 * @param argv - Vector of arguments.
 * @return int   Status of the program.
 */
int
main(int argc, char** argv)
{
    struct gengetopt_args_info args;
    if (cmdline_parser(argc, argv, &args)) {
        std::cerr << "Failed to correctly process command line options\n";
        return EXIT_FAILURE;
    }
    runNumber          = args.run_arg;
    title              = args.title_arg;
    buffersLeft        = args.buffers_arg;
    eventsPerScaler    = args.events_per_scaler_arg;

    
    time(&simulatedTod);
    beginRun();
    while(buffersLeft > 0) {
        for (int i = 0; (buffersLeft > 0) && (i < eventsPerScaler); i++) {
            timestamp++;
            emitEvent();
        }
        std::cerr << "Flushing prior to scaler\n";
        flushEvents();
        
        scaler();
    }
    scaler();
    endRun();
    
    return EXIT_SUCCESS;
}
