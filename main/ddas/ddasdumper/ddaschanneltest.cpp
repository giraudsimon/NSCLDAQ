/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#include <cppunit/extensions/HelperMacros.h>

#include "Asserts.h"
#include <ddaschannel.h>

#include <DDASHit.h>
#include <DDASHitUnpacker.h>
#include <cstdint>
#include <cmath>
#include <vector>

using namespace std;
using namespace ::DAQ::DDAS;


template<class T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vec)
{
  stream << "{ ";
  for (auto& element : vec ) stream << element << " ";
  stream << "}";

  return stream;
}


// A test suite 
class ddaschanneltest : public CppUnit::TestFixture
{
  private:
    ddaschannel hit;

  public:
    CPPUNIT_TEST_SUITE( ddaschanneltest );
    CPPUNIT_TEST( crateId_0 );
    CPPUNIT_TEST( slotId_0 );
    CPPUNIT_TEST( chanId_0 );
    CPPUNIT_TEST( headerLength_0 );
    CPPUNIT_TEST( eventLength_0 );
    CPPUNIT_TEST( finishCode_0 );
    CPPUNIT_TEST( msps_0 );
    CPPUNIT_TEST( timelow_0 );
    CPPUNIT_TEST( timehigh_0 );
    CPPUNIT_TEST( coarseTime_0 );
    CPPUNIT_TEST( time_0 );
    CPPUNIT_TEST( cfdFail_0 );
    CPPUNIT_TEST( cfdTrigSource_0 );
    CPPUNIT_TEST( energySums_0 );
    CPPUNIT_TEST( qdcSums_0 );
    CPPUNIT_TEST( trace_0 );
    CPPUNIT_TEST( hdwrRevision_0 );
    CPPUNIT_TEST( adcResolution_0 );
    CPPUNIT_TEST( overunderflow_0 );
    CPPUNIT_TEST( reset_0);
    CPPUNIT_TEST( defaultCtor_0);
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp() {

      vector<uint32_t> data = { 0x0000002c, 0x0f0e0064, 0x00290321,
                                0x0000f687, 0x947f000a, 0x800888be,
                                0x00000001, 0x00000002, 0x00000003,
                                0x00000004,
                                0x00000005, 0x00000006, 0x00000007,
                                0x00000008,
                                0x00000009, 0x0000000a, 0x0000000b,
                                0x0000000c,
                                0x00020001, 0x00040003, 0x00060005,
                                0x00080007};
      
      hit = ddaschannel();
      hit.UnpackChannelData(data.data());
    }

    void tearDown() {

    }

    void crateId_0 () {
      EQMSG("Simple body extracts crate id", Int_t(3), hit.GetCrateID()); 
    }
    void slotId_0 () {
      EQMSG("Simple body extracts slot id", Int_t(2), hit.GetSlotID());
    }
    void chanId_0 () {
      EQMSG("Simple body extracts channel id", Int_t(1), hit.GetChannelID()); 
    }
    void headerLength_0 () {
      EQMSG("Simple body extracts header length", Int_t(16), hit.GetChannelLengthHeader()); 
    }
    void eventLength_0 () {
      EQMSG("Simple body extracts event length", Int_t(20), hit.GetChannelLength()); 
    }

    void finishCode_0 () {
      EQMSG("Simple body extracts finish code", Int_t(0), hit.GetFinishCode()); 
    }

    void msps_0 () {
      EQMSG("Simple body extracts adc frequency", Int_t(100), hit.GetModMSPS()); 
    }

    void timelow_0 () {
      EQMSG("Simple body extracts time low", UInt_t(63111), hit.GetTimeLow()); 
    }
    void timehigh_0 () {
      EQMSG("Simple body extracts time high", UInt_t(10), hit.GetTimeHigh()); 
    }
    void coarseTime_0 () {
      EQMSG("Simple body coarse time", (0x000a0000f687)*10.0, hit.GetCoarseTime()); 
    }
    void time_0 () {
      ASSERTMSG("Simple body full time",
                 std::abs(hit.GetTime()-429497360711.601257) < 0.000001);
    }

    void cfdFail_0 () {
      EQMSG("Simple body compute cfd fail bit", Int_t(1), hit.cfdfailbit);
    }

    void cfdTrigSource_0 () {
      EQMSG("Simple body compute cfd trig source bit", Int_t(0), hit.cfdtrigsourcebit);
    }

    void energySums_0 () {
      std::vector<UInt_t> expected = {1, 2, 3, 4};
      EQMSG("Found all 4 energy sums",
            expected, hit.energySums);
    }

    void qdcSums_0 () {
      std::vector<UInt_t> expected = {5,6,7,8,9,10,11,12};
      EQMSG("Found all 4 energy sums",
            expected, hit.qdcSums);
    }

    void trace_0 () {
      std::vector<UShort_t> expected = {1, 2, 3, 4, 5, 6, 7, 8};
      EQMSG("Found all trace samples",
            expected, hit.GetTrace());
    }

    void hdwrRevision_0() {
        EQMSG("Hardware revision", Int_t(15), hit.GetHardwareRevision());
    }

    void adcResolution_0() {
        EQMSG("ADC resolution", Int_t(14), hit.GetADCResolution());
    }

    void overunderflow_0() {
        EQMSG("ADC overflow/underflow", true, hit.GetADCOverflowUnderflow());
    }

    void reset_0() {
        hit.Reset();
        EQMSG("crate id", Int_t(0), hit.GetCrateID());
        EQMSG("slot id", Int_t(0), hit.GetSlotID());
        EQMSG("channel id", Int_t(0), hit.GetChannelID());
        EQMSG("header length", Int_t(0), hit.GetChannelLengthHeader());
        EQMSG("event length", Int_t(0), hit.GetChannelLength());
        EQMSG("finish code", Int_t(0), hit.GetFinishCode());
        EQMSG("adc frequency", Int_t(0), hit.GetModMSPS());
        EQMSG("time low", UInt_t(0), hit.GetTimeLow());
        EQMSG("time high", UInt_t(0), hit.GetTimeHigh());
        EQMSG("coarse time", 0., hit.GetCoarseTime());
        EQMSG("full time", 0., hit.GetTime());
        EQMSG("cfd fail bit", Int_t(0), hit.cfdfailbit);
        EQMSG("cfd trig source bit", Int_t(0), hit.cfdtrigsourcebit);
        EQMSG("energy sums", size_t(0), hit.energySums.size());
        EQMSG("energy sums", size_t(0), hit.qdcSums.size());
        EQMSG("trace samples", size_t(0), hit.GetTrace().size());
        EQMSG("Hardware revision", Int_t(0), hit.GetHardwareRevision());
        EQMSG("ADC resolution", Int_t(0), hit.GetADCResolution());
        EQMSG("ADC overflow/underflow", false, hit.GetADCOverflowUnderflow());

    }

    void defaultCtor_0() {
        ddaschannel hit2;
        EQMSG("crate id", Int_t(0), hit2.GetCrateID());
        EQMSG("slot id", Int_t(0), hit2.GetSlotID());
        EQMSG("channel id", Int_t(0), hit2.GetChannelID());
        EQMSG("header length", Int_t(0), hit2.GetChannelLengthHeader());
        EQMSG("event length", Int_t(0), hit2.GetChannelLength());
        EQMSG("finish code", Int_t(0), hit2.GetFinishCode());
        EQMSG("adc frequency", Int_t(0), hit2.GetModMSPS());
        EQMSG("time low", UInt_t(0), hit2.GetTimeLow());
        EQMSG("time high", UInt_t(0), hit2.GetTimeHigh());
        EQMSG("coarse time", 0., hit2.GetCoarseTime());
        EQMSG("full time", 0., hit2.GetTime());
        EQMSG("cfd fail bit", Int_t(0), hit2.cfdfailbit);
        EQMSG("cfd trig source bit", Int_t(0), hit2.cfdtrigsourcebit);
        EQMSG("energy sums", size_t(0), hit2.energySums.size());
        EQMSG("energy sums", size_t(0), hit2.qdcSums.size());
        EQMSG("trace samples", size_t(0), hit2.GetTrace().size());
        EQMSG("Hardware revision", Int_t(0), hit2.GetHardwareRevision());
        EQMSG("ADC resolution", Int_t(0), hit2.GetADCResolution());
        EQMSG("ADC overflow/underflow", false, hit2.GetADCOverflowUnderflow());

    }



};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( ddaschanneltest );
