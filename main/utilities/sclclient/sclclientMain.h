#ifndef SCLCLIENTMAIN_H
#define SCLCLIENTMAIN_H
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
#include <vector>
#include <map>
#include <stdint.h>

// forward class definitions:

class CDataSource;
class CRingScalerItem;
class CRingStateChangeItem;
class TclServerConnection;
class TcpClientConnection;

/*!
  Main application class.  This should be instantiated by main() an
  used as a function object.
*/
class SclClientMain {
  // Private per object data.

private:
  CDataSource*         m_pRing;
  std::string          m_Host;
  int                  m_Port;
  TclServerConnection* m_pServer;
  std::vector<double>  m_Totals;
  std::map<uint32_t, std::vector<double> > m_sourcedTotals;

  // Constructors and canonicals.
public:
  SclClientMain();
  virtual ~SclClientMain();
private:
  SclClientMain(const SclClientMain& rhs);
  SclClientMain& operator=(const SclClientMain& rhs);
  int operator==(const SclClientMain& rhs);
  int operator!=(const SclClientMain& rhs);
public:

  // public interface:

  int operator()(int argc, char** argv);

  // utilities:

private:
  int  getDisplayPort(std::string portArg);
  void processItems();
  void processScalers(const CRingScalerItem& item);
  void processScalers(uint32_t sourceId, float start, float end,
                      std::vector<uint32_t> values, bool incremental);
  void processStateChange(const CRingStateChangeItem& item);
  void initializeStateChange();
  void connectTclServer();
  void ConnectionLost();
  static void ConnectionLostRelay(TcpClientConnection& connection, void* theObject);
  std::string defaultRing();

  void setInteger(std::string name, int value, int index = -1);
  void setDouble(std::string name,  double value, int index = -1);
  void clearTotals();
};

#endif
