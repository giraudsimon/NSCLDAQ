/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <CRingBuffer.h>
#include <DataFormat.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <vector>
#include <stdint.h>
#include <os.h>
#include <time.h>
#include <stdlib.h>

/**
 * @file testData.cpp
 * @brief Provide test scaler data to sclclient via a ring buffer.
 * @author Ron Fox<fox@nscl.msu.edu
 */

/**
 * Usage:
 *    testData ?1|0?
 *
 *    If a parameter is supplied it is 1 to create incremental scalers,
 *    0 to create non-incremental scaler ring items.
 *
 * scaler data is emitted once every few seconds and have these characteristics:
 *     - Couting patters for the scalers (16 of them).
 *     - Time intervals showing 10 'ticks' between scalers.
 *     - Time divisors of 3 indicating everything comes out at 1.333 second
 *       intervals.
 *     - 100 items are emitted bracketed by hard coded state transition
 *       records that show the correct time offsets (0 and 100 with divisors of
 *       3 as well)
 *     - Data go to the 'username' ring.
 */

static bool incremental(true);             // incremental state.
static int  divisor(3);
static int  ticksPerInterval(10);
static int  runNumber(5);
static const char* title = "Test scaler data";

static int runOffset(0);
static int scalerItems(0);

#define SCALERCOUNT 16
static uint32_t scalers[SCALERCOUNT] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/**
 * openRingBuffer
 *   Opens a ring buffer as a producer.
 *   The name of the ring buffer is taken from the username.
 *
 *   @return CRingBuffer*
 */
CRingBuffer*
openRingBuffer()
{
    std::string ringName = Os::whoami();
    
    return new CRingBuffer(ringName, CRingBuffer::producer);
}

/**
 * emitStateChange
 *
 * Emits a state change ring item.
 *
 * @param type - Type of state change to emit.
 * @param pR   - Pointer to the ring buffer.
 */
static void
emitStateChange(int type, CRingBuffer* pR)
{
    time_t stamp = time(NULL);
    CRingStateChangeItem item(0, 2, 0, type, runNumber, runOffset, stamp, title,
                              divisor);
    item.commitToRing(*pR);
}
/**
 * emitScaler
 *   Emit a scaler ring item.  To do this we need to also compute the
 *   value of the scalers as well as the interval end time.
 *
 * @param p  - Pointer to the ring.
 */
static void
emitScaler(CRingBuffer* p)
{
    scalerItems++;
    time_t stamp = time(NULL);
    // Figure out new values for the scalers and end time
    
    int endTime = runOffset + ticksPerInterval;
    std::vector<uint32_t> s;
    for (int i = 0; i < SCALERCOUNT; i++) {
        if (incremental) {
            scalers[i] = i;
        } else {
            scalers[i] += i;
        }
        s.push_back(scalers[i]);
    }
    // Create the ring item and submit it.
    
    CRingScalerItem item(0, scalerItems % 2 + 1, 0, runOffset, endTime, stamp, s, divisor, incremental);
    item.commitToRing(*p);
    
    // Update the run offset:
    
    runOffset = endTime;
}

/**
 * main - program entry point.
 *
 * @param argc - Number of command line parameters.
 * @param argv - Array of pointers to them.
 */
int main(int argc, char** argv)
{
    // Figure out if this is incremental or not
    
    if(argc == 2) {
        if(*argv[1] == '0') {
            incremental = false;
        }
    }
    CRingBuffer* pRing = openRingBuffer();
    
    emitStateChange(BEGIN_RUN, pRing);
    for (int i =0; i < 100; i++) {
        sleep(2);
        emitScaler(pRing);
    }
    
    emitStateChange(END_RUN, pRing);
    
}