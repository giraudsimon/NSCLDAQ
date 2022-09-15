//
//  This program tests the port manager access library.
//  The assumption is that the port manager is already
//  running on the default port.
//  If this is not the case, the test will fail.
//    
#include <config.h>
#include "CPortManager.h"
#include "CPortManagerException.h"
#include <iostream>
#include <vector>



using namespace std;

/*
   test1: 
     Construct a port manager on port 1234 (wrong port) for localhost.
     Attempt to allocate a port. Should get a ConnectionFailed.
*/
void
test1()
{
  CPortManager pm("localhost", 1234);
  bool threw=false;
  cerr << "test1 ...";
  try { 
    int port = pm.allocatePort("TestApp");
    cerr << " no exception thrown";
  }
  catch (CPortManagerException& ex) {
    if((CPortManagerException::Reason)(ex.ReasonCode()) == 
       CPortManagerException::ConnectionFailed) {
      threw = true;
    } else {
      cerr << " caught right exception, wrong reason";
      cerr << ex.ReasonCodeToText(ex.ReasonCode());
    }
  }
  catch(...) {
    cerr << "Wrong exception type";
  }
  if (threw) {
    cerr << "passesd\n";
  } else {
    cerr << " failed";
  }
}

/*
   Test port allocation from a good port..
  We can't pre-determine the port we should
  get  we just require no exception.
*/
void
test2()
{
  bool threw = false;
  cerr << "test2...";
  CPortManager pm ("localhost");
  try {
    int port = pm.allocatePort("testapp");
    cerr << "Got: " << port;
  }
  catch (CPortManagerException& e) {
    cerr << "CPortManagerException" << e;
    threw = true;
  }
  catch(...) {
    cerr << "other exception";
    threw = true;
  }
  if (threw) {
    cerr << "failed\n";
  } else {
    cerr << "passed\n";
  }

}
/*
   Get two ports.. they should be different.
*/
void
test3()
{
  cerr << "test3...";

  CPortManager pm1 ("localhost");
  CPortManager pm2 ("localhost");     // Rust one only allows one port per conn
  int port1 = pm1.allocatePort("test");
  int port2 = pm2.allocatePort("test2");

  if(port1 != port2) {
    cerr << "passed\n";
  }
  else {
    cerr << "Ports the same: " << port1 << " " << port2
	 << " failed\n";
  }
}

/*  
   List the ports in use... should be 3 in use.
*/
void
test4()
{
  cerr << "test4...";
  CPortManager pm("localhost");
  vector<CPortManager::portInfo> info;
  try {
    info = pm.getPortUsage();
  } 
  catch (CPortManagerException e) {
    cerr << "Caught CPortmanagerException: " << e;
    cerr << "failed\n";
    return;
  }
  catch (...) {
    cerr << "Caught some other exception failed\n";
    return;
  }

  /* > 3 because on a production system port man may already have
     connections.
     TODO: Analyze the actual results looking for the ports >I< created.
  */
  
  if(info.size() >= 3) {
    cerr << "passed\n";
  }
  else {
    cerr << "Incorrect # of port information items: " 
	 << info.size() << " failed\n";
  }
}

int main()
{
  cerr << "Testing the port manager C++ interface\n";
  test1();
  test2();
  test3();
 // test4();  // Can't really run this because the port manager may have prio attachments.
}

