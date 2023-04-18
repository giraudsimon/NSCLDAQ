//  Solver.h
//  
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifndef SOLVER_H
#define SOLVER_H


#include <deque>
#include "TObject.h"

namespace TrAnal
{

/// Pure abstract base class for root-finding algorithms
/**
 * Solver objects fit a set of values with a function and then 
 * extracts the nearest zero crossing value. These are used by the
 * CFD algorithms to locate zero crossings.
 */
class Solver
{

    public:
        /// The root-finding algorithm
        /**
        * Locates the distance from the first element of the deque
        * to a zero crossing point.
        */
        virtual double operator()(const std::deque<double>& que) const =0;

        /// The minimum number of points required by the algorithm to define unique solution
        virtual unsigned int npoints() const = 0;

    // ROOT dictionary generating code
    /// \cond
    ClassDef(Solver,0);
    /// \endcond

}; // end Solver

/// \class LinearSolver
/// \brief Root finder based on linear interpolation
class LinearSolver : public Solver
{

    public:
        /// Interpolates between two points linear to find a zero crossing
        /**
         * Locates the distance of the zero crossing from the first element in the que.
         * The return value is to be interpreted as a fractional index. For example, if 
         * the argument contained two values, que[0]=0.75, que[1]=-0.25. The algorithm
         * will treat this as two points (0,0.75) and (1,-0.25) and find the "x" value
         * of the zero crossing.
         */
        virtual double operator() (const std::deque<double>& que) const;
    
        /// Number of points required for a unique solution
        virtual unsigned int npoints() const {return 2;}

        // ROOT dictionary generating code
        /// \cond
        ClassDef(LinearSolver,0);
        /// \endcond

}; // end LinearSolver

} // end namespace

#endif
