//  Exceptions.h
//  
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>
#include <TObject.h>

namespace TrAnal 
{

//    class ProcessorException 
//    {
//        public:
//            ProcessorException() throw() {}
//
//            ProcessorException(const ProcessorException& exc) throw() {}
//
//            virtual ~ProcessorException() throw() {}
//
//            virtual const char* what() throw() { return "Processor exception"; }
//
//            ClassDef(ProcessorException,0);
//    };


    class AlgorithmOutOfBoundsException// : public ProcessorException
    {
        private:
            std::string fMessage;

        public:
            AlgorithmOutOfBoundsException(const unsigned int algopoint, const unsigned int valid_low, const unsigned int valid_high) throw();

            AlgorithmOutOfBoundsException(const AlgorithmOutOfBoundsException& exc) throw();

            AlgorithmOutOfBoundsException& operator=(const AlgorithmOutOfBoundsException& exc) throw();

            ~AlgorithmOutOfBoundsException() throw();

            const char* what() const throw();

            /// \cond
            ClassDef(AlgorithmOutOfBoundsException,0);
            /// \endcond
    };

    class InvalidResultException // : public ProcessorException
    {
        private:
            std::string fMessage;

        public:
            InvalidResultException(const std::string& message) throw();

            InvalidResultException(const InvalidResultException& exc) throw();

            InvalidResultException& operator=(const InvalidResultException& exc) throw();

            ~InvalidResultException() throw();

            const char* what() const throw();

            /// \cond
            ClassDef(InvalidResultException,0);
            /// \endcond

    };

} // end namespace

#endif

