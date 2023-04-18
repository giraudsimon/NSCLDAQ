//  Solver.cpp
//  
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#include "Solver.h"

namespace TrAnal
{

double LinearSolver::operator()(const std::deque<double>& que) const
{
    // given (0,y0) and (1,y1), the line b/t is 
    // y = (y1-y0) * x + y0
    //   = m * x + b
    // The root is then at
    //  x_root = -b/m
    //         = -y0 / (y1-y0)
    return -1.0*que[0]/(que[1]-que[0]);
}

}// end namespace
