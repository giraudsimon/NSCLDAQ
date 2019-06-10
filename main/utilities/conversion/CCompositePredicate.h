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
#ifndef CCOMPOSITEPREDICATE_H
#define CCOMPOSITEPREDICATE_H

#include <CPredicate.h>

#include <vector>
#include <memory>

class CCompositePredicate : public CPredicate
{
  private:
    std::vector<std::unique_ptr<CPredicate> > m_predicates;

  public:
    void addPredicate(std::unique_ptr<CPredicate> pPred);

    bool operator()();
};

#endif
