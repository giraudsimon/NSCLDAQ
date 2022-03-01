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

/** @file:  integrationtests.cpp
 *  @brief: Tests for glom as a whole.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <DataFormat.h>
#include <time.h>
#include <string.h>
#include <fragment.h>



static  const char* argv[] = {
    "./glom", "--dt=100", "--timestamp-policy=earliest", "--sourceid=10",
    "--maxfragments=2", nullptr
};
static const char* glom="./glom";

class integrationtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(integrationtest);
    CPPUNIT_TEST(nothing_1);
    CPPUNIT_TEST(begin_1);
    CPPUNIT_TEST(scaler_1);
    CPPUNIT_TEST(text_1);
    
    CPPUNIT_TEST(event_1);
    CPPUNIT_TEST(event_2);
    CPPUNIT_TEST(event_3);
    CPPUNIT_TEST_SUITE_END();

protected:
    void nothing_1();
    void begin_1();
    void scaler_1();
    void text_1();
    void event_1();
    void event_2();
    void event_3();
private:
    pid_t m_glomPid;
    int m_stdinpipe[2];         // glom reads 0, we write 1
    int m_stdoutpipe[2];        //glom writes 1 we read 0.
public:
    void setUp() {
        m_glomPid = startGlom();
    }
    void tearDown() {
        close(m_stdinpipe[1]);         // This EOF will klll off glom.
        close(m_stdoutpipe[0]);
        if (m_glomPid > 0) {
            int exitStatus;
            EQ(waitpid(m_glomPid, &exitStatus, 0), m_glomPid);
        }
            
    }
private:
#pragma packed(push, 1)
struct Scaler {
    RingItemHeader  s_header;
    BodyHeader      s_bodyHeader;
    ScalerItemBody  s_body;
    uint32_t        scalers[32];
};
#pragma packed(pop)
#pragma    pack(push, 1)
struct Text {
    RingItemHeader s_header;
    BodyHeader     s_bodyHeader;
    TextItemBody   s_body;
    char           s_strings[500];
};
#pragma    pack (pop)
#pragma pack(push, 1)
struct Event {
    RingItemHeader s_header;
    BodyHeader     s_bodyHeader;
    uint16_t       s_payload[100];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ChangeState {
    RingItemHeader s_header;
    BodyHeader     s_bodyHeader;
    StateChangeItemBody s_body;
};
#pragma pack(pop)
#pragma pack(push, 1)
struct BuiltEvent {                   // Built event looks like this:
    RingItemHeader s_header;
    BodyHeader     s_bodyHeader;
    uint32_t       s_builtBytes;
    uint8_t        s_payload[1000];    // Just big.
} readEvent;
#pragma pack(pop)
#pragma pack(push, 1)
struct EvtCount {
    RingItemHeader s_header;
    BodyHeader     s_bodyHeader;
    PhysicsEventCountItemBody s_body;
};
#pragma pack(pop)

private:
    pid_t startGlom();
    ssize_t writeGlom(const void* pBuffer, size_t nBytes);
    ssize_t readGlom(void* pBuffer, size_t nBytes);
    ssize_t rdItem(void* pItem);
    
};
// Write glom - must succeed or fail on one shot:

ssize_t
integrationtest::writeGlom(const void* pBuffer, size_t nBytes)
{
    return write(m_stdinpipe[1], pBuffer, nBytes);
}
ssize_t
integrationtest::readGlom(void* pBuffer, size_t nBytes)
{
    return read(m_stdoutpipe[0], pBuffer, nBytes);
}
// Read a ring item
ssize_t
integrationtest::rdItem(void* pBuffer)
{
    pRingItemHeader p =reinterpret_cast<pRingItemHeader>(pBuffer);
    ssize_t hdrSize = readGlom(p, sizeof(RingItemHeader));
    if (hdrSize <= 0) return hdrSize;
    ssize_t bodySize = readGlom(p+1, p->s_size - sizeof(RingItemHeader));
    if (bodySize <= 0) return bodySize;
    return p->s_size;
}


/**
 * startGlom
 *    Starts the glom program off with the arguments in argv:
 * @return pid_t - the PID of glom.
 */
pid_t
integrationtest::startGlom()
{
    ASSERT(pipe2(m_stdinpipe, O_DIRECT) == 0);
    ASSERT(pipe2(m_stdoutpipe, 0) == 0);
    pid_t pid = fork();
    ASSERT(pid >= 0);
    if (pid > 0) {                  // parent
        close(m_stdinpipe[0]);
        close(m_stdoutpipe[1]);
        return pid;
    } else {                       // child
        // m_stdinpipe[0] is our stdin.

        
        close(STDIN_FILENO);
        close(m_stdinpipe[1]);
        int status = dup2(m_stdinpipe[0], STDIN_FILENO);
        if (status == -1) {
            perror("Glom dup of pipe[0] -> stdin failed");
            exit(EXIT_FAILURE);
        }
        // stdoutpipe[1] becomes stdout:
        
        close(STDOUT_FILENO);
        close(m_stdoutpipe[0]);
        status = dup2(m_stdoutpipe[1], STDOUT_FILENO);
        if (status == -1) {
            perror("Glom dup of pipe[1] -> stdout failed");
            exit(EXIT_FAILURE);
        }
        // Exec glom.
        
        status = execv(glom, const_cast<char**>(argv));
        if (status == -1) {
            perror("Could not exec glom");
        }
    }
    return pid;
}

CPPUNIT_TEST_SUITE_REGISTRATION(integrationtest);

// closing Gloms input makes glom exit sending nothing.
void integrationtest::nothing_1()
{
    close(m_stdinpipe[1]);   // EOF to glom's stin.
    uint8_t buffer[1024];
    ssize_t n = read(m_stdoutpipe[0], buffer, sizeof(buffer));
    if (n < 0) {
        perror("Failed read");
    }
    EQ(sizeof(DataFormat), size_t(n));
    pDataFormat p = reinterpret_cast<pDataFormat>(buffer);
    EQ(uint32_t(sizeof(DataFormat)),p->s_header.s_size);
    EQ(RING_FORMAT, p->s_header.s_type);
    EQ(FORMAT_MAJOR, p->s_majorVersion);
    EQ(FORMAT_MINOR, p->s_minorVersion);
    
    int status;
    EQ(m_glomPid, waitpid(m_glomPid, &status, 0));
    
    m_glomPid = 0;
}
// Sending a begin run at glom should give a format and begin run back.
// We've already confirmed the format comes

void integrationtest::begin_1()
{
    StateChangeItem begin;
    DataFormat      format;
    
    // Should be able to get the data format right away:
    
    ssize_t n = readGlom(&format, sizeof(format));
    EQ(sizeof(format), size_t(n));
    
    // Now format and produce the begin run item....we're going to not
    // say this is a barrier to avoid  glom parameters record:
    
    
    uint32_t itemsize =
        sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(StateChangeItemBody);
    begin.s_header.s_type = BEGIN_RUN;
    begin.s_header.s_size = itemsize;
    begin.s_body.u_hasBodyHeader.s_bodyHeader.s_size = sizeof(BodyHeader);
    begin.s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp = 0;
    begin.s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId = 1; // gets overwritten.
    begin.s_body.u_hasBodyHeader.s_bodyHeader.s_barrier = 0;  // cheat.
    pStateChangeItemBody pBody = &begin.s_body.u_hasBodyHeader.s_body;
    pBody->s_runNumber = 12;
    pBody->s_timeOffset = 0;
    pBody->s_Timestamp = time(nullptr);
    pBody->s_offsetDivisor = 1;
    pBody->s_originalSid = 1;   // Not overwritten.
    strcpy(pBody->s_title, "A title string");
    
    // Need a fragment header for this:
    
    EVB::FragmentHeader h;
    h.s_timestamp = 0;
    h.s_sourceId = 1;
    h.s_barrier = 0;
    h.s_size = itemsize;
    
    // Write the flat fragment.
    
    writeGlom(&h, sizeof(h));
    writeGlom(&begin, itemsize);  // Includes a flush.
    
    uint8_t buffer[1000];
    memset(buffer, 0, sizeof(buffer));
    n = readGlom(buffer, sizeof(buffer));
    EQ(itemsize, uint32_t(n));
    
    // The result should be a BEGIN_RUN item with the source id rewritten:
    
    pStateChangeItem p = reinterpret_cast<pStateChangeItem>(buffer);
    
    EQ(begin.s_header.s_type, p->s_header.s_type);
    EQ( begin.s_header.s_size, p->s_header.s_size);
    EQ( begin.s_body.u_hasBodyHeader.s_bodyHeader.s_size, p->s_body.u_hasBodyHeader.s_bodyHeader.s_size );
    EQ(begin.s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp, p->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), p->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId);
    EQ(begin.s_body.u_hasBodyHeader.s_bodyHeader.s_barrier, p->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);
    
    auto pB = &(p->s_body.u_hasBodyHeader.s_body);
    EQ(pBody->s_runNumber, pB->s_runNumber);
    EQ(pBody->s_timeOffset, pB->s_timeOffset);
    EQ(pBody->s_Timestamp, pB->s_Timestamp);
    EQ(pBody->s_offsetDivisor, pB->s_offsetDivisor);
    EQ(pBody->s_originalSid, pB->s_originalSid);
    EQ(0, strcmp(pBody->s_title, pB->s_title));

    
}
// Sending a scaler item immediately gives me a scaler item back:
// Note that oob also sends  a sclaer request.
void integrationtest::scaler_1()
{
    // This declaration is a bit odd - it provides storage for the
    // scalers with body header as well as 32 actual scalers.
    // Doing it in this way means we don't have the variable sized
    // union that ScalerItem has.

    Scaler s;                                // This is what we'll send:
    s.s_header.s_type = PERIODIC_SCALERS;
    s.s_header.s_size = sizeof(struct Scaler);
    s.s_bodyHeader.s_size = sizeof(BodyHeader);
    s.s_bodyHeader.s_timestamp = 0x123456789;
    s.s_bodyHeader.s_sourceId  = 1;          // Will be edited to 10.
    s.s_bodyHeader.s_barrier   = 0;
    s.s_body.s_intervalStartOffset = 0;     // first 2 seconds of the run.
    s.s_body.s_intervalEndOffset = 2;
    s.s_body.s_timestamp = time(nullptr);
    s.s_body.s_intervalDivisor = 1;
    s.s_body.s_scalerCount = 32;
    s.s_body.s_isIncremental=1;
    s.s_body.s_originalSid = s.s_bodyHeader.s_sourceId;
    for (int i = 0; i < 32; i++) {
        s.scalers[i] = i*10;
    }
    EVB::FragmentHeader h;
    h.s_timestamp = 0x123456789;
    h.s_size      = sizeof(struct Scaler);
    h.s_sourceId  = s.s_bodyHeader.s_sourceId;
    h.s_barrier   = 0;
    
    // send glom the fragment header and the scaler item:
    
    writeGlom(&h, sizeof(h));
    writeGlom(&s, sizeof(s));
    
    // Read the result, which should be a format item and a scaler item:
    
    DataFormat      format;
    ssize_t n = readGlom(&format, sizeof(format));
    EQ(sizeof(format), size_t(n));
    
    //
    
    uint8_t buffer[1000];
    memset(buffer, 0, sizeof(buffer));    // No coinidences
    n = readGlom(buffer, sizeof(buffer));
    EQ(sizeof(Scaler), size_t(n));
    
    // The only things that should differ are the sourceid in the body header:
    
    Scaler* p = reinterpret_cast<Scaler*>(buffer);
    EQ(0, memcmp(&(s.s_header), &(p->s_header), sizeof(RingItemHeader)));
    
    EQ(s.s_bodyHeader.s_size, p->s_bodyHeader.s_size);
    EQ(uint32_t(10), p->s_bodyHeader.s_sourceId);
    EQ(s.s_bodyHeader.s_timestamp, p->s_bodyHeader.s_timestamp);
    EQ(s.s_bodyHeader.s_barrier, p->s_bodyHeader.s_barrier);
    
    EQ(0, memcmp(&s.s_body, &p->s_body, sizeof(ScalerItemBody)));
    EQ(0, memcmp(s.scalers, p->scalers, 32*sizeof(uint32_t)));
}
// Text items also should come out right away
// We do the same struct trick as for Scalers to make handing this easier.

void integrationtest::text_1()
{

    const char* const strings[] = {
        "One string", "Two String", "Three string", "more",
        "red string", "blue string", "that's all for now"
    };
    size_t nStrings = sizeof(strings)/sizeof(char*);
    
    // We need to figure out how much storage the strings really require so that
    // we know how to set s_header.s_size:
    size_t strsize = 0;
    for (int i = 0;i < nStrings; i++) {
        strsize += strlen(strings[i]) + 1;      // (+1 for null terminator).
    }
    ASSERT(strsize <= 500);                   // Else we blow up filling the strings.
    
    // Now fill in the fragment body:
    
    Text frag;
    frag.s_header.s_type = PACKET_TYPES;    // Gotta chose one.
    frag.s_header.s_size = sizeof(Text) + strsize;
    
    frag.s_bodyHeader.s_timestamp = 0x987654321;
    frag.s_bodyHeader.s_sourceId = 1;
    frag.s_bodyHeader.s_barrier   = 0;
    frag.s_bodyHeader.s_size    = sizeof(BodyHeader);
    
    frag.s_body.s_timeOffset = 100;
    frag.s_body.s_timestamp = time(nullptr);
    frag.s_body.s_stringCount      = nStrings;
    frag.s_body.s_offsetDivisor    = 1;
    frag.s_body.s_originalSid = frag.s_bodyHeader.s_sourceId;
    
    char* p = frag.s_strings;
    for (int i =0; i < nStrings; i++) {
        strcpy(p, strings[i]);
        p += strlen(strings[i]) + 1;
    }
    
    // Fill in the fragment header:
    
    EVB::FragmentHeader hdr;
    hdr.s_timestamp = frag.s_bodyHeader.s_timestamp;
    hdr.s_sourceId  = frag.s_bodyHeader.s_sourceId;
    hdr.s_barrier   = frag.s_bodyHeader.s_barrier;
    hdr.s_size      = frag.s_header.s_size;
    
    
    // Submit the fragment to glom:
    
    writeGlom(&hdr, sizeof(hdr));
    writeGlom(&frag, frag.s_header.s_size);
    
    // Ignore the format item.
    
    DataFormat      format;
    ssize_t n = readGlom(&format, sizeof(format));
    EQ(sizeof(format), size_t(n));
        
    // Get the string item back and compare  - again only the body header
    // sourceid should be different - it gets rewritten to 10.
    
    uint8_t buffer[1000];
    memset(buffer, 0, sizeof(buffer));    // No coinidences
    n = readGlom(buffer, sizeof(buffer));
    EQ(n, ssize_t(frag.s_header.s_size));
    
    Text* pf = reinterpret_cast<Text*>(buffer);
    
    // Only the body header source id does not match.
    
    EQ(0, memcmp(&frag.s_header, &pf->s_header, sizeof(RingItemHeader)));
    
    EQ(frag.s_bodyHeader.s_timestamp, pf->s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), pf->s_bodyHeader.s_sourceId);
    EQ(frag.s_bodyHeader.s_barrier, pf->s_bodyHeader.s_barrier);
    EQ(frag.s_bodyHeader.s_size, pf->s_bodyHeader.s_size);
    
    EQ(0, memcmp(&frag.s_body, &pf->s_body, sizeof(TextItemBody) + strsize));
    
    
}
// Put a single fragment in followed by a statechange (end run)
// I should get format, the event in built format and the state change item.

void integrationtest::event_1()
{
    
    Event event;
    ChangeState end;
    
    // Fill in and send the event.
    
    event.s_header.s_size = sizeof(Event);
    event.s_header.s_type = PHYSICS_EVENT;
    event.s_bodyHeader.s_timestamp = 0x1243512345;
    event.s_bodyHeader.s_sourceId  = 1;
    event.s_bodyHeader.s_barrier  = 0;
    event.s_bodyHeader.s_size = sizeof(BodyHeader);
    for (int i =0; i < 100; i++) {
        event.s_payload[i] = i;
    }
    EVB::FragmentHeader evhdr;
    evhdr.s_timestamp = event.s_bodyHeader.s_timestamp;
    evhdr.s_sourceId  = event.s_bodyHeader.s_sourceId;
    evhdr.s_barrier   = event.s_bodyHeader.s_barrier;
    evhdr.s_size      = event.s_header.s_size;
    
    writeGlom(&evhdr, sizeof(evhdr));
    writeGlom(&event, event.s_header.s_size);
    
    // Fill in and send the end run.
    
    end.s_header.s_size = sizeof(end);
    end.s_header.s_type = END_RUN;
    end.s_bodyHeader.s_timestamp = 0x222222222;
    end.s_bodyHeader.s_sourceId  = 1;
    end.s_bodyHeader.s_barrier   = 0;
    end.s_bodyHeader.s_size     = sizeof(BodyHeader);
    end.s_body.s_runNumber = 3;
    end.s_body.s_timeOffset = 200;
    end.s_body.s_Timestamp = time(nullptr);
    end.s_body.s_offsetDivisor = 1;
    end.s_body.s_originalSid = 1;
    memset(end.s_body.s_title, 0, sizeof end.s_body.s_title);
    strcpy(end.s_body.s_title, "Some title neeced to end the run");
    
    //  only need to update the timestap and payload size.
    
    evhdr.s_timestamp = end.s_bodyHeader.s_timestamp;
    evhdr.s_size      = end.s_header.s_size;
    evhdr.s_barrier   = 2;                // Else it's just an oob and does not push out event
    
    writeGlom(&evhdr, sizeof(evhdr));
    writeGlom(&end, sizeof(end));             // Forces the event out.
    
    
    
    // Get and ignore the format
    
    DataFormat format;
    readGlom(&format, sizeof(format));
    
    // Get and check the event -- note since there are 2 items on the output
    // we need to be careful how we do this and read each item by itself.
    // Since the event has been built we need to read the size of the event
    // and then the rest of it:
    

    rdItem(&readEvent);
    
    EQ(
       size_t(readEvent.s_header.s_size),
       sizeof(event) + sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) +
       sizeof(RingItemHeader) + sizeof(uint32_t));
    EQ(PHYSICS_EVENT, readEvent.s_header.s_type);
    
    EQ(uint64_t(0x1243512345), readEvent.s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), readEvent.s_bodyHeader.s_sourceId);
    EQ(uint32_t(0), readEvent.s_bodyHeader.s_barrier);
    
    EQ(sizeof(event) + sizeof(EVB::FragmentHeader) + sizeof(uint32_t),
       size_t(readEvent.s_builtBytes)
    );
    
    EVB::pFragmentHeader pf = reinterpret_cast<EVB::pFragmentHeader>(readEvent.s_payload);
    EQ(uint64_t(0x1243512345), pf->s_timestamp);
    EQ(event.s_header.s_size, pf->s_size);
    EQ(uint32_t(1), pf->s_sourceId);
    EQ(uint32_t(0), pf->s_barrier);
    
    // The remainder shoulid just be a copy of event:
    
    uint8_t* pPayload = reinterpret_cast<uint8_t*>(pf+1);
    EQ(0, memcmp(&event, pPayload, event.s_header.s_size));
    
    // Get and check the end run.
    
    ChangeState readEnd;
    ssize_t sSize = readGlom(&readEnd, sizeof(readEnd));
    EQ(sSize, ssize_t(sizeof(readEnd)));
    
    // The body header has been re-written with 10 as the srcid everything else
    // should be the same as end:
    
    EQ(0, memcmp(&end.s_header, &readEnd.s_header, sizeof(RingItemHeader)));
    
    EQ(end.s_bodyHeader.s_timestamp, readEnd.s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), readEnd.s_bodyHeader.s_sourceId);
    EQ(end.s_bodyHeader.s_barrier, readEnd.s_bodyHeader.s_barrier);
    
    EQ(0, memcmp(&end.s_body, &readEnd.s_body, sizeof(StateChangeItemBody)));
}
// 2 fragment event pushed out by end run.

void integrationtest::event_2()
{
    Event frag1;
    Event frag2;
    ChangeState end;
    
    EVB::FragmentHeader fraghdr;
    
    // Fill this crap in.
    
    frag1.s_header.s_size = sizeof(Event);
    frag1.s_header.s_type = PHYSICS_EVENT;
    frag1.s_bodyHeader.s_timestamp = 0x1243512345;
    frag1.s_bodyHeader.s_sourceId  = 1;
    frag1.s_bodyHeader.s_barrier  = 0;
    frag1.s_bodyHeader.s_size = sizeof(BodyHeader);
    for (int i =0; i < 100; i++) {
        frag1.s_payload[i] = i;
    }
    
    // Just change the payload, sourceid and timestamp between frag1 and 2.
    
    memcpy(&frag2, &frag1, sizeof(Event));
    frag2.s_bodyHeader.s_sourceId = 2;
    frag2.s_bodyHeader.s_timestamp += 50;           // Within the build window.
    for (int i =0; i < 100; i++) {
        frag2.s_payload[i] += 100;
    }
    
    // Set the fragment header and send the header and bodies of the two fragments:
    
    fraghdr.s_size      = frag1.s_header.s_size;
    fraghdr.s_timestamp = frag1.s_bodyHeader.s_timestamp;
    fraghdr.s_sourceId  = frag1.s_bodyHeader.s_sourceId;
    fraghdr.s_barrier   = frag1.s_bodyHeader.s_barrier;
    
    writeGlom(&fraghdr, sizeof(fraghdr));
    writeGlom(&frag1, sizeof(frag1));
    
    fraghdr.s_timestamp = frag2.s_bodyHeader.s_timestamp;
    fraghdr.s_sourceId  = frag2.s_bodyHeader.s_sourceId;
    
    writeGlom(&fraghdr, sizeof(fraghdr));
    writeGlom(&frag2, sizeof(frag2));
    
    // Fill in the end of run, its fragment header and send it as well.
    
    end.s_header.s_type = END_RUN;
    end.s_header.s_size = sizeof(ChangeState);
    
    end.s_bodyHeader.s_size = sizeof(BodyHeader);
    end.s_bodyHeader.s_timestamp = fraghdr.s_timestamp + 100;
    end.s_bodyHeader.s_sourceId  = 1;
    end.s_bodyHeader.s_barrier   = 2;
    
    end.s_body.s_runNumber = 3;
    end.s_body.s_timeOffset = 200;
    end.s_body.s_Timestamp = time(nullptr);
    end.s_body.s_offsetDivisor = 1;
    end.s_body.s_originalSid = 1;
    memset(end.s_body.s_title, 0, sizeof end.s_body.s_title);
    strcpy(end.s_body.s_title, "Some title neeced to end the run");
    
    fraghdr.s_timestamp = end.s_bodyHeader.s_timestamp;
    fraghdr.s_sourceId  = end.s_bodyHeader.s_sourceId;
    fraghdr.s_size      = sizeof(end);
    fraghdr.s_barrier   = end.s_bodyHeader.s_barrier;
    
    writeGlom(&fraghdr, sizeof(fraghdr));
    writeGlom(&end, sizeof(end));
    
    
    
    // What we should get back is a format item,
    
    DataFormat format;
    readGlom(&format, sizeof(format));     // Ignore the format item.

    
    // a 2 fragment event
    
    BuiltEvent event;
    rdItem(&event);
    
    uint32_t payloadSize  = sizeof(uint32_t) + 2*(sizeof(EVB::FragmentHeader) + sizeof(Event));
    uint32_t totalSize    = sizeof(RingItemHeader) + sizeof(BodyHeader) + payloadSize;
    EQ(totalSize, event.s_header.s_size);
    EQ(PHYSICS_EVENT, event.s_header.s_type);
    
    EQ(sizeof(BodyHeader), size_t(event.s_bodyHeader.s_size));
    EQ(frag1.s_bodyHeader.s_timestamp, event.s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), event.s_bodyHeader.s_sourceId);
    EQ(uint32_t(0), event.s_bodyHeader.s_barrier);
    
    EQ(payloadSize, event.s_builtBytes);
    
    //   The two fragments follow:
    
    EVB::pFragmentHeader pFrag = reinterpret_cast<EVB::pFragmentHeader>(event.s_payload);
    EQ(pFrag->s_timestamp , frag1.s_bodyHeader.s_timestamp );
    EQ(pFrag->s_sourceId, frag1.s_bodyHeader.s_sourceId);
    EQ(pFrag->s_barrier, frag1.s_bodyHeader.s_barrier);
    EQ(pFrag->s_size, frag1.s_header.s_size);
    
    uint8_t* p = reinterpret_cast<uint8_t*>(pFrag+1);         // Event.
    EQ(0, memcmp(&frag1, p, frag1.s_header.s_size));          // Should be unscathed.
    
    pFrag = reinterpret_cast<EVB::pFragmentHeader>(p + frag1.s_header.s_size); // 2nd frag.
    EQ(pFrag->s_timestamp, frag2.s_bodyHeader.s_timestamp);
    EQ(pFrag->s_sourceId,  frag2.s_bodyHeader.s_sourceId);
    EQ(pFrag->s_barrier,   frag2.s_bodyHeader.s_barrier);
    EQ(pFrag->s_size,      frag2.s_header.s_size);
    p = reinterpret_cast<uint8_t*>(pFrag+1);
    
    EQ(0, memcmp(&frag2, p, frag2.s_header.s_size));
    
    // and the end run item.
    
    ChangeState readEnd;
    ssize_t n = readGlom(&readEnd, sizeof(readEnd));
    EQ(n, ssize_t(sizeof(readEnd)));
}
// Sending a fragment, a scaler, a fragment and end run should yield (if
// the --dt is made):  a scaler, a built event and an end run.
//
void integrationtest::event_3()
{
    Event frag1;
    Event frag2;
    ChangeState end;
    Scaler  scaler;
    
    EVB::FragmentHeader fraghdr;
    
    // Fill this crap in.
    
    frag1.s_header.s_size = sizeof(Event);
    frag1.s_header.s_type = PHYSICS_EVENT;
    frag1.s_bodyHeader.s_timestamp = 0x1243512345;
    frag1.s_bodyHeader.s_sourceId  = 1;
    frag1.s_bodyHeader.s_barrier  = 0;
    frag1.s_bodyHeader.s_size = sizeof(BodyHeader);
    for (int i =0; i < 100; i++) {
        frag1.s_payload[i] = i;
    }
    
    // Just change the payload, sourceid and timestamp between frag1 and 2.
    
    memcpy(&frag2, &frag1, sizeof(Event));
    frag2.s_bodyHeader.s_sourceId = 2;
    frag2.s_bodyHeader.s_timestamp += 50;           // Within the build window.
    for (int i =0; i < 100; i++) {
        frag2.s_payload[i] += 100;
    }
    
    // Set the fragment header and send the header and bodies of the two fragments:
    
    fraghdr.s_size      = frag1.s_header.s_size;
    fraghdr.s_timestamp = frag1.s_bodyHeader.s_timestamp;
    fraghdr.s_sourceId  = frag1.s_bodyHeader.s_sourceId;
    fraghdr.s_barrier   = frag1.s_bodyHeader.s_barrier;
    
    writeGlom(&fraghdr, sizeof(fraghdr));     // Frag1 written.
    writeGlom(&frag1, sizeof(frag1));
    
    // Fill in and write the scaler (OOB) item
    
    scaler.s_header.s_size = sizeof(scaler);
    scaler.s_header.s_type = PERIODIC_SCALERS;
    
    scaler.s_bodyHeader.s_size = sizeof(BodyHeader);
    scaler.s_bodyHeader.s_timestamp = frag1.s_bodyHeader.s_timestamp;
    scaler.s_bodyHeader.s_sourceId  = 1;
    scaler.s_bodyHeader.s_barrier = 0;
    
    scaler.s_body.s_intervalStartOffset = 10;
    scaler.s_body.s_intervalEndOffset   = 12;
    scaler.s_body.s_timestamp           = time(nullptr);
    scaler.s_body.s_intervalDivisor     = 1;
    scaler.s_body.s_scalerCount = 32;
    scaler.s_body.s_isIncremental = 1;
    scaler.s_body.s_originalSid = scaler.s_bodyHeader.s_sourceId;
    for (int i =0; i < 32; i++) {
        scaler.scalers[i] = 100*i;
    }
    fraghdr.s_size = sizeof(scaler);
    
    writeGlom(&fraghdr, sizeof(fraghdr));
    writeGlom(&scaler, sizeof(scaler));      // OOB fragment.
    
    // FIll in the fragment header and write the second fragment:
    
    fraghdr.s_timestamp = frag2.s_bodyHeader.s_timestamp;
    fraghdr.s_sourceId  = frag2.s_bodyHeader.s_sourceId;
    fraghdr.s_size      = frag2.s_header.s_size;
    
    writeGlom(&fraghdr, sizeof(fraghdr));
    writeGlom(&frag2, sizeof(frag2));             // Second fragment.
    
    
    // Fill in the end of run, its fragment header and send it as well.
    
    end.s_header.s_type = END_RUN;
    end.s_header.s_size = sizeof(ChangeState);
    
    end.s_bodyHeader.s_size = sizeof(BodyHeader);
    end.s_bodyHeader.s_timestamp = fraghdr.s_timestamp + 100;
    end.s_bodyHeader.s_sourceId  = 1;
    end.s_bodyHeader.s_barrier   = 2;
    
    end.s_body.s_runNumber = 3;
    end.s_body.s_timeOffset = 200;
    end.s_body.s_Timestamp = time(nullptr);
    end.s_body.s_offsetDivisor = 1;
    end.s_body.s_originalSid = 1;
    memset(end.s_body.s_title, 0, sizeof end.s_body.s_title);
    strcpy(end.s_body.s_title, "Some title neeced to end the run");
    
    fraghdr.s_timestamp = end.s_bodyHeader.s_timestamp;
    fraghdr.s_sourceId  = end.s_bodyHeader.s_sourceId;
    fraghdr.s_size      = sizeof(end);
    fraghdr.s_barrier   = end.s_bodyHeader.s_barrier;
    
    writeGlom(&fraghdr, sizeof(fraghdr));
    writeGlom(&end, sizeof(end));    // forces out event.
    
    
    // Skip the format item:
    
    DataFormat fmt;
    readGlom(&fmt, sizeof(DataFormat));
    
    // Next should be a scaler item:
    
    Scaler rdScaler;
    ssize_t n = rdItem(&rdScaler);
    EQ(ssize_t(sizeof(Scaler)), n);
    EQ(scaler.s_header.s_size, rdScaler.s_header.s_size);
    EQ(PERIODIC_SCALERS, rdScaler.s_header.s_type);
    
    // Body header source id was re-written -> 10
    
    EQ(scaler.s_bodyHeader.s_size, rdScaler.s_bodyHeader.s_size);
    EQ(scaler.s_bodyHeader.s_timestamp, rdScaler.s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), rdScaler.s_bodyHeader.s_sourceId);
    EQ(scaler.s_bodyHeader.s_barrier, rdScaler.s_bodyHeader.s_barrier);
    
    EQ(0, memcmp(&scaler.s_body, &rdScaler.s_body, sizeof(ScalerItemBody)));
    EQ(0, memcmp(scaler.scalers, rdScaler.scalers, 32*sizeof(uint32_t)));
    
    // Scaler push out an outputEvent count:
    
    EvtCount cnt;
    n = rdItem(&cnt);
    EQ(n, ssize_t(sizeof(cnt)));
    
    // Next should be a 2 fragment event
    
    BuiltEvent rdEvent; 
    uint32_t payloadSize = sizeof(uint32_t) + 2*(sizeof(EVB::FragmentHeader) + sizeof(Event));
    uint32_t totalSize   = payloadSize + sizeof(RingItemHeader) + sizeof(BodyHeader);
    ssize_t esize  = rdItem(&rdEvent);
    EQ(totalSize, uint32_t(esize));
    
    // Contents of the event as this may be one suspect place where we used to have
    // corrupted data.
    
    EQ(totalSize, rdEvent.s_header.s_size);
    EQ(PHYSICS_EVENT, rdEvent.s_header.s_type);
    
    EQ(sizeof(BodyHeader), size_t(rdEvent.s_bodyHeader.s_size));
    EQ(uint64_t(0x1243512345), rdEvent.s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), rdEvent.s_bodyHeader.s_sourceId);
    EQ(uint32_t(0), rdEvent.s_bodyHeader.s_barrier);
    
    EQ(payloadSize, rdEvent.s_builtBytes);
    
    // Fragment 1:
    
    EVB::pFragmentHeader pf = reinterpret_cast<EVB::pFragmentHeader>(rdEvent.s_payload);
    EQ(frag1.s_bodyHeader.s_timestamp, pf->s_timestamp);
    EQ(frag1.s_bodyHeader.s_sourceId, pf->s_sourceId);
    EQ(frag1.s_header.s_size, pf->s_size);
    EQ(frag1.s_bodyHeader.s_barrier, pf->s_barrier);
    uint8_t* pE1 = reinterpret_cast<uint8_t*>(pf+1);
    EQ(0, memcmp(&frag1, pE1, frag1.s_header.s_size));
    
    pf = reinterpret_cast<EVB::pFragmentHeader>(pE1 + frag1.s_header.s_size);
    EQ(frag2.s_bodyHeader.s_timestamp, pf->s_timestamp);
    EQ(frag2.s_bodyHeader.s_sourceId, pf->s_sourceId);
    EQ(frag2.s_header.s_size, pf->s_size);
    EQ(frag2.s_bodyHeader.s_barrier, pf->s_barrier);
    uint8_t* pE2 = reinterpret_cast<uint8_t*>(pf+1);
    EQ(0, memcmp(&frag2, pE2, frag2.s_header.s_size));
    
    // FInally an end of run.
    
    ChangeState rds;
    ssize_t ssize = rdItem(&rds);
    EQ(ssize_t(sizeof(ChangeState)), ssize);
    
    // The source id got changed to 10:
    
    EQ(0, memcmp(&end.s_header, &rds.s_header, sizeof(RingItemHeader)));
    EQ(end.s_bodyHeader.s_size, rds.s_bodyHeader.s_size);
    EQ(end.s_bodyHeader.s_timestamp, rds.s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), rds.s_bodyHeader.s_sourceId);
    EQ(end.s_bodyHeader.s_barrier, rds.s_bodyHeader.s_barrier);
    
    EQ(0, memcmp(&end.s_body, &rds.s_body, sizeof(StateChangeItemBody)));
    
}