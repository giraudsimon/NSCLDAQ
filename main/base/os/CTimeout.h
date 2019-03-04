/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
         Jeromy Tompkins
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/


#ifndef CTIMEOUT_H
#define CTIMEOUT_H

#include <chrono>

/*!
 * \brief The CTimeout class
 *
 * This is simple class to encapsulate the logic required for
 * evaluating if a timeout has expired. One can create it and
 * then evaluate at any point whether it has expired.
 *
 */
class CTimeout
{
private:
    double                                         m_nSeconds;
    std::chrono::high_resolution_clock::time_point m_start;

public:
    /*!
     * \brief Constructor
     *
     * \param nSeconds - length of timeout in units of seconds
     *
     */
    CTimeout(double nSeconds);

    /*!
     * \brief getRemainingSeconds
     *
     * This retrieves the amount of time that remains before expiration.
     *
     * \retval 0 - timeout has elapsed
     * \retval >0 - otherwise
     */
    double getRemainingSeconds();

    /*!
     * \brief expired
     *
     * Checks whether the timeout has expired
     *
     * \return bool
     * \retval false - not expired
     * \retval ture  - expired
     */
    bool expired();
};

#endif // CTIMEOUT_H
