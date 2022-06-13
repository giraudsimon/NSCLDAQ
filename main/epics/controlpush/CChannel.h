#ifndef CCHANEL_H
#define CCHANEL_H

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

#include <cadef.h>
#include <string>
#include <time.h>
#include <typeinfo>
#include <CExtensibleFactory.h>

class CConverter;

/**
 *   The CChannel class implements transparently updated EPICS channels for C++
 * programs.  Objects require a two-phase construction, as we won't be sure of the
 * timing of static construction vs. the EPICS startup.
 *  - The first phase intializes the class member data.
 *  - The second phase connects the channel to the EPICS updating mechanism.
 */

class CChannel 
{
public:
  typedef void (*Slot)(CChannel*, void*);
private:
  std::string   m_sName;
  bool          m_fConnected;
  chid          m_nChannel;
  bool          m_fUpdateHandlerEstablished;
  bool          m_fConnectionHandlerEstablished;
  std::string   m_sValue;
  time_t        m_LastUpdateTime;
  CConverter*   m_pConverter;
  Slot          m_pHandler;
  void*         m_pHandlerData;
  evid          m_eventHandlerId;
  
public:
  CChannel(std::string name);
  virtual ~CChannel();

  // Forbidden operations.

private:
  CChannel(const CChannel&);
  CChannel& operator=(const CChannel&);
  int operator==(const CChannel&) const;
  int operator!=(const CChannel&) const;
public:

  // Operations on the object:

  std::string getName()     const;
  bool        isConnected() const;

  virtual void Connect();
  time_t       getLastUpdate() const;
  std::string  getValue()      const;

  void setSlot(Slot handler, void* data);

  // operations to set channel values
  // this is just an overloaded assign to the channel:
  // the return value is the thing assigned:

  std::string operator=(std::string value);
  int         operator=(int value);
  double      operator=(double value);




  // Class level operations.

  static void doEvents(float seconds);

protected:
  static void StateHandler(connection_handler_args args);
  static void UpdateHandler(event_handler_args     args);
};

//////////////////////////////////////////////////////////////////////////////
/*!
**   The classes below are used to convert between epics native values
**   and std::strings... this is required becuse epics does not always
**   choose a good conversion.
**
**   In addition a conversion factory is provided.
**   This is all much simpler than it looks.  There are basically
**   only the following conversion types that we entertain:
**    std::string : Ask the user to request DBF_STRING and convert that
**             as %s
**    uint:    Ask the user to request DBF_ULONG and convert that.
**             as %ul
**             NOTE: Due to some funkiness in the epics headers I don't
**             understand, these can'tbe made to work. at this time.
**    int      Ask the user to request DBF_LONG and convert that.
**             and convert that as %l
**    float:   Ask the user to request DBF_DOUBLE  and convert that.
**             as %12.9g
** 
**  The normal path usage of the stuff below is:
**    In the connection handler for the channel
**    get the underlying data type using ca_field_type
**    Pass that in to an invocation of 
**      CConversionFactory::getInstance->Converter();
**    This dynamically allocates a converter, call the pointer to it
**     pConverter
**    Invoke pConverter->requestType()
**    And use the return value as the data type requested when
**    invoking ca_add_event on the channel.
**    In the UpdateHandler,
**       std::string value = (*pConverter)(args);
**    to get a nice std::string value returned for the channel.
**
*/

/*!
   Abstract CConverter base class.... this is pure abstract.
*/

class CConverter 
{
public:
  virtual short  requestType() = 0;
  virtual std::string operator()(event_handler_args args) = 0;
};

class CStringConverter : public CConverter 
{
public:
  virtual short requestType();
  virtual std::string operator()(event_handler_args args);

};



class CIntegerConverter : public CConverter
{
public:
  virtual short requestType();
  virtual std::string operator()(event_handler_args args);
};

class CFloatConverter : public CConverter
{
public:
  virtual short requestType();
  virtual std::string operator()(event_handler_args args);
};

/*!
   Provides the correct converter type for the
   particular epics data type.
   If no type exists, an exception is thrown.
*/

// per daqdev/NSCLDAQ#510 - recasting in terms of extensible factories.


using ConverterFactory = CGenericExtensibleFactory<short, CConverter>;
using ConverterCreator = CCreator<CConverter>;


class ConversionFactory {
  static ConversionFactory* m_pInstance;
  ConverterFactory           m_factory;
public:
  static ConversionFactory* getInstance();
  CConverter* Converter(short type);
  
private:
  ConversionFactory();
  
};
#endif
