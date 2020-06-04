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

/** @file: clienttests.cpp  
 *  @brief: Tess for the CEventOrderClient class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CEventOrderClient.h"
#undef private

#include <CPortManager.h>
#include <CPortManagerException.h>
#include "fragment.h"

#include <sys/uio.h>
#include <stdint.h>
#include <ErrnoException.h>
#include <vector>
#include <CSocket.h>
#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include <Thread.h>
#include <string.h>

// This 'class' accepts a connecton on a port and then saves
// data from the peer until the connection is broken.

struct Simulator {
    struct Request {
        EVB::ClientMessageHeader s_hdr;
        void*                   s_body;
    };
    std::vector<Request> m_requests;
    int                  m_nPort;
    CSocket*             m_pSocket;
    Simulator(int port) : m_nPort(port), m_pSocket(nullptr) {}
    ~Simulator() {
        delete m_pSocket;
        for (int i =0;i < m_requests.size(); i++) {
            free(m_requests[i].s_body);
        }
    }
        
    void operator()();
};
// accept a connection on m_nPort, accept messages on the port an reply
// OK to them until disconnected.

void Simulator::operator()() {
    m_pSocket = new CSocket;
    std::stringstream port;
    port << m_nPort;
    m_pSocket->Bind(port.str());
    m_pSocket->Listen();
    
    std::string client;
    CSocket* pConn = m_pSocket->Accept(client);
    
    // Each request is a header and, if the header body size is nonzero
    // a dynamically allocated body.
    
    while (pConn->getState() == CSocket::Connected) {
        try {
            EVB::ClientMessageHeader header;
            void*                   pBody(nullptr);
            
            int nBytes = pConn->Read(&header, sizeof(header));
            assert(nBytes == sizeof(header));          // Needed to get a full header
            
            if (header.s_bodySize) {
                pBody = malloc(header.s_bodySize);
                assert(pBody);
                
                assert(pConn->Read(pBody, header.s_bodySize) == header.s_bodySize);
            }
            // Save the message:
            
            Request req = {header, pBody };
            m_requests.push_back(req);
            
            pBody = nullptr;
            pConn->Write("OK\n", strlen("OK\n"));
        } catch(...) {
            break;
        }
    }
    delete pConn;
    
}


// Now a thread to run the Simulator:

class SimulatorThread : public Thread
{
private:
    Simulator&  m_rSimulator;
public:
    SimulatorThread(Simulator& sim) : m_rSimulator(sim)
    {}
    
    void run() {
        m_rSimulator();
    }
};


// the test suite.

class clienttest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(clienttest);
    CPPUNIT_TEST(chainbytes_1);
    CPPUNIT_TEST(chainbytes_2);
    CPPUNIT_TEST(chainbytes_3);
    
    CPPUNIT_TEST(iovecs_1);
    CPPUNIT_TEST(iovecs_2);
    CPPUNIT_TEST(iovecs_3);

    CPPUNIT_TEST(fillvecs_1);
    CPPUNIT_TEST(fillvecs_2);
    CPPUNIT_TEST(fillvecs_3);
    
    CPPUNIT_TEST(connect_1);
    CPPUNIT_TEST(connect_2);
    CPPUNIT_TEST(connect_3);
    
    CPPUNIT_TEST(disconnect_1);
    CPPUNIT_TEST(disconnect_2);
    
    CPPUNIT_TEST(frag_1);
    CPPUNIT_TEST(frag_2);
    CPPUNIT_TEST_SUITE_END();
protected:
    void chainbytes_1();
    void chainbytes_2();
    void chainbytes_3();
    
    void iovecs_1();
    void iovecs_2();
    void iovecs_3();
     
    void fillvecs_1();
    void fillvecs_2();
    void fillvecs_3();
    
    void connect_1();
    void connect_2();
    void connect_3();
    
    void disconnect_1();
    void disconnect_2();
    
    void frag_1();
    void frag_2();
private:
    CPortManager*  m_pPortManager;
    int            m_nPort;
    CEventOrderClient* m_pClient;
public:
    clienttest() :m_pPortManager(nullptr) {}
    void setUp() {
        // If I don't have the port yet, allocate it.
        if (!m_pPortManager) {
            m_pPortManager = new CPortManager;
            m_nPort        = m_pPortManager->allocatePort("ORDERER");
        }
        // make the client each time:
        
        m_pClient = new CEventOrderClient("localhost", m_nPort);
    }
    void tearDown() {
        delete m_pClient;
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(clienttest);

void clienttest::chainbytes_1()
{
    // An empty chain results in no bytes in the chain.
    
    size_t nBytes = m_pClient->bytesInChain(nullptr);
    EQ(size_t(0), nBytes);
}
void clienttest::chainbytes_2()
{
    // One chain entry is the header size + body size.
    
    EVB::Fragment      frag;
    frag.s_header.s_size = 100;
    
    EVB::FragmentChain head;
    head.s_pNext =nullptr;
    head.s_pFragment = &frag;
    
    size_t nBytes =  m_pClient->bytesInChain(&head);
    EQ(sizeof(EVB::FragmentHeader) + frag.s_header.s_size, nBytes);
}

void clienttest::chainbytes_3()
{
    
    // Several frags:
    
    EVB::Fragment frags[10];
    EVB::FragmentChain chain[10];
    for (int i =0; i < 10; i++) {
        frags[i].s_header.s_size = 100;
        chain[i].s_pFragment = &(frags[i]);
        if (i < 9) {
            chain[i].s_pNext = &(chain[i+1]);
        } else {
            chain[i].s_pNext = nullptr;
        }
    }
    
    size_t nBytes = m_pClient->bytesInChain(chain);
    EQ(10*(100 + sizeof(EVB::FragmentHeader)), nBytes);
    
}
void clienttest::iovecs_1()
{
    // Empty chain no iovecs:
    
    size_t nVecs = m_pClient->iovecsInChain(nullptr);
    EQ(size_t(0), nVecs);
}
void clienttest::iovecs_2()
{
    // One chain entry gives 2 (one for header, one for discontiguous body).
    
    EVB::Fragment      frag;
    frag.s_header.s_size = 100;
    
    EVB::FragmentChain head;
    head.s_pNext =nullptr;
    head.s_pFragment = &frag;

    size_t nVecs = m_pClient->iovecsInChain(&head);
    EQ(size_t(2), nVecs);
    
}
void clienttest::iovecs_3()
{
    // 10 chain entries gives 20 iovecs:
    
    EVB::Fragment frags[10];
    EVB::FragmentChain chain[10];
    for (int i =0; i < 10; i++) {
        frags[i].s_header.s_size = 100;
        chain[i].s_pFragment = &(frags[i]);
        if (i < 9) {
            chain[i].s_pNext = &(chain[i+1]);
        } else {
            chain[i].s_pNext = nullptr;
        }
    }
    
    size_t nVecs = m_pClient->iovecsInChain(chain);
    EQ(size_t(20), nVecs);
}


void clienttest::fillvecs_1()
{
    // No frags at least won't fail.
    
    CPPUNIT_ASSERT_NO_THROW(
        m_pClient->fillFragmentDescriptors(nullptr, nullptr)
    );
}

void clienttest::fillvecs_2()
{
    uint8_t data[100];
    
    EVB::Fragment      frag;
    frag.s_header.s_size = 100;
    frag.s_pBody          = data;
    
    
    EVB::FragmentChain head;
    head.s_pNext =nullptr;
    head.s_pFragment = &frag;
    
    iovec vec[2];
    m_pClient->fillFragmentDescriptors(vec, &head);
    
    EQ(sizeof(EVB::FragmentHeader), vec[0].iov_len);
    EQ((void*)(&(frag.s_header)), vec[0].iov_base);
    
    EQ(sizeof(data), vec[1].iov_len);
    EQ((void*)(data), vec[1].iov_base);
}

void clienttest::fillvecs_3()
{
    // Bunch of chain entries.
    
    uint8_t data[10*100];
    EVB::Fragment frags[10];
    EVB::FragmentChain chain[10];
    
    for (int i =0; i < 10; i++) {
        frags[i].s_header.s_size = 100;
        frags[i].s_pBody         = &(data[i*100]);
        chain[i].s_pFragment = &(frags[i]);
        if (i < 9) {
            chain[i].s_pNext = &(chain[i+1]);
        } else {
            chain[i].s_pNext = nullptr;
        }
    }
    
    iovec vecs[20];               // 2 / chain element.
    m_pClient->fillFragmentDescriptors(vecs, chain);
    
    for (int i =0; i < 20; i+=2) {
        EQ(sizeof(EVB::FragmentHeader), vecs[i].iov_len);
        EQ((void*)(&(frags[i/2].s_header)),        vecs[i].iov_base);
        
        EQ(size_t(100), vecs[i+1].iov_len);
        EQ((void*)(&(data[i/2*100])), vecs[i+1].iov_base);
    }
}

void clienttest::connect_1()
{
    // bad host throws.
    
    std::list<int> sources;
    
    CPPUNIT_ASSERT_THROW(
        m_pClient->Connect("some.junk.node", sources),
        CErrnoException
    );
    
    
}
void clienttest::connect_2()
{
    // Connect but no sources
    
    Simulator       sim(m_nPort);
    SimulatorThread thread(sim);
    thread.start();
    usleep(200);
    
    std::list<int> sources;                // No sources.
    CPPUNIT_ASSERT_NO_THROW(
        m_pClient->Connect("A test", sources)
    );
    
    // Close it and join:
    
    delete m_pClient;
    m_pClient = nullptr;            // So cleanup doesn't fail.
    thread.join();                  // Wait on thread exit.
    
    // Check out the results... should be one request, a connection.
    
    EQ(size_t(1), sim.m_requests.size());
    Simulator::Request r = sim.m_requests[0];
    EQ(EVB::CONNECT, r.s_hdr.s_msgType);
    EQ(sizeof(EVB::ConnectBody), size_t(r.s_hdr.s_bodySize));
    EVB::pConnectBody b = static_cast<EVB::pConnectBody>(r.s_body);
    EQ(0, strcmp("A test", b->s_description));
    EQ(uint32_t(0), b->s_nSids);
}
void clienttest::connect_3()
{
    // connect with source specs.

    Simulator       sim(m_nPort);
    SimulatorThread thread(sim);
    thread.start();
    usleep(200);
    
    std::list<int> sources;                // No sources.
    sources.push_back(0);
    sources.push_back(1);
    sources.push_back(2);
    
    CPPUNIT_ASSERT_NO_THROW(
        m_pClient->Connect("A test", sources)
    );
    
    // Close it and join:
    
    delete m_pClient;
    m_pClient = nullptr;            // So cleanup doesn't fail.
    thread.join();                  // Wait on thread exit.
    
    Simulator::Request r = sim.m_requests[0];
    EVB::pConnectBody b = static_cast<EVB::pConnectBody>(r.s_body);
    EQ(uint32_t(3), b->s_nSids);
    for (int i =0; i < 3; i++) {
        EQ(uint32_t(i), b->s_sids[i]);
    }
}
void clienttest::disconnect_1()
{
    // disconnect without connect is an error.
    
    CPPUNIT_ASSERT_THROW(
        m_pClient->disconnect(),
        CErrnoException
    );
    
}

void clienttest::disconnect_2()
{
    // Good disconnect with simulator.
    
    Simulator       sim(m_nPort);
    SimulatorThread thread(sim);
    thread.start();
    usleep(200);
    
    std::list<int> sources;                // No sources.
    
    m_pClient->Connect("A test", sources);
    CPPUNIT_ASSERT_NO_THROW(m_pClient->disconnect());
    
    delete m_pClient;
    m_pClient = nullptr;
    
    thread.join();
    
    // Should be two messges, a connect and a disconnect:
    
    EQ(size_t(2), sim.m_requests.size());
    
    // Only care about the second which should be a disconnect no body.
    
    Simulator::Request r = sim.m_requests[1];
    EQ(EVB::DISCONNECT, r.s_hdr.s_msgType);
    EQ(uint32_t(0),     r.s_hdr.s_bodySize);
    
    
}
void clienttest::frag_1()
{
    // Empty fragment chain is legal and results in an empty fragments
    //msg.
    
     Simulator       sim(m_nPort);
    SimulatorThread thread(sim);
    thread.start();
    usleep(200);
    
    std::list<int> sources;                // No sources - not caller checks not us.
    
    m_pClient->Connect("A test", sources);
    
    CPPUNIT_ASSERT_NO_THROW(
        m_pClient->submitFragments(nullptr);
    );
    delete m_pClient;
    m_pClient = nullptr;
    thread.join();
    
    EQ(size_t(2), sim.m_requests.size());
    Simulator::Request f = sim.m_requests[1];
    
    EQ(EVB::FRAGMENTS, f.s_hdr.s_msgType);
    EQ(uint32_t(0), f.s_hdr.s_bodySize);
    
}

void clienttest::frag_2()
{
    // fragment chain with one element containing a counting pattern.

    Simulator       sim(m_nPort);
    SimulatorThread thread(sim);
    thread.start();
    usleep(200);

    
    uint8_t data[100];
    for (int i =0;i < 100; i++) { data[i] = i;}
    
    EVB::Fragment      frag;
    frag.s_header.s_size = 100;
    frag.s_header.s_sourceId = 1;
    frag.s_header.s_timestamp = 0x123456789;
    frag.s_header.s_barrier = 0;
    frag.s_pBody = data;
    
    EVB::FragmentChain head;
    head.s_pNext =nullptr;
    head.s_pFragment = &frag;
    
    std::list<int> sources; sources.push_back(1);
    m_pClient->Connect("Fragment test", sources);
    
    m_pClient->submitFragments(&head);
    
    delete m_pClient;             // drops connection
    m_pClient = nullptr;
    thread.join();
    
    // Lets' look at that second message.
    
    Simulator::Request m = sim.m_requests.at(1);
    EVB::ClientMessageHeader& h(m.s_hdr);
    EVB::pFlatFragment         f = static_cast<EVB::pFlatFragment>(m.s_body);
    
    EQ(EVB::FRAGMENTS, h.s_msgType);
    EQ(sizeof(EVB::FragmentHeader) + 100, size_t(h.s_bodySize));
    
    // The fragment heder:
    
    EVB::FragmentHeader& fh(f->s_header);
    EQ(frag.s_header.s_size, fh.s_size);
    EQ(frag.s_header.s_sourceId, fh.s_sourceId);
    EQ(frag.s_header.s_timestamp, fh.s_timestamp);
    EQ(frag.s_header.s_barrier, fh.s_barrier);
    
    // The data:
    
    uint8_t* fbody = reinterpret_cast<uint8_t*>(f->s_body);
    for (int i =0; i < 100; i++) {
        EQ(data[i], fbody[i]);
    }
    
}