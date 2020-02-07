// Tests of the output observer

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define private public
#include "COrdererOutput.h"
#undef private

#include "fragment.h"


// We need some mocks:


// Blocks of data written by 'writeDataVUnlimited:

struct DataDescriptor {
    size_t nBytes;
    char*  pData;
} ;
std::vector<DataDescriptor> writtenData;

static void clearWrittenData()
{
    for (int i = 0; i < writtenData.size(); i++) {
        delete []writtenData[i].pData;
    }
    writtenData.clear();
}

static size_t sizeData(iovec* v, int n)
{
    size_t result(0);
    for (int i =0; i < n; i++) {
        result += v[i].iov_len;
    }
    
    return result;
}
static void collect(char* pData, iovec* v, int n) {
    for (int i =0; i < n; i++) {
        memcpy(pData, v[i].iov_base, v[i].iov_len);
        pData += v[i].iov_len;
    }
}
namespace io {
    // Mock to capture output so we can integrity test it.
    void writeDataVUnlimited(int fd, iovec* v, int n)
    {
        size_t nBytes = sizeData(v, n);
        char* pData   = new char[nBytes];
        collect(pData, v, n);
        DataDescriptor d = {nBytes, pData};
        writtenData.push_back(d);
    }
}

CFragmentHandler fhandlerInstance;

CFragmentHandler::CFragmentHandler() :
    m_outputThread(*(static_cast<COutputThread*>(nullptr))),
    m_sorter(*(static_cast<CSortThread*>(nullptr)))
{}

CFragmentHandler::~CFragmentHandler() {}

CFragmentHandler*

CFragmentHandler::getInstance() {return &fhandlerInstance;}

 void
 CFragmentHandler::addObserver(CFragmentHandler::Observer* p) {}

 void
 CFragmentHandler::removeObserver(CFragmentHandler::Observer* p) {}
 
 
 class OutputTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(OutputTests);
    CPPUNIT_TEST(output_1);
    CPPUNIT_TEST(output_2);
    CPPUNIT_TEST(output_3);
    CPPUNIT_TEST_SUITE_END();
    
    // Test methods
protected:
    void output_1();
    void output_2();
    void output_3();
    
// We need a pipe and COrdererOutput:

    int m_pipeFds[2];
    long m_maxWrites;
    COrdererOutput* m_pTestObj;
public:
    void setUp() {
        clearWrittenData();
        if (pipe2(m_pipeFds, 0)) {
            perror("ouptutTests setup could not open pipes");
            CPPUNIT_FAIL("Setup for outputTests failed 1");
        }
        m_maxWrites = fcntl(m_pipeFds[1], F_GETPIPE_SZ, nullptr);
        if (m_maxWrites < 0) {
            perror("Failed to get pipe max write size");
            CPPUNIT_FAIL("Setup for output tests failed 2");
        }
        m_pTestObj = new COrdererOutput(m_pipeFds[1]);
        
    }
    void tearDown() {
        delete m_pTestObj;
        close(m_pipeFds[0]);
        close(m_pipeFds[1]);
    }
private:
    void checkContents(EVB::Fragment* pFrags, size_t nFrags, int i);
 };
 
 CPPUNIT_TEST_SUITE_REGISTRATION(OutputTests);
 
 // Check the contents of the fragment array described by pFrags/nFrags
 // against the contents of writtenData[i].
 
 void
 OutputTests::checkContents(EVB::Fragment* pFrags, size_t nFrags, int i)
 {
    // Iterate over the fragments in pfrags:
    
    int nBytes = writtenData[i].nBytes;
    char* pData= writtenData[i].pData;
    
    for (int f =0; f < nFrags; f++) {
        // The remaining data must be at lest a fragment header + payload
        // Since the writes are full fragments:
        
        ASSERT(sizeof(EVB::FragmentHeader) + pFrags[f].s_header.s_size <= nBytes);
        
        // The header and payload must match:
        
        EQ(0, memcmp(
            &(pFrags[f].s_header), pData, sizeof(EVB::FragmentHeader))
        );
        EQ(0, memcmp(
            pFrags[f].s_pBody, pData+sizeof(EVB::FragmentHeader),
            pFrags[f].s_header.s_size
        ));
        
        // Next chunk of stored data:
        
        pData += sizeof(EVB::FragmentHeader) + pFrags[f].s_header.s_size;
        nBytes -= sizeof(EVB::FragmentHeader) + pFrags[f].s_header.s_size;
    }
 }
 
 // EvbFragments is a std::deque<std::pair<time_t, EVB::pFragment>>
 
 // Write a fragment to the outoput.  This should come across
 // as a flattened fragment.  Note we're doing this as much
 // to ensure that our mocks work as anything else.
 //
 void OutputTests::output_1()
 {
    EVB::Fragment frag;
    char  body[100];
    EvbFragments frags;
   
    for (int i = 0; i < 100; i++) {
        body[i] = i;
    }
    frag = {{1234, 1, 100, 0}, body};
    std::pair<time_t, EVB::pFragment> f={time(nullptr), &frag};
    frags.push_back(f);
    
    (*m_pTestObj)(frags);
    
    // We should have one block of data, and it's our fragment
    // flattened.
    
    EQ(size_t(1), writtenData.size());
    EQ(sizeof(EVB::FragmentHeader) + 100 , writtenData[0].nBytes);
    EQ(0, memcmp(
        &frag.s_header, writtenData[0].pData, sizeof(EVB::FragmentHeader)
    ));
    EQ(0, memcmp(
        body, writtenData[0].pData + sizeof(EVB::FragmentHeader), 100)
    );
 }
 // THe biggest set of fragments that will fit in a single write.
 // each fragment body is 100 bytes and are different because
 // we continuously count.
 //
 void OutputTests::output_2()
 {
    int fragSize = 100 + sizeof(EVB::FragmentHeader);
    int maxFrags = m_maxWrites / fragSize;
    
    char payloads[maxFrags*100];
    for (int i =0; i < sizeof(payloads); i++) {
        payloads[i] = i;    // 100 is not a good base 2 no. so all payloads differ.
    }
    EVB::Fragment frags[maxFrags];
    EvbFragments fragList;
    
    char* p = payloads;
    for (int i =0; i < maxFrags; i++) {
        frags[i] = {{uint64_t(i), uint32_t(i), 100, 0}, p};
        fragList.push_back({time_t(i), &(frags[i])});
        p += 100;
    }
    (*m_pTestObj)(fragList);
    
    // there should be only one writevunlimited.  We don't have to
    // worry about iov chunking because of where we mocked.
    
    EQ(size_t(1), writtenData.size());
    checkContents(frags, maxFrags, 0);
    
 }
 // Queue up fragments that require 1 and a fraction writes.
 
 void OutputTests::output_3()
 {
    int fragSize = 100 + sizeof(EVB::FragmentHeader);
    int maxFrags = m_maxWrites / fragSize;
    int nFrags   = maxFrags + maxFrags/2;
    ASSERT(nFrags > maxFrags);     // Else the test is invalid.
    
    char payloads[nFrags*100];
    for (int i =0; i < sizeof(payloads); i++) {
        payloads[i] = i;    // 100 is not a good base 2 no. so all payloads differ.
    }
    EVB::Fragment frags[nFrags];
    EvbFragments fragList;
    
    char* p = payloads;
    for (int i =0; i < nFrags; i++) {
        frags[i] = {{uint64_t(i), uint32_t(i), 100, 0}, p};
        fragList.push_back({time_t(i), &(frags[i])});
        p += 100;
    }
    (*m_pTestObj)(fragList);
    
    // Should be two glops of data:
    
    EQ(size_t(2), writtenData.size());
    
    // First glop:
    
    checkContents(frags, maxFrags, 0);
    
    // Second glop
    
    checkContents(frags+maxFrags, nFrags - maxFrags, 1);
}
 