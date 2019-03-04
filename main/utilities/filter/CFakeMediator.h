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

#ifndef CFAKEMEDIATOR_H
#define CFAKEMEDIATOR_H

#include <CMediator.h>

#include <vector>
#include <string>

class CFakeMediator : public CMediator {
  private:
    std::vector<std::string> m_log;
  
  public:
    CFakeMediator(): CMediator(nullptr, nullptr, nullptr), m_log() {}
    void mainLoop() {
      m_log.push_back("mainLoop");
    }

    void initialize() {
      m_log.push_back("initialize");
    }

    void finalize() {
      m_log.push_back("finalize");
    }

    std::vector<std::string> getLog () const { return m_log;}
};

#endif
