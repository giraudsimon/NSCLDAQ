#ifndef CRINGMASTER_H
#define CRINGMASTER_H
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#include <string>

// Forward class definitions.

class CRingBuffer;
/*!
   This class provides access to the ring master server.  
   The ring master server provides diagnostic access
   to the rings in a system, as well as spawning off
   dat hoisters as needed for remote systems.

   See the documentation of the ring master process itself
   for more.  This class provides C++ clients easy to use
   access to a local or remote RingMaster.  Note that
   all transactions block.

   For some additional strangeness, see the documentation/comments
   for the RequestData member function.

*/
class CRingMaster
{
  // Instance data:
private:
  int      m_socket;		// Socket connected to the server.
  bool     m_isConnected ;      // True if the object is still 
  bool     m_isDataConnection;  // True if the object holds data connection.
  bool     m_isLocal;           // True if connected to localhost.

  // Constructors and canonicals.
public:
  CRingMaster(std::string host=std::string("localhost"));
  virtual ~CRingMaster();

private:
  CRingMaster(const CRingMaster& rhs);
  CRingMaster& operator=(const CRingMaster& rhs);
  int operator==(const CRingMaster& rhs);
  int operator!=(const CRingMaster& rhs);
public:

  // The public interface of the class...

  void notifyConsumerConnection(std::string ringname, int id, std::string comment);
  void notifyProducerConnection(std::string ringname, std::string comment);
  void notifyConsumerDisconnection(std::string ringname, int slot);
  void notifyProducerDisconnection(std::string ringname);
  void notifyCreate(std::string ringname);
  void notifyDestroy(std::string ringname);
  int  requestData(std::string ringname);
  std::string requestUsage();
  
  // Utilities:

private:
  void transactionOk();

  void simpleTransaction(std::string line);
  void sendLine(std::string line);
  std::string getLine();
  std::string ipAddress(std::string host);
  int         getPort(std::string host);
  static bool badErrno();
	void throwIfBadErrno();
  static std::string formatConnection(std::string ringname, const char* type, std::string comment);
  static std::string formatDisconnection(std::string ringname, const char* type);
};



#endif
