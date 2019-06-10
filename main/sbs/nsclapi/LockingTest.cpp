#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CVMEInterface.h>

#include <thread>
#include <iostream>
#include <string>
#include <chrono>
#include <algorithm>

#include <poll.h>
#include <time.h>

using namespace std;

// Simple RAII technique to make sure that we never fail
// to unlock the vme interface.
class ScopedVMELock {
  public:
  ScopedVMELock() {
    CVMEInterface::Lock();
  }

  ~ScopedVMELock() {
    CVMEInterface::Unlock();
  }
};

/*! \brief A set of tests to ensure that the CVMEInterface works as expected.
 *
 */
class LockingTest : public CppUnit::TestFixture {
  public:
  CPPUNIT_TEST_SUITE(LockingTest);
  CPPUNIT_TEST( tryLock_0 );
  CPPUNIT_TEST( tryLock_1 );
  CPPUNIT_TEST( tryLock_2 );
  CPPUNIT_TEST_SUITE_END();


  private:
  public:
  void setUp() {
  }
  void tearDown() {
  }
  protected:

  // this is a behavioral test. It merely record the default functionality that
  // has been tested to work.
  void tryLock_0 ()
  {
    //
    ScopedVMELock lock;

    // Because we are never trying to access the locked variable from more
    // than 1 thread at a time (note the join), we don't have to worry
    // about synchronization techniques
    bool locked;
    auto task = [&locked](){ 
      locked = CVMEInterface::TryLock(0); // timeout = 0 sec
    };

    // launch thread and let it finish
    std::thread t1(task);
    t1.join();

    EQMSG("TryLock should return false if it was unable to lock mutex",
        false, locked);
  }


  void tryLock_1 ()
  {
      bool locked = CVMEInterface::TryLock(0); // timeout = 0 sec

      CVMEInterface::Unlock();
      EQMSG("TryLock should return true if it was able to lock mutex",
            true, locked);
  }

void tryLock_2()
{
  using namespace std::chrono;
  // acquire the VMEInterface::Lock() in the current thread
  ScopedVMELock lock;

    // Because we are never trying to access the locked variable from more
    // than 1 thread at a time (note the join), we don't have to worry
    // about synchronization techniques
    bool locked;
    time_point<high_resolution_clock> begin, end;
    auto task = [&locked, &begin, &end](){ 
//      std::cout << "Inside thread" << std::endl;
      begin = high_resolution_clock::now();
      locked = CVMEInterface::TryLock(1); // timeout = 1 sec
      end = high_resolution_clock::now();
    };

//    std::cout << "Before thread" << std::endl;
    // launch thread and let it finish
    std::thread t1(task);
    t1.join();
//    std::cout << "After thread" << std::endl;

    double waitTime = duration<double>(end-begin).count();

    // we will say that a test that waited till within 1 ms of the 
    // complete time is sufficient.
    
    double resolution = getTimeoutResolution();

    // be very lenient with the time. we are measuring timeouts rather than
    // sleeps. The system just needs to wait for roughly the amount of time
    // that we are telliong it to timeout. The timeout is specified as 
    // seconds, and the clock resolution is 1-10 milliseconds on Linux 
    // machines. I don't care if we timeout after 980 ms if we told it to time
    // out after 1000 seconds.  The difference is not noticable to the user.
    double expectTime = 1.0 - 2*resolution;
    std::string expected ("Actual Time >= ");
    expected += std::to_string(expectTime);

    std::string actual = expected;
    if (waitTime < expectTime) {
      actual = std::string("Actual Time = ");
      actual += std::to_string(waitTime) + " sec";
    }

    EQMSG("TryLock should return false if it was unable to lock mutex",
        false, locked);

    EQMSG("TryLock should wait expected amount of time before failing",
           expected, actual);
}

double getTimeoutResolution() {
  using namespace std::chrono;

  auto start = high_resolution_clock::now();
  // wait for 1 millisecond
  poll(nullptr, 0, 1);
  auto stop = high_resolution_clock::now();
  double poll_resolution = duration<double>(stop-start).count();
//  std::cout << "poll resolution = " << poll_resolution << std::endl;

  struct timespec spec;
  clock_getres(CLOCK_REALTIME, &spec);
  double getres_resolution = spec.tv_sec + spec.tv_nsec*1.0e-9;
//  std::cout << "CLOCK_REALTIME resolution = " << getres_resolution << "(" << spec.tv_sec << "," << spec.tv_nsec << ")" << std::endl;

  clock_getres(CLOCK_MONOTONIC, &spec);
  double mono_resolution = spec.tv_sec + spec.tv_nsec*1.0e-9;
//  std::cout << "CLOCK_MONOTONIC resolution = " << mono_resolution << "(" << spec.tv_sec << "," << spec.tv_nsec << ")" << std::endl;
  return std::max(
            std::max(
                std::max(poll_resolution, getres_resolution), mono_resolution), 0.01);
}




};

CPPUNIT_TEST_SUITE_REGISTRATION(LockingTest);
