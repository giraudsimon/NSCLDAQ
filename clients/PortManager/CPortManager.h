#ifndef __CPORTMANAGER_H
#define __CPORTMANAGER_H
/*!
 *  The CPortManager class interfaces with the 
 * Tcp/Ip port allocation server. Using this class allows applications in
 * C++ to:
 * - List the port allocations on any system that runs the port allocation
 *   server.
 * - Allocate ports from the system that the client is running on.
 * 
 * Note that listing port usage only requires the remote system to run the
 * port allocation server, while acquiring a port requires the localhost to
 * run the server.
 */
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//(c) 2005 Board of Trustees of Michigan State Univeristy All rights reserved.
//
// Licensed under the GNU Public License, 
//   see http://www.gnu.org/licenses/gpl.txt for license text.
//
#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

// Forward references:

struct sockaddr_in;

// ---------------------------------------------------------------

class CPortManager {
public:
	//! portInfo lists the information about a single port.
	typedef struct _portInfo {
		int         s_Port;                 //!< Number of a port.
		STD(string) s_Application;          //!< Application owning the port.
		STD(string) s_User;					//!< User running the application
	} portInfo;
private:
	STD(string)     m_sHost;                //!< Host we're talking to
	int             m_nPort;                //!< Port to connect to.
	bool            m_fisConnected;         //!< holding persistent connection?
	int             m_nSocket;              //!< current socket.

    // Construction:
public:   
    CPortManager(STD(string) host = STD(string)("localhost"));
    CPortManager(STD(string) host, int Port);
    ~CPortManager();
    
    // Cannonical operations:
    
    // Copy construction is illegal.
private:
	CPortManager(const CPortManager& rhs);
	CPortManager& operator=(const CPortManager& rhs);
public:
	// But comparison does make sense.
	
	int operator==(const CPortManager& rhs) const;
	int operator!=(const CPortManager& rhs) const;
	
    // Class operations:
public:
    int allocatePort(STD(string) application);
    STD(vector)<portInfo> getPortUsage();
	// Utility functions
private:
	void Connect(); 
	void Disconnect();
	static int DetermineDefaultPort();
	void GetNetworkAddress(sockaddr_in& addr) const;
	STD(string) GetLine() const;
	void    GetPortInfo(portInfo& info, STD(string) line);
	static STD(string) GetUsername();
	static int tcp();
};

#endif
