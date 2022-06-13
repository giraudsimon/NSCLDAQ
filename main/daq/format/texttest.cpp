// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#define protected public
#include <CRingTextItem.h>
#undef private
#undef protected

#include <DataFormat.h>

#include <string>
#include <vector>
#include <time.h>
#include <string.h>
#include <iostream>
using namespace std;

class texttests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(texttests);
  CPPUNIT_TEST(simplecons);
  CPPUNIT_TEST(fullcons);
  CPPUNIT_TEST(castcons);
  CPPUNIT_TEST(accessors);
  CPPUNIT_TEST(copycons);
  CPPUNIT_TEST(tscons);
  CPPUNIT_TEST(fractionalRunTime);
  CPPUNIT_TEST(origsid);     // v12.0-pre1
  CPPUNIT_TEST(tsorigsid);   // v12.0-pre1
  CPPUNIT_TEST(origsid_1);
  CPPUNIT_TEST(origsid_2);
  CPPUNIT_TEST(origsid_3);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void simplecons();
  void fullcons();
  void castcons();
  void accessors();
  void copycons();
  void tscons();
  void fractionalRunTime();
  void origsid();
  void tsorigsid();
  void origsid_1();
  void origsid_2();
  void origsid_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(texttests);

// Test simple construction

void texttests::simplecons() {
  // Make some strings:

  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");

  vector<string> strings;
  strings.push_back(s1);
  strings.push_back(s2);
  strings.push_back(s3);
  strings.push_back(s4);

  CRingTextItem item(PACKET_TYPES, strings);
  
  pTextItem pItem = reinterpret_cast<pTextItem>(item.getItemPointer());
  
  pTextItemBody pB = reinterpret_cast<pTextItemBody>(
    bodyPointer(reinterpret_cast<pRingItem>(pItem)));
  EQ((uint32_t)0, pB->s_timeOffset);
  EQ((uint32_t)4, pB->s_stringCount);

  // Check the contents:

  const char* p = pB->s_strings;
  EQ(s1, string(p));
  p  += strlen(p) + 1;
  EQ(s2, string(p));
  p += strlen(p) +1;
  EQ(s3, string(p));
  p += strlen(p) + 1;
  EQ(s4, string(p));

}

// Test the full constructor.

void texttests::fullcons()
{
  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");

  vector<string> strings;
  strings.push_back(s1);
  strings.push_back(s2);
  strings.push_back(s3);
  strings.push_back(s4);

  CRingTextItem item(PACKET_TYPES, strings,
		     1234,
		     5678);
  
  pTextItem pItem = reinterpret_cast<pTextItem>(item.getItemPointer());
  pTextItemBody pB = reinterpret_cast<pTextItemBody>(
        bodyPointer(reinterpret_cast<pRingItem>(pItem))
  );
  EQ((uint32_t)1234, pB->s_timeOffset);
  EQ((uint32_t)4, pB->s_stringCount);
  EQ((uint32_t)5678, pB->s_timestamp);



   // Check the contents:

  const char* p = pB->s_strings;
  EQ(s1, string(p));
  p  += strlen(p) + 1;
  EQ(s2, string(p));
  p += strlen(p) +1;
  EQ(s3, string(p));
  p += strlen(p) + 1;
  EQ(s4, string(p));
}
// test the 'casting' constructor.

void texttests::castcons()
{
  // Create a RingItem that is actually a text ring item:

  CRingItem ritem(PACKET_TYPES);
  
  pTextItem pText = reinterpret_cast<pTextItem>(ritem.getItemPointer());
  pText->s_body.u_noBodyHeader.s_body.s_timeOffset = 1234;
  pText->s_body.u_noBodyHeader.s_body.s_timestamp  = 4321;
  pText->s_body.u_noBodyHeader.s_body.s_stringCount= 4;
  char* p = &(pText->s_body.u_noBodyHeader.s_body.s_strings[0]);

  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");




  memcpy(p, s1.c_str(), strlen(s1.c_str()) + 1 );
  p += strlen(s1.c_str())+1;
  memcpy(p, s2.c_str(), strlen(s2.c_str()) + 1);
  p += strlen(s2.c_str())+1;
  memcpy(p, s3.c_str(), strlen(s3.c_str()) + 1);
  p += strlen(s3.c_str())+1;
  memcpy(p, s4.c_str(), strlen(s4.c_str()) +1 );
  p += strlen(s4.c_str())+1;
  ritem.setBodyCursor(p);
  ritem.updateSize();

  bool thrown = false;

  try {
    CRingTextItem item(ritem);
    pText = reinterpret_cast<pTextItem>(item.getItemPointer());
    pTextItemBody pB = reinterpret_cast<pTextItemBody>(
       bodyPointer(reinterpret_cast<pRingItem>(pText))
    );
    EQ((uint32_t)1234, pB->s_timeOffset);
    EQ((uint32_t)4321, pB->s_timestamp);
    EQ((uint32_t)4,    pB->s_stringCount);
    
    // Check the contents:
    
    char* p = pB->s_strings;
    EQ(s1, string(p));
    p  += strlen(p) + 1;
    EQ(s2, string(p));
    p += strlen(p) +1;
    EQ(s3, string(p));
    p += strlen(p) + 1;
    EQ(s4, string(p));
    
  }
  catch (...) {
    thrown = true;
  }
  ASSERT(!thrown);


  // Should fail:

  CRingItem bad(BEGIN_RUN);
  thrown = false;
  try {
    CRingTextItem item(bad);
  }
  catch (...) {
    thrown = true;
  }
  ASSERT(thrown);
}


// test the accessor functions:

void texttests::accessors()
{
  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");

  vector<string> strings;
  strings.push_back(s1);
  strings.push_back(s2);
  strings.push_back(s3);
  strings.push_back(s4);


  CRingTextItem item(PACKET_TYPES, strings );

  vector<string> content = item.getStrings();
 ASSERT(strings ==  content);

  //  The simpler ones.

 item.setTimeOffset(1234);
 EQ((uint32_t)1234, item.getTimeOffset());

 item.setTimestamp(66776);
 EQ((time_t)66776, item.getTimestamp());

}
// test coyp construction.

void texttests::copycons()
{
  vector<std::string> testStrings; // With apologies to Theo Geissle aka Dr. Seuss.
  testStrings.push_back("one string");
  testStrings.push_back("two strings");
  testStrings.push_back("three strings more");
  testStrings.push_back("red string");
  testStrings.push_back("blue string");
  size_t stringCount = testStrings.size();

  CRingTextItem original(MONITORED_VARIABLES,
			 testStrings,
			 1234, 5678);
  CRingTextItem copy(original);

  EQ(original.getBodySize(), copy.getBodySize());
  pRingItem porig = original.getItemPointer();
  pRingItem pcopy = copy.getItemPointer();
  
  // headers must match 

  EQ(itemSize(porig), itemSize(pcopy));
  EQ(itemType(porig), itemType(pcopy));

  // Contents must match:

  EQ(original.getTimeOffset(), copy.getTimeOffset());
  EQ(original.getTimestamp(),  copy.getTimestamp());
  
  vector<string> copiedStrings = copy.getStrings();
  EQ(stringCount, copiedStrings.size());
  for (int i = 0; i < stringCount; i++ ) {
    EQ (testStrings[i], copiedStrings[i]);
  }
  
}
// Test construction with timestamps:

void
texttests::tscons()
{
    string s1("String 1");
    string s2("string 2");
    string s3("string 3");
    string s4("last string");
  
    vector<string> strings;
    strings.push_back(s1);
    strings.push_back(s2);
    strings.push_back(s3);
    strings.push_back(s4);

    time_t stamp = time(NULL);  
  
    size_t stringSize = 0;
    for (int i = 0; i < strings.size(); i++) {
        stringSize += strlen(strings[i].c_str()) + 1;
    }
  
    CRingTextItem item(
      PACKET_TYPES, static_cast<uint64_t>(0x8877665544332211ll), 1, 2,
      strings, 1234, stamp, 1
    );
    pTextItem pItem = reinterpret_cast<pTextItem>(item.getItemPointer());
    
    //Check out the header:
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    EQ(PACKET_TYPES, uint32_t(itemType(reinterpret_cast<pRingItem>(pItem))));
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(TextItemBody)
            + stringSize 
        ), itemSize(reinterpret_cast<pRingItem>(pItem))
    );
    // The body header:
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    pBodyHeader pH = reinterpret_cast<pBodyHeader>(
       bodyHeader(reinterpret_cast<pRingItem>(pItem))
    );
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pH->s_size);
    EQ(static_cast<uint64_t>(0x8877665544332211ll), pH->s_timestamp);
    EQ(static_cast<uint32_t>(1), pH->s_sourceId);
    EQ(static_cast<uint32_t>(2), pH->s_barrier);
    
    // Ensure the body pointers from pItem and from the class jibe:
		// Use of u_hasBodyHeader.s_body is ok here because we are generating
		// synthetic events that don't have a body header extension.

    pTextItemBody pBody = reinterpret_cast<pTextItemBody>(item.getBodyPointer());
    EQ(
       pBody,
      reinterpret_cast<pTextItemBody>(bodyPointer(reinterpret_cast<pRingItem>(pItem)))
    );
    // Check the body contents against what was constructed:
    
    EQ(static_cast<uint32_t>(1234), pBody->s_timeOffset);
    EQ(static_cast<uint32_t>(stamp), pBody->s_timestamp);
    EQ(static_cast<uint32_t>(strings.size()), pBody->s_stringCount);
    EQ(static_cast<uint32_t>(1), pBody->s_offsetDivisor);
    
    const char* pStrings = pBody->s_strings;
    for (int i = 0; i < strings.size(); i++) {
        EQ(strings[i], std::string(pStrings));
        pStrings += strlen(pStrings) + 1;
    }
}
// Test computeElapsedTime
//
void
texttests::fractionalRunTime()
{
    string s1("String 1");
    string s2("string 2");
    string s3("string 3");
    string s4("last string");
  
    vector<string> strings;
    strings.push_back(s1);
    strings.push_back(s2);
    strings.push_back(s3);
    strings.push_back(s4);

    time_t stamp = time(NULL);  
  
    size_t stringSize = 0;
    for (int i = 0; i < strings.size(); i++) {
        stringSize += strlen(strings[i].c_str()) + 1;
    }
  
    CRingTextItem item(
      PACKET_TYPES, static_cast<uint64_t>(0x8877665544332211ll), 1, 2,
      strings, 1234, stamp, 3
    );
    EQ(static_cast<float>(1234.0/3.0), item.computeElapsedTime());
}

// V12.0-pre1.

void texttests::origsid()
{
  // Apologies to Dr. Seuss.
  const char* strings[4] = {"one string", "two string", "three string", "more"};
  time_t now = time(nullptr);
  pTextItem pItem = formatTextItem(4, now, 10, strings, PACKET_TYPES);
  pTextItemBody pBody = reinterpret_cast<pTextItemBody>(bodyPointer(
   reinterpret_cast<pRingItem>(pItem) 
  ));
  EQ(uint32_t(0), pBody->s_originalSid);
}
void texttests::tsorigsid()
{
// Apologies to Dr. Seuss.
  const char* strings[4] = {"one string", "two string", "three string", "more"};
  time_t now = time(nullptr);
  pTextItem pItem = formatTimestampedTextItem(
   0x123456789, 6, 0, 4, now, 10, strings, PACKET_TYPES, 1 
  );
  pTextItemBody pBody = reinterpret_cast<pTextItemBody>(bodyPointer(
   reinterpret_cast<pRingItem>(pItem) 
  ));
  EQ(uint32_t(6), pBody->s_originalSid);
}

void texttests::origsid_1()
{
 std::vector<std::string> strings={"set a b", "set c d", "set d e"};
 CRingTextItem item(PACKET_TYPES, strings);
 EQ(uint32_t(0), item.getOriginalSourceId());
}
void texttests::origsid_2()
{
 std::vector<std::string> strings={"set a b", "set c d", "set d e"};
 CRingTextItem item(PACKET_TYPES, strings, 12340, time(nullptr));
 EQ(uint32_t(0), item.getOriginalSourceId());
}
void texttests::origsid_3()
{
 std::vector<std::string> strings={"set a b", "set c d", "set d e"};
 CRingTextItem item(
     PACKET_TYPES, 0x9876543210, 777, 0, strings, 10, time(nullptr)
 );
 
 EQ(uint32_t(777), item.getOriginalSourceId());

}
