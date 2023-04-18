//  Exceptions.cpp
//  
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#include "Exceptions.h"

namespace TrAnal
{

    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    // 
    // AlgorithmOutOfBoundsException
    //
    //________________________________________________________________________________________

    AlgorithmOutOfBoundsException::AlgorithmOutOfBoundsException(const unsigned int algopoint, 
            const unsigned int valid_low, 
            const unsigned int valid_high) throw() 
        : /* ProcessorException(),*/ 
            fMessage() 
    {
        std::string mess;
        mess += "Attempt to process point ";
	mess += std::to_string(algopoint);
        mess += "\nValid range is [";
	mess += std::to_string(valid_low) + "," + std::to_string(valid_high) + ")";
        fMessage = mess;
    }

    AlgorithmOutOfBoundsException::AlgorithmOutOfBoundsException(const AlgorithmOutOfBoundsException& exc) throw()
        : /*ProcessorException(exc),*/ 
            fMessage(exc.fMessage) 
    {}

    AlgorithmOutOfBoundsException& AlgorithmOutOfBoundsException::operator=(const AlgorithmOutOfBoundsException& exc) throw()
    {
        if (this!=&exc) {
            fMessage = exc.fMessage;
        }
        return *this;
    } 

    AlgorithmOutOfBoundsException::~AlgorithmOutOfBoundsException() throw() {}

    const char* AlgorithmOutOfBoundsException::what() const throw() { return fMessage.c_str(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    // 
    // InvalidResultException
    //
    //________________________________________________________________________________________
    
    InvalidResultException::InvalidResultException(const std::string& message) throw() :
        /* ProcessorException(),*/ 
        fMessage(message) 
    {}

    InvalidResultException::InvalidResultException(const InvalidResultException& exc) throw()
        : /*ProcessorException(),*/  
            fMessage(exc.fMessage) 
    {}

    InvalidResultException& InvalidResultException::operator=(const InvalidResultException& exc) throw()
    {
        if (this!=&exc) {
            fMessage = exc.fMessage;
        }
        return *this;
    } 

    InvalidResultException::~InvalidResultException() throw() {}

    const char* InvalidResultException::what() const throw() { return fMessage.c_str(); }

} // end namespace
