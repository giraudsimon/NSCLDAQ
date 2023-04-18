/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file: QDCLen.h
 *  @brief:  Define the QDC Length setting dialog.
 */
#ifndef QDCLEN_H
#define QDCLEN_H
#include "Table.h"

/**
 * @class QDCLen
 *    Provides the dialog that prompts for the 8 QDC lengths
 *    each channel can have.
 *
 */
class QDCLen : public Table
{
    short unsigned m_module;
    short unsigned m_channel;
    bool           m_loaded;
    TGNumberEntry* chanCopy;
public:
    QDCLen(const TGWindow* p, const TGWindow* main);
    virtual ~QDCLen();
    
    Bool_t ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2);
    int change_values(Long_t mod);
    int load_info(Long_t mod);
    void setModule(short unsigned mod);
    
};

#endif