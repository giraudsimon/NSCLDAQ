#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file ratioModel.tcl
# @brief Model that provides a ratio of scalers to the page.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ratioModel 1.0
package require snit

snit::type ratioModel {
    option -numerator
    option -denominator
    option -numeratorname
    option -denominatorname
    
    # Given the alarm combinations what to return
    
    variable alarmTable -array [list     \
      ok.ok         ok                   \
      low.ok        low                  \
      ok.low        low                  \
      low.low       low                  \
      high.ok       high                 \
      high.low      high                 \
      ok.high       high                 \
      low.high      high                 \
      high.high     high                 \
    ]
    
    ##
    # getNumeratorName
    # @return string - value of -name
    #
    method getNumeratorName {} {
        return $options(-numeratorname)
    }
    ##
    # getDenominatorName
    #
    # @return string (empty).
    #
    method getDenominatorName {}  {
        return $options(-denominatorname)
    }
    ##
    # getNumeratorRate
    #
    # @return float - rate of counts in scaler.
    #
    method getNumeratorRate {} {
        set channel $options(-numerator)
        return [format "%6.2f " [$channel rate]]
    }
    ##
    # getDenominatorRate
    #
    #  @return string (empty)
    #
    method getDenominatorRate {} {
        set channel $options(-denominator)
        return [format %6.2f [$channel rate]]
    }
    ##
    # getNumeratorTotal
    #
    # @return longint - Total scaler counts.
    #
    method getNumeratorTotal {} {
        set channel $options(-numerator)
        
        return [format "%6d " [$channel total]]
    }
    ##
    # getDenominatorTotal
    #
    # @return string ""
    #
    method getDenominatorTotal {} {
        set channel $options(-denominator)
        return [format %6d [$channel total]]
    }
    ##
    # getRateRatio
    #
    # @return string (empty)
    #
    method getRateRatio {} {
        set num $options(-numerator)
        set den $options(-denominator)
        
        set nrate [$num rate]
        set drate [$den rate]
        
        return [_ratio $nrate $drate]
        
    }
    ##
    # getTotalRatio
    #
    # @return string ""
    #
    
    method getTotalRatio {} {
        set num $options(-numerator)
        set den $options(-denominator)
        
        set ntot [$num total]
        set dtot [$den total]
        
        return [_ratio $ntot $dtot]
    }
    ##
    # alarmState
    #
    # @return string - in {ok, high, low} indicating if there is an alarm
    #                  in the underlying channel and, if so, which one.
    #                  we're going to return the worst of the alarms. Specifically
    #                  alarm levels from worst to best are high, low, ok.
    #
    method alarmState {} {
        set num $options(-numerator)
        set den $options(-denominator)
        
        set nalarm [$num alarming]
        set dalarm [$den alarming]
        
        set composite [join [list $nalarm $dalarm] .]
        
        
        return $alarmTable($composite)
    }
    
    proc _ratio {num den} {
        if {$den != 0} {
            return [format %6.2f [expr {double($num)/$den}]]
        } else {
            return *
        }
    }
}
