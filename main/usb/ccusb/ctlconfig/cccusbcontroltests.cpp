

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include <CCCUSBControlCreator.h>
#include <string> 
#include <iostream> 
#include <iomanip> 

#define private public
#define protected public
#include <CCCUSBControl.h>
#undef protected
#undef private

using namespace std;

class CCCUSBControlTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CCCUSBControlTests);
//  CPPUNIT_TEST(update);
  CPPUNIT_TEST(decodeInputSize0);
  CPPUNIT_TEST(decodeList0);
  CPPUNIT_TEST(marshallOutput0);
  CPPUNIT_TEST_SUITE_END();

  private:
    CCCUSBControl* m_pMod;
    CCCUSB* m_pUSB;

  public:
    void setUp();
    void tearDown();

//    void update0();
    void decodeInputSize0();
    void decodeList0();
    void marshallOutput0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CCCUSBControlTests);


void CCCUSBControlTests::setUp() {
  m_pMod = new CCCUSBControl();
  
}

void CCCUSBControlTests::tearDown() {
  delete m_pMod;
}

//void CCCUSBControlTests::update() {
//  EQMSG("update returns OK", string("OK"), m_pMod->Update(*m_pUSB));
//}

template<class Iter>
void printDiff(Iter lbegin, Iter lend, Iter rbegin, Iter rend) {
  Iter lcursor = lbegin;
  Iter rcursor = rbegin;
  int i=0;
  cout << "\n";
  while (lcursor!=lend || rcursor!=rend) {

    cout << setw(4) << i;
    if (lcursor==lend) {
      cout << setw(8) << " ";
    } else {
      cout << setw(8) << *lcursor;
      ++lcursor;
    } 

    if (rcursor==rend) {
      cout << setw(8) << " "; 
    } else {
      cout << setw(8) << *rcursor;
      ++rcursor;
    }
    cout << endl;

    ++i;
  } 

}


void CCCUSBControlTests::decodeInputSize0()
{
  string list("{512} {5 0 1 2 3 4}");
  size_t size = m_pMod->decodeInputSize(list);
  EQMSG("decode input returns first number",
        static_cast<size_t>(512), size);
}

void CCCUSBControlTests::decodeList0() {
  string strList("{512} {5 0 1 2 3 4}");
  std::vector<uint16_t> list = m_pMod->decodeList(strList);
  std::vector<uint16_t> modelList(6);
  modelList[0] = 5;
  modelList[1] = 0;
  modelList[2] = 1;
  modelList[3] = 2;
  modelList[4] = 3;
  modelList[5] = 4;


//  printDiff(modelList.begin(), modelList.end(), list.begin(), list.end());
  // We expect that the first integer has been stripped off 
  // and a list of the remaining five elements has been returned
  ASSERT(modelList == list);
}



void CCCUSBControlTests::marshallOutput0()
{
  uint8_t buffer[8];
  for (int i=0; i<8; i++) buffer[i] = i;

  string actual = m_pMod->marshallOutput(buffer,8);
  string model = "OK  - {0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 }";

  EQMSG("marshallOutput", model, actual);
  

}
