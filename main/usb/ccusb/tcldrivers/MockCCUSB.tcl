

package provide MockCCUSB 1.0

package require snit


namespace eval MockCCUSB {

  ## @brief a Mock of the CCUSB API to record command histories
  #
  snit::type Controller {

    option -q   -default 1
    option -x   -default 1
    option -value16 -default 65535
    option -value24 -default 16777215 

    variable m_history  ;#< the command history

    ## @brief Constructor
    #
    # @param args   option-value pairs
    #
    # @returns  fully-qualified name of new instance
    constructor {args} {
      set m_history [list]

      $self configurelist $args
    }

    # -------------------------------------------------------------------------
    #  Single shot operations
    #
    #  These all just append the name of the command and arguments to the
    #  history


    method simpleWrite24 {n a f data} {
      lappend m_history "simpleWrite24 $n $a $f $data"
      return [$self getXQ]
    }

    method simpleRead24 {n a f} {
      lappend m_history "simpleRead24 $n $a $f"
      
      set xq [$self getXQ]
      return [expr {$options(-value24)|($xq<<24)}]
    }

    method simpleControl {n a f} {
      lappend m_history "simpleControl $n $a $f"
      return [$self getXQ]
    }

    ## @brief Append all entries of the stack to the history
    #
    # @param stack  a MockCCUSB::ReadoutList
    # @param max    not used
    method executeList {stack max} {
      set stackHistory [$stack getHistory]
      foreach cmd $stackHistory {
        lappend m_history $cmd
      }

      # 
      set data [$stack getData]
      set lastCmd [lindex $m_history end]
      if {[string match "write*" $lastCmd]} {
        lappend data [$self getXQ]
      }

      return $data
    }

    ## @brief Retrieves the history of commands
    #
    method getHistory {} {
      return $m_history
    }

    method getXQ {} {
      set x $options(-x)
      set q $options(-q)
      return [expr {($x<<1)|$q}]
    }
  } ;# end Controller


  ## @brief A fake CCCUSBReadout list that maintains creates a cmd history
  #
  snit::type ReadoutList {

    option -q   -default 1
    option -x   -default 1
    option -value16 -default 65535
    option -value24 -default 16777215 

    variable m_history  ;#< the command history
    variable m_data  ;#< the command history

    ## @brief Constructor
    #
    # @param args   option-value pairs
    constructor {args} {
      set m_history [list]
      set m_data [list]
    }

    ## @brief Stack building commands
    # 
    # These all simply add to the history a description of the command
    #

    method addControl {n a f} {
      lappend m_history "control $n $a $f"
    }

    method addRead16 {n a f {lamWait 0}} {
      lappend m_history "read16 $n $a $f $lamWait"
      lappend m_data $options(-value16)
    }

    method addRead24 {n a f {lamWait 0}} {
      lappend m_history "read24 $n $a $f $lamWait"
      set xq [$self getXQ]
      lappend m_data [expr {$options(-value24) | ($xq<<24)}]
    }

    method addWrite16 {n a f data} {
      lappend m_history "write16 $n $a $f $data"
    }

    method addWrite24 {n a f data} {
      lappend m_history "write24 $n $a $f $data"
    }


    method addAddressPatternRead16 {n a f {lamWait 0}} {
      lappend m_history "addressPatternRead16 $n $a $f $lamWait"
    }
    

    method addQScan {n a f max {lamWait 0}} {
      lappend m_history "qscan $n $a $f $max $lamWait"
    }

    method addQStop {n a f max {lamWait 0}} {
      lappend m_history "qstop $n $a $f $max $lamWait"
    }

    ## @brief Retrieves the history
    #
    # @returns a list of the commands called
    #
    method getHistory {} {
      return $m_history
    }


    ## @brief Retrieves the history
    #
    # @returns a list of the commands called
    #
    method getData {} {
      return $m_data
    }

    method getXQ {} {
      set x $options(-x)
      set q $options(-q)
      return [expr {($x<<1)|$q}]
    }

  } ;# end ReadoutList


} ;# end namespace

