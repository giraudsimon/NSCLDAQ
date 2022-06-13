// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <string.h>
#include <sys/wait.h>
#include <CRingBuffer.h>
#include <time.h>
#include <pwd.h>

#include <vector>

#include <DataFormat.h>
#include <CRingStateChangeItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <stdlib.h>
#include <os.h>
#include <assert.h>

#include <DataFormat.h>


using namespace std;

pid_t childpid;


static void 
readItem(int fd, void* pBuffer)
{
  char* p = reinterpret_cast<char*>(pBuffer);

  // Read the header:

  size_t bytes = read(fd, p, sizeof(RingItemHeader));
  EQ(sizeof(RingItemHeader), bytes);
  pRingItemHeader h = reinterpret_cast<pRingItemHeader>(p);
  p += bytes;
  size_t remaining = h->s_size - sizeof(RingItemHeader);
  bytes = read(fd, p, remaining);

  EQ(remaining, bytes);

}



// Static utility program that will run a specified command
// with stdout and stderr -> a pipe.  The read end of the pipe is returned.
//

static int spawn(const char* command) 
{
  int pipes[2];
  if (pipe(pipes) == -1) {
    perror("Could not open pipes");
    throw "could not open pipes";
  }

  // I can never keep these straight so:

  int readPipe = pipes[0];
  int writePipe= pipes[1];

  if ((childpid = fork()) != 0) {
    // parent...will read from the pipe:

    sleep(1);			// Let child start.
    close(writePipe);
    return readPipe;

  }
  else {
    // child...
    setsid();

    close(readPipe);


    // Set the write pipe as our stdout/stderr:

    dup2(writePipe, STDOUT_FILENO);
    close(writePipe);

    int status = system(command);
    assert(status != -1);
    
    close(STDOUT_FILENO);

    exit(0);

  }
  
}

static void textItem(CRingBuffer& prod, int fd, bool check = true)
{
  vector<string>  items;
  items.push_back("String 1");
  items.push_back("String 2");
  items.push_back("The last string");

  CRingTextItem i(PACKET_TYPES, items);
  i.commitToRing(prod);
  usleep(1000);

  if (check) {
    char buffer[2048];
    readItem(fd, buffer);
    pTextItem e = reinterpret_cast<pTextItem>(buffer);
    pTextItemBody pb =
      reinterpret_cast<pTextItemBody>(bodyPointer(reinterpret_cast<pRingItem>(e)));
    EQ(uint16_t(PACKET_TYPES), itemType(reinterpret_cast<pRingItem>(e)));
    EQ((uint32_t)3,  pb->s_stringCount);

    char* p = &(pb->s_strings[0]);  // For reasons not clear to me this fails otherwise.
    EQ(strlen("String 1"), strlen(p));
    string s1(p);
    p += s1.size() + 1;
    EQ(strlen("String 2"), strlen(p));
    string s2(p);
    p += s2.size() +1;
    EQ(strlen("The last string"), strlen(p));
    string s3(p);

    EQ(string("String 1"), s1);
    EQ(string("String 2"), s2);
    EQ(string("The last string"), s3);
  }
}

static void scaler(CRingBuffer& prod, int fd, bool check=true)
{
  vector<uint32_t> scalers;
  for (int i=0; i < 32; i++) {
    scalers.push_back(i);
  }
  CRingScalerItem i(0, 10, time_t(NULL), scalers);
  i.commitToRing(prod);

  if (check) {
    char buffer[1024];
    readItem(fd, buffer);

    pScalerItem e = reinterpret_cast<pScalerItem>(buffer);
    pScalerItemBody pb = static_cast<pScalerItemBody>(
      bodyPointer(reinterpret_cast<pRingItem>(e))
    );
    EQ(uint16_t(PERIODIC_SCALERS), itemType(reinterpret_cast<pRingItem>(e)));
    EQ((uint32_t)0,         pb->s_intervalStartOffset);
    EQ((uint32_t)10,        pb->s_intervalEndOffset);
    EQ((uint32_t)32,        pb->s_scalerCount);
    for (uint32_t i =0; i < 32; i++) {
      EQ(i, pb->s_scalers[i]);
    }
  }

}

static void eventCount(CRingBuffer& prod, int fd, int count, bool check=true)
{
  CRingPhysicsEventCountItem i(count, 12);
  i.commitToRing(prod);

  if (check) {
    char buffer[1024];
    readItem(fd, buffer);
    pPhysicsEventCountItem e = reinterpret_cast<pPhysicsEventCountItem>(buffer);
    pPhysicsEventCountItemBody pb = static_cast<pPhysicsEventCountItemBody>(
      bodyPointer(reinterpret_cast<pRingItem>(e))
    );
    EQ(
       uint16_t(PHYSICS_EVENT_COUNT),
       itemType(reinterpret_cast<pRingItem>(e))
    );
    EQ((uint32_t)12, pb->s_timeOffset);
    EQ((uint64_t)count, pb->s_eventCount);
  }
}


static void event(CRingBuffer& prod, int fd, bool check=true)
{
  CRingItem i(PHYSICS_EVENT);
  uint16_t* p = reinterpret_cast<uint16_t*>(i.getBodyPointer());
  *p++        = 11;
  for (int i =0; i < 10; i++) {
    *p++ = i;
  }
  i.setBodyCursor(p);
  i.commitToRing(prod);

  if (check) {
    char buffer[1024];
    readItem(fd, buffer);
    pPhysicsEventItem e = reinterpret_cast<pPhysicsEventItem>(buffer);
    uint16_t* pB   = static_cast<uint16_t*>(
        bodyPointer(reinterpret_cast<pRingItem>(e))
    );
    EQ(uint16_t(PHYSICS_EVENT), itemType(reinterpret_cast<pRingItem>(e)));
    EQ((uint16_t)11, pB[0]);
    for (int i = 0; i < 10; i++) {
      EQ((uint16_t)i, pB[i+1]);
    }
  }


}


static void beginRun(CRingBuffer& prod, int fd,  bool check = true)
{
  CRingStateChangeItem i(BEGIN_RUN, 1234, 0, time(NULL), "This is a title");
  i.commitToRing(prod);
  
  // Should now be able to read the item from the pipe and it should match
  // the item we put in.
  
  
  char buffer[1024];
 
  if (check) {
    // We should be able to get what we put in:
    
    readItem(fd, buffer);
    pStateChangeItem item = reinterpret_cast<pStateChangeItem>(buffer);
    pStateChangeItemBody  pb = static_cast<pStateChangeItemBody>(
      bodyPointer(reinterpret_cast<pRingItem>(item)));
    EQ(
      uint16_t(BEGIN_RUN),
      itemType(reinterpret_cast<pRingItem>(item))
    );
    EQ((uint32_t)1234,      pb->s_runNumber);
    EQ((uint32_t)0,         pb->s_timeOffset);
    EQ(string("This is a title"), string(pb->s_title));
  }
}

static void pauseRun(CRingBuffer& prod, int fd, bool check=true)
{
  CRingStateChangeItem i(PAUSE_RUN, 1234, 15, time(NULL), "This is a title");
  i.commitToRing(prod);
  
  // Should now be able to read the item from the pipe and it should match
  // the item we put in.
  
  
  char buffer[1024];
 
  if (check) {
    // We should be able to get what we put in:
    
    readItem(fd, buffer);
    pStateChangeItem item = reinterpret_cast<pStateChangeItem>(buffer);
    pStateChangeItemBody pb = static_cast<pStateChangeItemBody>(
      bodyPointer(reinterpret_cast<pRingItem>(item))
    );
    EQ(
       uint16_t(PAUSE_RUN),
       itemType(reinterpret_cast<pRingItem>(item))
    );
    EQ((uint32_t)1234,      pb->s_runNumber);
    EQ((uint32_t)15,         pb->s_timeOffset);
    EQ(string("This is a title"), string(pb->s_title));
  }
}

static void resumeRun(CRingBuffer& prod, int fd, bool check = true) 
{
  CRingStateChangeItem i(RESUME_RUN, 1234, 15, time(NULL), "This is a title");
  i.commitToRing(prod);
  
  // Should now be able to read the item from the pipe and it should match
  // the item we put in.
  
  
  char buffer[1024];
 
  if (check) {
    // We should be able to get what we put in:
    
    readItem(fd, buffer);
    pStateChangeItem item = reinterpret_cast<pStateChangeItem>(buffer);
    pStateChangeItemBody pb = reinterpret_cast<pStateChangeItemBody>(
      bodyPointer(reinterpret_cast<pRingItem>(item)));
    EQ(uint16_t(RESUME_RUN), itemType(reinterpret_cast<pRingItem>(item)));
    EQ((uint32_t)1234,      pb->s_runNumber);
    EQ((uint32_t)15,         pb->s_timeOffset);
    EQ(string("This is a title"), string(pb->s_title));
  }
}


static void endRun(CRingBuffer& prod, int fd, bool check = true)
{
  CRingStateChangeItem i(END_RUN, 1234, 25, time(NULL), "This is a title");
  i.commitToRing(prod);
  
  // Should now be able to read the item from the pipe and it should match
  // the item we put in.
  
  
  char buffer[1024];
 
  if (check) {
    // We should be able to get what we put in:
    
    readItem(fd, buffer);
    pStateChangeItem item = reinterpret_cast<pStateChangeItem>(buffer);
    pStateChangeItemBody pb = &(item->s_body.u_noBodyHeader.s_body);
    
    EQ(END_RUN, item->s_header.s_type);
    EQ((uint32_t)1234,       pb->s_runNumber);
    EQ((uint32_t)25,         pb->s_timeOffset);
    EQ(string("This is a title"), string(pb->s_title));
  }
}

class rseltests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rseltests);
  CPPUNIT_TEST(all);
  CPPUNIT_TEST(exclude);
  CPPUNIT_TEST(only);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    if (!CRingBuffer::isRing(Os::whoami())) {
      CRingBuffer::create(Os::whoami());
    }
  }
  void tearDown() {
  }
protected:
  void all();
  void exclude();
  void only();
};

CPPUNIT_TEST_SUITE_REGISTRATION(rseltests);


// Starts up the ring selector with no selectivity and
// ensures we can send data to that ring and get it back
// on the pipe connecting us to the child process.
// The preprocessor symobl BINDIR isthe directory in which
// the ringselector was installed.
//
void rseltests::all() 
{
  
  string programName = BINDIR;
  programName       += "/ringselector";

  int fd = spawn(programName.c_str());

  
  try {
    // attach to our ring as a producer.
    
    CRingBuffer prod(Os::whoami(), CRingBuffer::producer);
    
    // Make a begin_run item, commit it.
    
    beginRun(prod, fd);
    for (int i =0; i < 100; i++) {
      event(prod,fd);
    }
    eventCount(prod, fd, 100);
    scaler(prod, fd);
    pauseRun(prod, fd);
    resumeRun(prod, fd);
    //    textItem(prod, fd);
    endRun(prod, fd);
    


  }    
  catch (...) {
    kill(childpid*-1, SIGTERM);
    
    int status;
    wait(&status);
    throw;
  }
  
  
  // Cleanup by killing the child.
  
  kill(childpid*-1, SIGTERM);
  
  int status;
  wait(&status);
  close(fd);

}
// build use the --exclude switch to not accept BEGIN_RUN items.

void rseltests::exclude()
{
  string programName = BINDIR;
  programName       += "/ringselector --exclude=BEGIN_RUN";
  int fd             = spawn(programName.c_str());

  try {
    
    CRingBuffer prod(Os::whoami(), CRingBuffer::producer);
    

    beginRun(prod, fd, false);
    pauseRun(prod, fd);
    endRun(prod,fd);		// Should be the first one back from the program.
  }
  catch (...) {
    kill (childpid*-1, SIGTERM);
    int s;
    wait(&s);
    throw;
  }

  kill (childpid*-1, SIGTERM);
  int s;
  wait(&s);
  close(fd);
}

// Build using the --accept switch...
void rseltests::only()
{
  string programName = BINDIR;
  programName       += "/ringselector --accept=BEGIN_RUN"; // only begin runs.
  int  fd            = spawn(programName.c_str());
  
  try {
    CRingBuffer prod(Os::whoami(), CRingBuffer::producer);

    beginRun(prod,fd);		// Should be fine.
    eventCount(prod, fd, 100, false);
    scaler(prod, fd, false);
    pauseRun(prod, fd, false);
    resumeRun(prod, fd, false);
    beginRun(prod,fd);

  }
  catch (...) {
    kill (childpid*-1, SIGTERM);
    int s;
    wait(&s);
    throw;
  }

  kill (childpid*-1, SIGTERM);
  int s;
  wait(&s);
  close(fd);
}
// don't know how to test for sampling.

