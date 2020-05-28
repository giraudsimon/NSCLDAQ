/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef CEVBFRAMEWORKAPP_H
#define CEVBFRAMEWORKAPP_H


#include <list>

// forward definitions.

struct gengetopt_args_info;
class  CEVBClientApp;
class  CEventOrderClient;
#ifndef EVBFRAMEWORK_H
class  CEVBFragmentList;	/* Could be prior defined completely. */
#endif
/**
 *  CEVBFrameworkApp - the application class of the frameowrk.
 *  Mainly controls the lifecycle of all of the framework
 *  data source objects.
 *  We are also responsible for creating the event builder connection
 *  class and maintaining its health.
 */

class CEVBFrameworkApp 
{
private:
  // Class level data.

  static CEVBFrameworkApp*    m_pInstance;

  // Object level data.

  struct gengetopt_args_info* m_pArgs;
  std::list<CEVBClientApp*>   m_sources;
  CEventOrderClient*          m_pBuilder; 


  // This is a singleton, so the constructor is private.
  // use getInstance to get the single instance of this object:

private:
  CEVBFrameworkApp();

  // These are private because they are not implemented/supported.

  CEVBFrameworkApp(const CEVBFrameworkApp& rhs);
  CEVBFrameworkApp& operator=(const CEVBFrameworkApp& rhs);
  int operator==(const CEVBFrameworkApp& rhs);
  int operator!=(const CEVBFrameworkApp& rhs);
  
  // Singleton method:

public:
  static CEVBFrameworkApp* getInstance();
  static int main(int argc, char** argv);


  // Object operations:
public:

  void addSource(CEVBClientApp* pSource);
  void removeSource(CEVBClientApp* pSource);
  void send(CEVBFragmentList& fragmentList);
  const struct gengetopt_args_info* getParsedArgs() const;

  // Private methods.

private:
  int operator()(int argc, char** argv);

  void EVBConnect(const char* host, const char* port, const char* description,
		  std::list<int> sourceIds, const char* pName =0);
  void ReportError(const char* context, const char* pType, const char* message);
  int  LookupService(const char* serviceName);

  // Inner classes:

public:
  class SourcePoll {
  private:
    int m_ms;
  public:
    SourcePoll(int ms);
    void operator()(CEVBClientApp* pSource);
  };
};


#endif
