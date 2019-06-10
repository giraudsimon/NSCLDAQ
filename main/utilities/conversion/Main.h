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

#ifndef MAIN_H
#define MAIN_H

#include <CTransformFactory.h>
#include <format_converter_options.h>

#include <memory>
#include <utility>

class CBaseMediator;

/*!
 * \brief The Main for format_converter
 *
 * This is mainly in charge of setting stuff up. It will process the
 * command line options and then set up the mediator. Once the mediator
 * is set up, then the mediator mainLoop() is what defines the behavior
 * of the program.
 */
class Main
{
private:
  cmdLineOpts                       m_argsInfo;
  DAQ::Transform::CTransformFactory m_factory;
  std::unique_ptr<CBaseMediator>    m_pMediator;

public:
    Main(int argc, char** argv);

    int run();

private:
    std::unique_ptr<CDataSource> createSource() const;
    std::unique_ptr<CDataSink> createSink() const;
    void setUpTransformFactory();
    std::pair<int, int> parseInOutVersions() const;
    void setUpV8Transform() const;
    void setUpBufferSize() const;

};

#endif // MAIN_H
