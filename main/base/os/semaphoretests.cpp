
// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CSemaphore.h"
#include <thread>
#include <chrono>

#include <unistd.h>
#include <sys/wait.h>




class CPosixSemaphoreTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CPosixSemaphoreTests);
  CPPUNIT_TEST(wait_0);
  CPPUNIT_TEST(cleanup_0);
  CPPUNIT_TEST(getCount_0);
  CPPUNIT_TEST_SUITE_END();


private:
  static const char* shmName;
public:
  void setUp() {
  }
  void tearDown() {
  }
protected:

  // The following test attempts to ensure that the basic
  // mechanics of the semaphore work correctly.
  // It forks the process so that there is a child to do
  // interprocess synchronization. We use two different
  // semaphores. The first semaphore (named "startUp") is used to synchronize the start
  // of the child process. The semaphore is initialized with a
  // count of 0, so that when the parent calls wait(), it will block
  // until the child calls post(). This ensures that the two
  // processes are running before a second semaphore test is begun.
  // The second semaphore test concerns the semaphore named "sem".
  // This semaphore is initialized with a count of 1 and then the child
  // waits on it before posting to the startUp semaphore. This means that
  // any further semaphore wait by the parent will cause it to block.
  // Rather than doing a full wait, the parent calls tryWait, which should
  // return false if the process would have blocked. If this does in
  // in fact return false, then all is working as it should, so we
  // check that this fails.
   void wait_0() {

       DAQ::OS::CPosixSemaphore startUp("start", 0);


       int pid = fork();

       if (pid < 0) {
           CPPUNIT_FAIL("Failed to fork");
       }

       if (pid == 0) {
           // child

//           std::cout << "child running" << std::endl;

           DAQ::OS::CPosixSemaphore sem("test", 1);
           sem.wait();

           // parent is now allowed to wake up.
           startUp.post();
//           std::cout << "child signalling parent" << std::endl;

           std::this_thread::sleep_for(std::chrono::seconds(1));

           sem.post();

           exit(EXIT_SUCCESS);

       } else {

           // parent
           DAQ::OS::CPosixSemaphore sem("test", 1);

//           std::cout << "parent waiting on child" << std::endl;
           startUp.wait();
//           std::cout << "parent done waiting on child" << std::endl;

           // see if we will acquire the lock or not
           bool success = sem.tryWait();

//           std::cout << "parent waiting on child completion" << std::endl;
           // wait on the child process to exit
           int status;
           int waitStatus = waitpid(pid, &status, 0);
           if (waitStatus < 0) {
               std::string msg("waitpid returned error status ");
               msg += std::to_string(waitStatus);
               CPPUNIT_FAIL(msg.c_str());
           }
//           std::cout << "done waiting" << std::endl;

           EQMSG("wait would have blocked", false, success);

           if (success) sem.post();
       }

   }

   // Test that no shared memory is left hanging around after the
   // the semaphore is no longer being used.
   void cleanup_0() {

       // this test may not be fully portable... I don't know how else to check it
       // though.
       std::ifstream file("/dev/shm/sem.___test");
       if (! file.is_open() ) {
           std::remove("/dev/shm/sem.___test");
       } else {
           file.close();
       }

       {
           DAQ::OS::CPosixSemaphore sem("/___test", 1);
       }

       file.open("/dev/shm/sem.___test");

       EQMSG("semaphore shmem file open should fail when not in use", false, file.is_open());
   }


   // Test that we can effectively retrieve the count of the semaphore.
   void getCount_0() {

       DAQ::OS::CPosixSemaphore sem("/____test", 23);
       EQMSG("count", 23, sem.getCount());
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CPosixSemaphoreTests);
