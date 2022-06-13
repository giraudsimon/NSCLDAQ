/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/


#include "Main.h"
#include "CDataSourceFactory.h"
#include "CDataSinkFactory.h"
#include "CTransformFactory.h"
#include "C8p0to10p0Mediator.h"
#include "C10p0to8p0Mediator.h"
#include "C10p0to11p0Mediator.h"
#include "CTransform11p0to10p0.h"
#include "CTransform10p0to11p0.h"
#include "CTransformMediator.h"
#include "V8/CPhysicsEventBuffer.h"

using namespace std;
using namespace DAQ::Transform;

namespace DAQ {
  namespace V8 {
    std::size_t gBufferSize = 8192;
  }
}



//
Main::Main(int argc, char **argv)
  : m_pMediator(),
    m_argsInfo()
{

  cmdline_parser(argc, argv, &m_argsInfo);

  auto pSource = createSource();
  auto pSink = createSink();

  auto transformSpec = parseInOutVersions();

  if (transformSpec.first == 8) {
    setUpV8Transform();
  }

  if (transformSpec.first == 8 || transformSpec.second == 8) {
    setUpBufferSize();
  }

  setUpTransformFactory();

  m_pMediator.reset(m_factory.create(transformSpec.first, transformSpec.second));
  m_pMediator->setDataSource(pSource);
  m_pMediator->setDataSink(pSink);
}


//
unique_ptr<CDataSource> Main::createSource() const
{
  CDataSourceFactory sourceFactory;
  unique_ptr<CDataSource> pSource( sourceFactory.makeSource(m_argsInfo.source_arg, {}, {}) );

  return move(pSource);
}

//
unique_ptr<CDataSink> Main::createSink() const
{
  CDataSinkFactory factory;
  unique_ptr<CDataSink> pSink( factory.makeSink(m_argsInfo.sink_arg) );

  return move(pSink);
}

//
std::pair<int, int> Main::parseInOutVersions() const
{
  int inputVsn  = std::atoi(cmdline_parser_input_version_values[m_argsInfo.input_version_arg]);
  int outputVsn = std::atoi(cmdline_parser_output_version_values[m_argsInfo.output_version_arg]);

  return std::make_pair(inputVsn, outputVsn);
}

//
int Main::run()
{
  m_pMediator->initialize();

  m_pMediator->mainLoop();

  m_pMediator->finalize();

  return 0;
}

//
void Main::setUpTransformFactory()
{
  m_factory.setCreator(  8, 10, (new C8p0to10p0MediatorCreator()));
  m_factory.setCreator(  10, 8, (new C10p0to8p0MediatorCreator()));
  m_factory.setCreator( 10, 11, (new C10p0to11p0MediatorCreator()));
  m_factory.setCreator( 11, 10, (new CGenericCreator<CTransform11p0to10p0>()));

}


//
void Main::setUpV8Transform() const
{
  using namespace DAQ::V8;

  switch (m_argsInfo.v8_size_policy_arg) {
    case v8_size_policy_arg_Inclusive16BitWords:
      CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::Inclusive16BitWords;
      break;
    case v8_size_policy_arg_Inclusive32BitWords:
      CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::Inclusive32BitWords;
      break;
    case v8_size_policy_arg_Inclusive32BitBytes:
      CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::Inclusive32BitBytes;
      break;
    case v8_size_policy_arg_Exclusive16BitWords:
      CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::Exclusive16BitWords;
      break;
    default:
      // we should nevert get here
      throw std::runtime_error("Main::setUpTransform() User provided size policy unknown");
      break;
  }
}

//
void Main::setUpBufferSize() const {

  DAQ::V8::gBufferSize = m_argsInfo.v8_buffer_size_arg;

}

//
int main (int argc, char** argv) 
{
  Main theMain(argc, argv);

  return theMain.run();
}
