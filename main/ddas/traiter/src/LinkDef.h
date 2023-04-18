//  LinkDef.h
//  
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;


#pragma link C++ namespace TrAnal;
#pragma link C++ nestedclasses;

// Trace class
#pragma link C++ class TrAnal::Basic_Trace<uint16_t>+;
#pragma link C++ class TrAnal::TraceT<uint16_t>+;

// Iterator classes
#pragma link C++ class TrAnal::TrIterator<uint16_t>;
#pragma link C++ class TrAnal::TrRange<uint16_t>;
#pragma link C++ class TrAnal::AlgoIterator<uint16_t>;

#pragma link C++ class TrAnal::SumIterator<uint16_t>;
#pragma link C++ class TrAnal::TrapFilter<uint16_t>;

// Algorithms and necessary return types
#pragma link C++ function TrAnal::ComputeMean<uint16_t>(const TrAnal::TrIterator<uint16_t>&,const TrAnal::TrIterator<uint16_t>&);
#pragma link C++ function TrAnal::ComputeStDev<uint16_t>(const TrAnal::TrIterator<uint16_t>&,const TrAnal::TrIterator<uint16_t>&,double);
#pragma link C++ class TrAnal::BaseLineProcResult+;
#pragma link C++ function TrAnal::ComputeBaseLine<uint16_t>(const TrAnal::TrIterator<uint16_t>&,const TrAnal::TrIterator<uint16_t>&);
#pragma link C++ function TrAnal::FindPeak<uint16_t>(const TrAnal::TrIterator<uint16_t>&,const TrAnal::TrIterator<uint16_t>&);
#pragma link C++ class TrAnal::PeakFindProcResult<uint16_t>+;
#pragma link C++ function TrAnal::Threshold<uint16_t>(const TrAnal::TrIterator<uint16_t>&, const TrAnal::TrIterator<uint16_t>&, double);
#pragma link C++ function TrAnal::Threshold<uint16_t>(const TrAnal::TrRange<uint16_t>&, double);
#pragma link C++ class TrAnal::CFDResult<uint16_t>;
#pragma link C++ function TrAnal::CFD<uint16_t>(const TrAnal::TrRange<uint16_t>&, int, int, int, int, const TrAnal::Solver&);

#pragma link C++ function TrAnal::ComputeAmplitude<uint16_t>(const TrAnal::TrRange<uint16_t>&, const TrAnal::TrRange<uint16_t>&);
#pragma link C++ class TrAnal::AmplitudeProcResult+;

#pragma link C++ function TrAnal::ComputeRiseTime(const TrAnal::TrRange<uint16_t>&, const TrAnal::TrRange<uint16_t>&);
#pragma link C++ function TrAnal::ComputeRiseTime(const TrAnal::TrIterator<uint16_t>&, int, int);
#pragma link C++ class TrAnal::RiseTimeProcResult<uint16_t>;

// Solvers
#pragma link C++ class TrAnal::Solver;
#pragma link C++ class TrAnal::LinearSolver;

// Exception classes
#pragma link C++ class TrAnal::AlgorithmOutOfBoundsException;
#pragma link C++ class TrAnal::InvalidResultException;

#endif

