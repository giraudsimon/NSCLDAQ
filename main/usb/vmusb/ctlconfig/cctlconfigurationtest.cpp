/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
     Jeromy Tompkins
     NSCL
     Michigan State University
     East Lansing, MI 48824-1321
*/

#include <make_unique.h>
#include <CMxDCReset.h>
#include <CControlModule.h>

#define private public
#define protected public
#include <CCtlConfiguration.h>
#undef protected
#undef private

#include <TCLInterpreter.h>
#include <TCLObjectProcessor.h>
#include <TCLObject.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iterator>

using namespace std;

// Define a test command
class CTestCmd : public CTCLObjectProcessor 
{
  public:
  CTestCmd(CTCLInterpreter& interp) : CTCLObjectProcessor(interp, "test", false)
  {}

  int operator()(CTCLInterpreter&  interp, std::vector<CTCLObject>& objv) {
    return TCL_OK;
  }
};



class CCtlConfigurationTests : public CppUnit::TestFixture {
  public:
    CPPUNIT_TEST_SUITE(CCtlConfigurationTests);
    CPPUNIT_TEST(construct_0);
    CPPUNIT_TEST(addModule_0);
    CPPUNIT_TEST(findModule_0);
    CPPUNIT_TEST(addCommand_0);
    CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construct_0();
  void addModule_0();
  void findModule_0();
  void addCommand_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CCtlConfigurationTests);



void CCtlConfigurationTests::construct_0() {
  CCtlConfiguration config;

  CPPUNIT_ASSERT_EQUAL_MESSAGE( "Initial CCtlConfiguration has 0 modules", 
                                 size_t(0), config.getModules().size());
                                
}



void CCtlConfigurationTests::addModule_0() {

  CCtlConfiguration config;

  // create the control module
  auto pHdwr = make_unique( new CMxDCReset );
  auto pModule =make_unique(new CControlModule( "test", pHdwr.get() ));
  
  // store the location of our module for testing purposes
  CControlModule* pMod = pModule.get();

  config.addModule( pModule.get() );

  // ensure that what we added is the same as what ended up in the 
  // configuration
  CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Adding module successfully registers the control module to ctl configuration",
      pMod, config.getModules().at(0)
      );
}

void CCtlConfigurationTests::findModule_0() {

  CCtlConfiguration config;

  // create the control module
  auto pHdwr = make_unique( new CMxDCReset );
  auto pModule = make_unique( new CControlModule( "test", pHdwr.get() ));
  
  // store the location of our module for testing purposes
  CControlModule* pMod = pModule.get();

  // explicitly insert the module into the list of registered modules
  config.m_Modules.push_back( pModule.get() );

  // ensure that what we added is the same as what ended up in the 
  // configuration
  CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Finding module successfully returns the associated module",
      pMod, config.findModule("test")
      );
}


void CCtlConfigurationTests::addCommand_0()
{
  CCtlConfiguration config;

  // create out fake command
  CTCLInterpreter interp;
  auto pCommand = make_unique( new CTestCmd(interp) );

  // store address for testing purposes
  CTCLObjectProcessor* pCmd = pCommand.get();

  config.addCommand( move(pCommand) );

  CPPUNIT_ASSERT_EQUAL_MESSAGE( 
      "Adding command registers the object directly to configuration",
      pCmd, config.m_Commands.at(0).get() );
}
