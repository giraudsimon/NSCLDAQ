#==============================================================================
# Contains the implementation of the tablelist widget.
#
# Structure of the module:
#   - Namespace initialization
#   - Public procedure
#   - Private configuration procedures
#   - Private procedures implementing the tablelist widget command
#   - Private callback procedures
#   - Private procedures used in bindings
#   - Private helper procedures
#
# Copyright (c) 2000-2002  Csaba Nemethi (E-mail: csaba.nemethi@t-online.de)
#==============================================================================

#
# Namespace initialization
# ========================
#
package provide tablelistWidget 1.0
if {[info var tk_version] == ""} {
    return;                             # pkg_mkIndex runing us.
}

namespace eval tablelist {
    #
    # The array configSpecs is used to handle configuration options.  The
    # names of its elements are the configuration options for the Tablelist
    # class.  The value of an array element is either an alias name or a list
    # containing the database name and class as well as an indicator specifying
    # the widget(s) to which the option applies: c stands for all children
    # (text widgets and labels), b for the body text widget, h for the header
    # text widget, l for the labels, f for the frame, and w for the widget
    # itself.
    #
    #	Command-Line Name	 {Database Name		  Database Class      W}
    #	------------------------------------------------------------------------
    #
    variable configSpecs
    array set configSpecs {
	-activestyle		 {activeStyle		  ActiveStyle	      w}
	-arrowcolor		 {arrowColor		  ArrowColor	      w}
	-arrowdisabledcolor	 {arrowDisabledColor	  ArrowDisabledColor  w}
	-background		 {background		  Background	      b}
	-bg			 -background
	-borderwidth		 {borderWidth		  BorderWidth	      f}
	-bd			 -borderwidth
	-columns		 {columns		  Columns	      w}
	-cursor			 {cursor		  Cursor	      c}
	-disabledforeground	 {disabledForeground	  DisabledForeground  w}
	-exportselection	 {exportSelection	  ExportSelection     w}
	-font			 {font			  Font		      b}
	-foreground		 {foreground		  Foreground	      b}
	-fg			 -foreground
	-height			 {height		  Height	      w}
	-highlightbackground	 {highlightBackground	  HighlightBackground f}
	-highlightcolor		 {highlightColor	  HighlightColor      f}
	-highlightthickness	 {highlightThickness	  HighlightThickness  f}
	-incrarrowtype		 {incrArrowType		  IncrArrowType	      w}
	-labelbackground	 {labelBackground	  Background	      l}
	-labelbg		 -labelbackground
	-labelborderwidth	 {labelBorderWidth	  BorderWidth	      l}
	-labelbd		 -labelborderwidth
	-labelcommand		 {labelCommand		  LabelCommand	      w}
	-labeldisabledforeground {labelDisabledForeground DisabledForeground  l}
	-labelfont		 {labelFont		  Font		      l}
	-labelforeground	 {labelForeground	  Foreground	      l}
	-labelfg		 -labelforeground
	-labelheight		 {labelHeight		  Height	      l}
	-labelpady		 {labelPadY		  Pad		      l}
	-labelrelief		 {labelRelief		  Relief	      l}
	-listvariable		 {listVariable		  Variable	      w}
	-relief			 {relief		  Relief	      f}
	-resizablecolumns	 {resizableColumns	  ResizableColumns    w}
	-resizecursor		 {resizeCursor		  ResizeCursor	      w}
	-selectbackground	 {selectBackground	  Foreground	      w}
	-selectborderwidth	 {selectBorderWidth	  BorderWidth	      w}
	-selectforeground	 {selectForeground	  Background	      w}
	-selectmode		 {selectMode		  SelectMode	      w}
	-setgrid		 {setGrid		  SetGrid	      w}
	-showarrow		 {showArrow		  ShowArrow	      w}
	-showlabels		 {showLabels		  ShowLabels	      w}
	-showseparators		 {showSeparators	  ShowSeparators      w}
	-sortcommand		 {sortCommand		  SortCommand	      w}
	-state			 {state			  State		      w}
	-stretch		 {stretch		  Stretch	      w}
	-stripebackground	 {stripeBackground	  Background	      w}
	-stripebg		 -stripebackground
	-stripeforeground	 {stripeForeground	  Foreground	      w}
	-stripefg		 -stripeforeground
	-stripeheight		 {stripeHeight		  StripeHeight	      w}
	-takefocus		 {takeFocus		  TakeFocus	      f}
	-width			 {width			  Width		      w}
	-xscrollcommand		 {xScrollCommand	  ScrollCommand	      h}
	-yscrollcommand		 {yScrollCommand	  ScrollCommand	      b}
    }

    proc extendConfigSpecs {} {
	variable helpLabel
	variable configSpecs

	# Kludges for 8.5:

	#
	# Append the default values of the configuration options
	# of a temporary, invisible listbox widget to the values
	# of the corresponding elements of the array configSpecs
	#
	
	set helpListbox .__helpListbox
	for {set n 0} {[winfo exists $helpListbox]} {incr n} {
	    set helpListbox .__helpListbox$n
	}
	listbox $helpListbox
	foreach configSet [$helpListbox config] {
	    if {[llength $configSet] != 2} {
		set opt [lindex $configSet 0]
		if {[info exists configSpecs($opt)]} {
		    lappend configSpecs($opt) [lindex $configSet 3]
		}
	    }
	}
	destroy $helpListbox

	#
	# Append the default values of some configuration options
	# of an invisible label widget to the values of the
	# corresponding -label* elements of the array configSpecs
	#
	set helpLabel .__helpLabel
	for {set n 0} {[winfo exists $helpLabel]} {incr n} {
	    set helpLabel .__helpLabel$n
	}
	label $helpLabel
	foreach optTail {font height} {
	    set configSet [$helpLabel config -$optTail]
	    lappend configSpecs(-label$optTail) [lindex $configSet 3]
	}
	if {[catch {$helpLabel config -state disabled}] == 0 &&
	    [catch {$helpLabel config -state normal}] == 0 &&
	    [catch {$helpLabel config -disabledforeground} configSet] == 0} {
	    lappend configSpecs(-labeldisabledforeground) [lindex $configSet 3]
	} else {
	    unset configSpecs(-labeldisabledforeground)
	}
	if {[string compare $::tcl_platform(platform) windows] == 0} {
	    lappend configSpecs(-labelpady) 0
	} else {
	    set configSet [$helpLabel config -pady]
	    lappend configSpecs(-labelpady) [lindex $configSet 3]
	}

	#
	# Steal the default values of some configuration
	# options from a temporary, invisible button widget
	#
	set helpButton .__helpButton
	for {set n 0} {[winfo exists $helpButton]} {incr n} {
	    set helpButton .__helpButton$n
	}
	button $helpButton
	foreach opt {-disabledforeground -state} {
	    set configSet [$helpButton config $opt]
	    lappend configSpecs($opt) [lindex $configSet 3]
	}
	foreach optTail {background foreground} {
	    set configSet [$helpButton config -$optTail]
	    lappend configSpecs(-label$optTail) [lindex $configSet 3]
	}
	if {[string compare $::tcl_platform(platform) macintosh] == 0} {
	    lappend configSpecs(-labelborderwidth) 1
	} else {
	    set configSet [$helpButton config -borderwidth]
	    lappend configSpecs(-labelborderwidth) [lindex $configSet 3]
	}
	destroy $helpButton

	#
	# Extend the remaining elements of the array configSpecs
	#
	lappend configSpecs(-activestyle)		underline
	lappend configSpecs(-arrowcolor)		{}
	lappend configSpecs(-arrowdisabledcolor)	{}
	lappend configSpecs(-columns)			{0 {} left}
	lappend configSpecs(-incrarrowtype)		up
	lappend configSpecs(-labelcommand)		{}
	lappend configSpecs(-labelrelief)		raised
	lappend configSpecs(-listvariable)		{}
	lappend configSpecs(-resizablecolumns)		1
	lappend configSpecs(-resizecursor)		sb_h_double_arrow
	lappend configSpecs(-showarrow)			1
	lappend configSpecs(-showlabels)		1
	lappend configSpecs(-showseparators)		0
	lappend configSpecs(-sortcommand)		{}
	lappend configSpecs(-stretch)			{}
	lappend configSpecs(-stripebackground)		{}
	lappend configSpecs(-stripeforeground)		{}
	lappend configSpecs(-stripeheight)		1
    }
    extendConfigSpecs

    variable configOpts [lsort [array names configSpecs]]

    #
    # The array colConfigSpecs is used to handle column configuration options.
    # The names of its elements are the column configuration options for the
    # Tablelist widget class.  The value of an array element is either an alias
    # name or a list containing the database name and class.
    #
    #	Command-Line Name	{Database Name		Database Class	}
    #	-----------------------------------------------------------------
    #
    variable colConfigSpecs
    array set colConfigSpecs {
	-align			{align			Align		}
	-background		{background		Background	}
	-bg			-background
	-font			{font			Font		}
	-foreground		{foreground		Foreground	}
	-fg			-foreground
	-formatcommand		{formatCommand		FormatCommand	}
	-hide			{hide			Hide		}
	-labelalign		{labelAlign		Align		}
	-labelbackground	{labelBackground	Background	}
	-labelbg		-labelbackground
	-labelborderwidth	{labelBorderWidth	BorderWidth	}
	-labelbd		-labelborderwidth
	-labelcommand		{labelCommand		LabelCommand	}
	-labelfont		{labelFont		Font		}
	-labelforeground	{labelForeground	Foreground	}
	-labelfg		-labelforeground
	-labelheight		{labelHeight		Height		}
	-labelimage		{labelImage		Image		}
	-labelpady		{labelPadY		Pad		}
	-labelrelief		{labelRelief		Relief		}
	-resizable		{resizable		Resizable	}
	-selectbackground	{selectBackground	Foreground	}
	-selectforeground	{selectForeground	Background	}
	-showarrow		{showArrow		ShowArrow	}
	-sortcommand		{sortCommand		SortCommand	}
	-sortmode		{sortMode		SortMode	}
	-title			{title			Title		}
	-width			{width			Width		}
    }

    #
    # Extend some elements of the array colConfigSpecs
    #
    lappend colConfigSpecs(-align)	- left
    lappend colConfigSpecs(-hide)	- 0
    lappend colConfigSpecs(-resizable)	- 1
    lappend colConfigSpecs(-showarrow)	- 1
    lappend colConfigSpecs(-sortmode)	- ascii
    lappend colConfigSpecs(-width)	- 0

    #
    # The array rowConfigSpecs is used to handle row configuration options.
    # The names of its elements are the row configuration options for the
    # Tablelist widget class.  The value of an array element is either an alias
    # name or a list containing the database name and class.
    #
    #	Command-Line Name	{Database Name		Database Class	}
    #	-----------------------------------------------------------------
    #
    variable rowConfigSpecs
    array set rowConfigSpecs {
	-background		{background		Background	}
	-bg			-background
	-font			{font			Font		}
	-foreground		{foreground		Foreground	}
	-fg			-foreground
	-selectable		{selectable		Selectable	}
	-selectbackground	{selectBackground	Foreground	}
	-selectforeground	{selectForeground	Background	}
	-text			{text			Text		}
    }

    #
    # Extend some elements of the array rowConfigSpecs
    #
    lappend rowConfigSpecs(-selectable)	- 1

    #
    # The array cellConfigSpecs is used to handle cell configuration options.
    # The names of its elements are the cell configuration options for the
    # Tablelist widget class.  The value of an array element is either an alias
    # name or a list containing the database name and class.
    #
    #	Command-Line Name	{Database Name		Database Class	}
    #	-----------------------------------------------------------------
    #
    variable cellConfigSpecs
    array set cellConfigSpecs {
	-background		{background		Background	}
	-bg			-background
	-font			{font			Font		}
	-foreground		{foreground		Foreground	}
	-fg			-foreground
	-image			{image			Image		}
	-selectbackground	{selectBackground	Foreground	}
	-selectforeground	{selectForeground	Background	}
	-text			{text			Text		}
    }

    #
    # Use a list to facilitate the handling of the command options 
    #
    variable cmdOpts [list \
	activate attrib bbox bodypath cellcget cellconfigure cellindex \
	cget columncget columnconfigure columncount columnindex configure \
	curselection delete get index insert insertlist labelpath labels \
	nearest nearestcell nearestcolumn resetsortinfo rowcget rowconfigure \
	scan see selection separatorpath separators size sort sortbycolumn \
	sortcolumn sortorder xview yview]

    #
    # Use lists to facilitate the handling of miscellaneous options
    #
    variable activeStyles	[list underline frame dotbox]
    variable alignments		[list left right center]
    variable arrowTypes		[list up down]
    variable states		[list disabled normal]
    variable sortModes		[list ascii command dictionary integer real]
    variable sortOrders		[list -increasing -decreasing]
    variable scanCmdOpts	[list mark dragto]
    variable selCmdOpts		[list anchor clear includes set]

    #
    # Define some Tablelist class bindings
    #
    bind Tablelist <FocusIn> {
	tablelist::addActiveTag %W

	if {[string compare [focus -lastfor %W] %W] == 0} {
	    focus [%W bodypath]
	}
    }
    bind Tablelist <FocusOut> {
	tablelist::removeActiveTag %W
    }
    bind Tablelist <Destroy> {
	tablelist::cleanup %W
    }

    #
    # Define the binding tag TablelistBody to have the same events as Listbox
    # and the binding scripts obtained from those of Listbox by replacing the
    # widget %W with [winfo parent %W] as well as the %x and %y fields with
    # [expr {%x + [winfo x %W]}] and [expr {%y + [winfo y %W]}], respectively
    # 
    proc defineTablelistBody {} {
	foreach event [bind Listbox] {
	    set script [bind Listbox $event]
	    regsub -all %W $script {$tablelist::win} script
	    regsub -all %x $script {$tablelist::x} script
	    regsub -all %y $script {$tablelist::y} script
	    regsub -all {tk(::)?ListboxAutoScan} $script \
			tablelist::tablelistAutoScan script

	    bind TablelistBody $event [format {
		set tablelist::win [winfo parent %%W]
		set tablelist::x [expr {%%x + [winfo x %%W]}]
		set tablelist::y [expr {%%y + [winfo y %%W]}]
		%s
	    } $script]
	}
    }
    defineTablelistBody

    #
    # Define the virtual event <<Button3>>
    #
    switch $::tcl_platform(platform) {
	unix -
	windows {
	    event add <<Button3>> <Button-3>
	}
	macintosh {
	    event add <<Button3>> <Control-Button-1>
	}
    }
}

#
# Public procedure
# ================
#

#------------------------------------------------------------------------------
# tablelist::tablelist
#
# Creates a new tablelist widget whose name is specified as the first command-
# line argument, and configures it according to the options and their values
# given on the command line.  Returns the name of the newly created widget.
#------------------------------------------------------------------------------
proc tablelist::tablelist args {
    variable configSpecs
    variable configOpts



    if {[llength $args] == 0} {
	mwutil::wrongNumArgs "tablelist pathName ?options?"
    }

    #
    # Create a frame of the class Tablelist
    #
    set win [lindex $args 0]
    if {[catch {
	    frame $win -class Tablelist -colormap . -container 0 \
		       -height 0 -width 0
	} result] != 0} {
	return -code error $result
    }

    #
    # Create a namespace within the current one to hold the data of the widget
    #
    namespace eval ns$win {
	#
	# The folowing array holds various data for this widget
	#
	variable data
	array set data {
	    charWidth		 1
	    hasListVar		 0
	    isDisabled		 0
	    hdrPixels		 0
	    activeIdx		 0
	    anchorIdx		 0
	    seqNum		-1
	    itemList		 {}
	    itemCount		 0
	    lastRow		-1
	    colList		 {0 left}
	    colCount		 1
	    lastCol		 0
	    clickedLblCol	-1
	    arrowCol		-1
	    sortCol		-1
	    sortOrder		 {}
	    forceAdjust		 0
	    0-delta		 0
	    0-hide		 0
	}

	#
	# The following array is used to hold arbitrary
	# attributes and their values for this widget
	#
	variable attribVals
    }
    #
    # Initialize some further components of data
    #
    upvar ::tablelist::ns${win}::data data
    foreach opt $configOpts {
	set data($opt) [lindex $configSpecs($opt) 3]
    }
    set data(colFontList)	[list $data(-font)]
    set data(body)		$win.body
    set data(hdr)		$win.hdr
    set data(hdrTxt)		$data(hdr).t
    set data(hdrTxtFr)		$data(hdrTxt).f
    set data(hdrTxtFrCanv)	$data(hdrTxtFr).c
    set data(hdrTxtFrLbl)	$data(hdrTxtFr).l
    set data(hdrLbl)		$data(hdr).l
    set data(lb)		$win.lb
    set data(sep)		$win.sep

    #
    # Create a child hierarchy used to hold the column labels.  The
    # labels will be created as children of the frame data(hdrTxtFr),
    # which is embedded into the text widget data(hdrTxt) (in order
    # to make it scrollable), which in turn fills the frame data(hdr)
    # (whose width and height can be set arbitrarily in pixels).
    #
    set w $data(hdr)			;# header frame
    frame $w -borderwidth 0 -colormap . -container 0 -height 0 \
	     -highlightthickness 0 -relief flat -takefocus 0 -width 0
    bind $w <Configure> [list tablelist::stretchColumnsWhenIdle $win]
    pack $w -fill x
    set w $data(hdrTxt)			;# text widget within the header frame
    text $w -borderwidth 0 -highlightthickness 0 -insertwidth 0 \
	    -padx 0 -pady 0 -state normal -takefocus 0 -wrap none
    place $w -relheight 1.0 -relwidth 1.0
    frame $data(hdrTxtFr) -borderwidth 0 -colormap . -container 0 -height 0 \
			  -highlightthickness 0 -relief flat -takefocus 0 \
			  -width 0
    $w window create 1.0 -window $data(hdrTxtFr)
    label $data(hdrTxtFrLbl)0 
    set w $data(hdrLbl)			;# filler label within the header frame
    label $w -bitmap "" -highlightthickness 0 -image "" -takefocus 0 \
	     -text "" -textvariable "" -underline -1 -wraplength 0
    place $w -relheight 1.0 -relwidth 1.0

    #
    # Create a canvas as a child of the frame data(hdrTxtFr),
    # needed for displaying an up- or down-arrow when
    # sorting the items by a column.   Set its width and
    # height to temporary values and create two 3-D arrows
    #
    set w $data(hdrTxtFrCanv)
    set size 9
    canvas $w -borderwidth 0 -height $size -highlightthickness 0 \
	      -relief flat -takefocus 0 -width $size
    create3DArrows $w

    #
    # Remove the binding tag Text from the list
    # of binding tags of the header text widget
    #
    set w $data(hdrTxt)
    bindtags $w [list $w [winfo toplevel $w] all]

    #
    # Create the body text widget within the main frame
    #
    set w $data(body)
    text $w -borderwidth 0 -exportselection no -highlightthickness 0 \
	    -insertwidth 0 -padx 0 -pady 0 -state normal \
	    -takefocus tablelist::focusCtrl -wrap none
    bind $w <Configure> [list tablelist::adjustSepsWhenIdle $win]
    pack $w -expand yes -fill both

    #
    # Replace the binding tags Text and all with TablelistBody and allModif,
    # respectively, in the list of binding tags of the body text widget
    # (see mwutil.tcl for the definition of the bindings for the tag allModif)
    #
    bindtags $w [list $w TablelistBody [winfo toplevel $w] allModif]

    #
    # Create the "active", "stripe", "select", and "disabled" tags
    # in the body text widget.  Don't use the built-in "sel" tag
    # because on Windows the selection in a text widget only
    # becomes visible when the window gets the input focus.
    #
    $w tag configure stripe -background "" -foreground ""
    $w tag configure active -borderwidth 1 -underline yes
    $w tag configure select -relief raised
    $w tag configure disabled -underline no

    #
    # Create an unmanaged listbox child, used to handle the -setgrid option
    #
    listbox $data(lb)

    #
    # Configure the widget according to the command-line
    # arguments and to the available database options
    #
    if {[catch {
	    mwutil::configure $win configSpecs data tablelist::doConfig \
			      [lrange $args 1 end] 1
	} result] != 0} {
	destroy $win
	return -code error $result
    }


    #
    # Move the original widget command into the current namespace
    # and build a new widget procedure in the global one
    #
    rename ::$win $win
    proc ::$win args [format {
	if {[catch {tablelist::tablelistWidgetCmd %s $args} result] == 0} {
	    return $result
	} else {
	    return -code error $result
	}
    } [list $win]]

    #
    # Register a callback to be invoked whenever the PRIMARY selection is
    # owned by the window win and someone attempts to retrieve it as a STRING
    #
    selection handle $win [list ::tablelist::fetchSelection $win]

    return $win
}

#
# Private configuration procedures
# ================================
#

#------------------------------------------------------------------------------
# tablelist::doConfig
#
# Applies the value val of the configuration option opt to the tablelist widget
# win.
#------------------------------------------------------------------------------
proc tablelist::doConfig {win opt val} {
    variable helpLabel
    variable configSpecs
    upvar ::tablelist::ns${win}::data data

    #
    # Apply the value to the widget(s) corresponding to the given option
    #
    switch [lindex $configSpecs($opt) 2] {
	c {
	    #
	    # Apply the value to all children and save the
	    # properly formatted value of val in data($opt)
	    #
	    foreach w [winfo children $win] {
		if {[regexp {^(body|hdr|sep[0-9]+)$} [winfo name $w]]} {
		    $w configure $opt $val
		}
	    }
	    $data(hdrTxt) configure $opt $val
	    $data(hdrLbl) configure $opt $val
	    foreach w [winfo children $data(hdrTxtFr)] {
		$w configure $opt $val
		foreach c [winfo children $w] {
		    $c configure $opt $val
		}
	    }
	    $helpLabel configure $opt $val
	    set data($opt) [$helpLabel cget $opt]
	}

	b {
	    #
	    # Apply the value to the body text widget and save
	    # the properly formatted value of val in data($opt)
	    #
	    set w $data(body)
	    $w configure $opt $val
	    set data($opt) [$w cget $opt]

	    switch -- $opt {
		-background {
		    #
		    # Apply the value to the frame (because of the shadow
		    # colors of its 3-D border), to the header frame (this will
		    # make it fully invisible if the labels are not being
		    # shown), to the separators, and to the "disabled" tag
		    #
		    $win configure $opt $val
		    foreach c [winfo children $win] {
			if {[regexp {^(hdr|sep[0-9]+)$} [winfo name $c]]} {
			    $c configure $opt $val
			}
		    }
		    $w tag configure disabled $opt $val
		}
		-font {
		    #
		    # Apply the value to the listbox child, adjust the columns,
		    # rebuild the lists of the column fonts and tag names,
		    # and make sure the items will be redisplayed at idle time
		    #
		    $data(lb) configure $opt $val
		    set data(charWidth) [font measure $val -displayof $win 0]
		    makeColFontAndTagLists $win
		    adjustColumns $win all 1
		    redisplayWhenIdle $win
		}
		-foreground {
		    #
		    # Apply the value to the "disabled" tag if needed
		    #
		    if {[string compare $data(-disabledforeground) ""] == 0} {
			$w tag configure disabled $opt $val
		    }
		}
	    }
	}

	h {
	    #
	    # Apply the value to the header text widget and save
	    # the properly formatted value of val in data($opt)
	    #
	    set w $data(hdrTxt)
	    $w configure $opt $val
	    set data($opt) [$w cget $opt]
	}

	l {
	    #
	    # Apply the value to all not individually configured labels
	    # and save the properly formatted value of val in data($opt)
	    #
	    set optTail [string range $opt 6 end]	;# remove the -label
	    for {set col 0} {$col < $data(colCount)} {incr col} {
		set w $data(hdrTxtFrLbl)$col
		if {![info exists data($col$opt)]} {
		    configLabel $w -$optTail $val
		}
	    }
	    $helpLabel configure -$optTail $val
	    set data($opt) [$helpLabel cget -$optTail]

	    switch -- $opt {
		-labelbackground {
		    #
		    # Apply the value to the label child of the header frame
		    # and conditionally both to the children of the labels (if
		    # any) and to the canvas displaying an up- or down-arrow
		    #
		    $data(hdrLbl) configure -$optTail $val
		    for {set col 0} {$col < $data(colCount)} {incr col} {
			set w $data(hdrTxtFrLbl)$col
			if {![info exists data($col$opt)]} {
			    foreach c [winfo children $w] {
				$c configure -$optTail $val
			    }
			}
		    }
		    if {$data(arrowCol) >= 0 &&
			![info exists data($data(arrowCol)$opt)]} {
			configCanvas $win
		    }
		}
		-labelborderwidth {
		    #
		    # Apply the value to the label child of the
		    # header frame and adjust the columns
		    # (including the height of the header frame)
		    #
		    $data(hdrLbl) configure -$optTail $val
		    adjustColumns $win all 1
		}
		-labeldisabledforeground {
		    #
		    # Apply the value to the children of the labels (if any)
		    #
		    foreach w [winfo children $data(hdrTxtFr)] {
			foreach c [winfo children $w] {
			    $c configure $opt $val
			}
		    }
		}
		-labelfont {
		    #
		    # Conditionally apply the value to the children of
		    # the labels (if any), conditionally resize the canvas
		    # displaying an up- or down-arrow, and adjust the
		    # columns (including the height of the header frame)
		    #
		    for {set col 0} {$col < $data(colCount)} {incr col} {
			set w $data(hdrTxtFrLbl)$col
			if {![info exists data($col$opt)]} {
			    foreach c [winfo children $w] {
				$c configure -$optTail $val
			    }
			}
		    }
		    if {$data(arrowCol) >= 0 &&
			![info exists data($data(arrowCol)$opt)]} {
			configCanvas $win
			drawArrows $win
		    }
		    adjustColumns $win all 1
		}
		-labelforeground {
		    #
		    # Conditionally apply the value to
		    # the children of the labels (if any)
		    #
		    for {set col 0} {$col < $data(colCount)} {incr col} {
			set w $data(hdrTxtFrLbl)$col
			if {![info exists data($col$opt)]} {
			    foreach c [winfo children $w] {
				$c configure -$optTail $val
			    }
			}
		    }
		}
		-labelheight -
		-labelpady {
		    #
		    # Adjust the height of the header frame
		    #
		    adjustHeaderHeight $win
		}
		-labelrelief {
		    #
		    # Apply the value to the label child of the header frame
		    #
		    $data(hdrLbl) configure -$optTail $val
		}
	    }
	}

	f {
	    #
	    # Apply the value to the frame and save the
	    # properly formatted value of val in data($opt)
	    #
	    $win configure $opt $val
	    set data($opt) [$win cget $opt]
	}

	w {
	    switch -- $opt {
		-activestyle {
		    #
		    # Configure the "active" tag and save the
		    # properly formatted value of val in data($opt)
		    #

		    variable activeStyles
		    set val [mwutil::fullOpt "active style" $val $activeStyles]
		    set w $data(body)
		    switch $val {
			underline {
			    $w tag configure active -relief flat -underline 1
			}
			frame {
			    $w tag configure active -relief solid -underline 0
			}

		    }
		    set data($opt) $val
		}
		-arrowcolor {
		    #
		    # Set the color of the normal arrow and save the
		    # properly formatted value of val in data($opt)
		    #
		    set data($opt) [fillArrow $data(hdrTxtFrCanv) normal $val]
		}
		-arrowdisabledcolor {
		    #
		    # Set the color of the disabled arrow and save the
		    # properly formatted value of val in data($opt)
		    #
		    set data($opt) [fillArrow $data(hdrTxtFrCanv) disabled $val]
		}
		-columns {
		    #
		    # Set up and adjust the columns, rebuild
		    # the lists of the column fonts and tag
		    # names, and redisplay the items
		    #
		    setupColumns $win $val 1
		    adjustColumns $win all 1
		    makeColFontAndTagLists $win
		    redisplay $win 0 end
		}
		-disabledforeground {
		    #
		    # Configure the "disabled" tag in the body text widget and
		    # save the properly formatted value of val in data($opt)
		    #
		    set w $data(body)
		    if {[string compare $val ""] == 0} {
			$w tag configure disabled -fgstipple gray50 \
				-foreground $data(-foreground)
			set data($opt) ""
		    } else {
			$w tag configure disabled -fgstipple "" \
				-foreground $val
			set data($opt) [$w tag cget disabled -foreground]
		    }
		}
		-exportselection {
		    #
		    # Save the boolean value specified by val in
		    # data($opt).  In addition, if the selection is
		    # exported and there are any selected rows in the
		    # widget then make win the new owner of the PRIMARY
		    # selection and register a callback to be invoked
		    # when it loses ownership of the PRIMARY selection
		    #
		    set data($opt) [expr {$val ? 1 : 0}]
		    if {$val &&
			[llength [$data(body) tag nextrange select 1.0]] != 0} {
			selection own -command \
				[list ::tablelist::lostSelection $win] $win
		    }
		}
		-height {
		    #
		    # Adjust the height of the body text widget and save
		    # the properly formatted value of val in data($opt)
		    #
		    set val [format %d $val]	;# integer check with error msg
		    if {$val <= 0} {
			$data(body) configure $opt $data(itemCount)
		    } else {
			$data(body) configure $opt $val
		    }
		    set data($opt) $val
		}
		-incrarrowtype {
		    #
		    # Save the properly formatted value of val
		    # in data($opt) and draw the arrows if
		    # the canvas widget is presently mapped
		    #
		    variable arrowTypes
		    set data($opt) \
			[mwutil::fullOpt "arrow type" $val $arrowTypes]
		    if {$data(arrowCol) >= 0} {
			drawArrows $win
		    }
		}
		-labelcommand -
		-selectmode -
		-sortcommand {
		    #
		    # Save val in data($opt)
		    #
		    set data($opt) $val
		}
		-listvariable {
		    #
		    # Associate val as list variable with the
		    # given widget and save it in data($opt)
		    #
		    makeListVar $win $val
		    set data($opt) $val
		    if {[string compare $val ""] == 0} {
			set data(hasListVar) 0
		    } else {
			set data(hasListVar) 1
		    }
		}
		-resizablecolumns {
		    #
		    # Save the boolean value specified by val in data($opt)
		    #
		    set data($opt) [expr {$val ? 1 : 0}]
		}
		-resizecursor {
		    #
		    # Save the properly formatted value of val in data($opt)
		    #
		    $helpLabel configure -cursor $val
		    set data($opt) [$helpLabel cget -cursor]
		}
		-selectbackground -
		-selectforeground {
		    #
		    # Configure the "select" tag in the body text widget
		    # and save the properly formatted value of val in
		    # data($opt).  Don't use the built-in "sel" tag
		    # because on Windows the selection in a text widget only
		    # becomes visible when the window gets the input focus.
		    #
		    set w $data(body)
		    set optTail [string range $opt 7 end] ;# remove the -select
		    $w tag configure select -$optTail $val
		    set data($opt) [$w tag cget select -$optTail]
		}
		-selectborderwidth {
		    #
		    # Configure the "select" tag in the body text widget
		    # and save the properly formatted value of val in
		    # data($opt).  Don't use the built-in "sel" tag
		    # because on Windows the selection in a text widget only
		    # becomes visible when the window gets the input focus.
		    # In addition, adjust the line spacing accordingly and
		    # apply the value to the listbox child, too.
		    #
		    set w $data(body)
		    set optTail [string range $opt 7 end] ;# remove the -select
		    $w tag configure select -$optTail $val
		    set data($opt) [$w tag cget select -$optTail]
		    if {$val < 0} {
			set val 0
		    }
		    $w configure -spacing1 $val -spacing3 [expr {$val + 1}]
		    $data(lb) configure $opt $val
		}
		-setgrid {
		    #
		    # Apply the value to the listbox child and save
		    # the properly formatted value of val in data($opt)
		    #
		    $data(lb) configure $opt $val
		    set data($opt) [$data(lb) cget $opt]
		}
		-showarrow {
		    #
		    # Save the boolean value specified by val in
		    # data($opt) and conditionally unmanage the
		    # canvas displaying an up- or down-arrow
		    #
		    set data($opt) [expr {$val ? 1 : 0}]
		    if {!$data($opt)} {
			place forget $data(hdrTxtFrCanv)
			set colOfUnknownWidth $data(arrowCol)
			set data(arrowCol) -1
			adjustColumns $win $colOfUnknownWidth 1
		    }
		}
		-showlabels {
		    #
		    # Save the boolean value specified by val in data($opt)
		    # and adjust the height of the header frame
		    #
		    set data($opt) [expr {$val ? 1 : 0}]
		    adjustHeaderHeight $win
		}
		-showseparators {
		    #
		    # Save the boolean value specified by val in data($opt),
		    # and create or destroy the separators if needed
		    #
		    set oldVal $data($opt)
		    set data($opt) [expr {$val ? 1 : 0}]
		    if {!$oldVal && $data($opt)} {
			createSeps $win
		    } elseif {$oldVal && !$data($opt)} {
			foreach w [winfo children $win] {
			    if {[regexp {^sep[0-9]+$} [winfo name $w]]} {
				destroy $w
			    }
			}
		    }
		}
		-state {
		    #
		    # Apply the value to all labels and their children (if
		    # any), raise the corresponding arrow in the canvas,
		    # add/remove the "disabled" tag to/from the contents
		    # of the body text widget, configure the borderwidth
		    # of the "active" and "select" tags, and save the
		    # properly formatted value of val in data($opt)
		    #
		    variable states
		    set val [mwutil::fullOpt "state" $val $states]
		    catch {
			foreach w [winfo children $data(hdrTxtFr)] {
			    $w configure $opt $val
			    foreach c [winfo children $w] {
				$c configure $opt $val
			    }
			}
		    }
		    raiseArrow $data(hdrTxtFrCanv) $val
		    set w $data(body)
		    switch $val {
			disabled {
			    $w tag add disabled 1.0 end
			    $w tag configure active -borderwidth 0
			    $w tag configure select -borderwidth 0
			    set data(isDisabled) 1
			}
			normal {
			    $w tag remove disabled 1.0 end
			    $w tag configure active -borderwidth 1
			    $w tag configure select -borderwidth \
				    $data(-selectborderwidth)
			    set data(isDisabled) 0
			}
		    }
		    set data($opt) $val
		}
		-stretch {
		    #
		    # Save val in data($opt) and
		    # stretch the stretchable columns
		    #
		    if {[string first $val all] == 0} {
			set data($opt) all
		    } else {
			foreach col $val {
			    colIndex $win $col 0 ;# index check with error msg
			}
			set data($opt) $val
		    }
		    set data(forceAdjust) 1
		    stretchColumnsWhenIdle $win
		}
		-stripebackground -
		-stripeforeground {
		    #
		    # Configure the "stripe" tag in the body text
		    # widget, save the properly formatted value of val
		    # in data($opt), and draw the stripes if necessary
		    #
		    set w $data(body)
		    set optTail [string range $opt 7 end] ;# remove the -stripe
		    $w tag configure stripe -$optTail $val
		    set data($opt) [$w tag cget stripe -$optTail]
		    makeStripesWhenIdle $win
		}
		-stripeheight {
		    #
		    # Save the properly formatted value of val val
		    # data($opt) and draw the stripes if necessary
		    #
		    set val [format %d $val]	;# integer check with error msg
		    set data($opt) $val
		    makeStripesWhenIdle $win
		}
		-width {
		    #
		    # Adjust the widths of the body text widget
		    # and of the header frame, and save the
		    # properly formatted value of val in data($opt)
		    #
		    set val [format %d $val]	;# integer check with error msg
		    $data(body) configure $opt $val
		    if {$val <= 0} {
			$data(hdr) configure $opt $data(hdrPixels)
		    } else {
			$data(hdr) configure $opt 0
		    }
		    set data($opt) $val
		}
	    }
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::doColConfig
#
# Applies the value val of the column configuration option opt to the col'th
# column of the tablelist widget win.
#------------------------------------------------------------------------------
proc tablelist::doColConfig {col win opt val} {
    upvar ::tablelist::ns${win}::data data

    switch -- $opt {
	-align {
	    #
	    # Set up and adjust the columns, and make sure the
	    # given column will be redisplayed at idle time
	    #
	    set idx [expr {3*$col + 2}]
	    setupColumns $win [lreplace $data(-columns) $idx $idx $val] 0
	    adjustColumns $win -1 1
	    redisplayColWhenIdle $win $col
	}

	-background -
	-foreground {
	    set w $data(body)
	    set tag $col$opt
	    if {[string compare $val ""] == 0} {
		if {[info exists data($tag)]} {
		    $w tag delete $tag
		    unset data($tag)
		}
	    } else {
		#
		# Configure the tag $tag in the body text widget
		#
		$w tag configure $tag $opt $val
		$w tag raise $tag stripe

		if {!$data($col-hide)} {
		    #
		    # Apply the tag to the elements of the given column
		    #
		    for {set idx 0; set line 1} {$idx < $data(itemCount)} \
			{incr idx; incr line} {
			if {[lsearch -exact [$w tag names $line.0] select]
			    < 0} {
			    findCellTabs $win $line.0 $col tabIdx1 tabIdx2
			    $w tag add $tag $tabIdx1 $tabIdx2+1c
			}
		    }
		}

		#
		# Save the properly formatted value of val in data($tag)
		#
		set data($tag) [$w tag cget $tag $opt]
	    }

	    #
	    # Rebuild the lists of the column fonts and tag names
	    #
	    makeColFontAndTagLists $win
	}

	-font {
	    set w $data(body)
	    set tag $col$opt
	    if {[string compare $val ""] == 0} {
		if {[info exists data($tag)]} {
		    $w tag delete $tag
		    unset data($tag)
		}
	    } else {
		#
		# Configure the tag $tag in the body text widget
		#
		$w tag configure $tag $opt $val
		$w tag lower $tag

		if {!$data($col-hide)} {
		    #
		    # Apply the tag to the elements of the given column
		    #
		    for {set idx 0; set line 1} {$idx < $data(itemCount)} \
			{incr idx; incr line} {
			findCellTabs $win $line.0 $col tabIdx1 tabIdx2
			$w tag add $tag $tabIdx1 $tabIdx2+1c
		    }
		}

		#
		# Save the properly formatted value of val in data($tag)
		#
		set data($tag) [$w tag cget $tag $opt]
	    }

	    #
	    # Rebuild the lists of the column fonts and tag names
	    #
	    makeColFontAndTagLists $win

	    #
	    # Adjust the columns, and make sure the specified
	    # column will be redisplayed at idle time if needed
	    #
	    adjustColumns $win $col 1
	    set pixels [lindex $data(colList) [expr {2*$col}]]
	    if {$pixels != 0} {			;# convention: static width
		redisplayColWhenIdle $win $col
	    }
	}

	-formatcommand {
	    if {[string compare $val ""] == 0} {
		if {[info exists data($col$opt)]} {
		    unset data($col$opt)
		}
		set fmtCmdFlag 0
	    } else {
		#
		# Save val in data($col$opt)
		#
		set data($col$opt) $val
		set fmtCmdFlag 1
	    }

	    #
	    # Update the corresponding element of the list data(fmtCmdFlagList)
	    #
	    set data(fmtCmdFlagList) \
		[lreplace $data(fmtCmdFlagList) $col $col $fmtCmdFlag]

	    #
	    # Adjust the columns and make sure the specified
	    # column will be redisplayed at idle time
	    #
	    adjustColumns $win $col 1
	    redisplayColWhenIdle $win $col
	}

	-hide {
	    #
	    # Save the boolean value specified by val in data($col$opt),
	    # adjust the columns, and redisplay the items
	    #
	    set data($col$opt) [expr {$val ? 1 : 0}]
	    adjustColumns $win $col 1
	    redisplay $win 0 end
	}

	-labelalign {
	    if {[string compare $val ""] == 0} {
		#
		# Unset data($col$opt)
		#
		set alignment [lindex $data(colList) [expr {2*$col + 1}]]
		if {[info exists data($col$opt)]} {
		    unset data($col$opt)
		}
	    } else {
		#
		# Save the properly formatted value of val in data($col$opt)
		#
		variable alignments
		set val [mwutil::fullOpt "label alignment" $val $alignments]
		set alignment $val
		set data($col$opt) $val
	    }

	    #
	    # Adjust the col`th label
	    #
	    set pixels [lindex $data(colList) [expr {2*$col}]]
	    if {$pixels != 0} {			;# convention: static width
		incr pixels $data($col-delta)
	    }
	    adjustLabel $win $col $pixels $alignment
	}

	-labelbackground {
	    set w $data(hdrTxtFrLbl)$col
	    set optTail [string range $opt 6 end]	;# remove the -label
	    if {[string compare $val ""] == 0} {
		#
		# Apply the value of the corresponding widget configuration
		# option to the col'th label and its children (if any)
		# and conditionally to the canvas displaying an up-
		# or down-arrow, and unset data($col$opt)
		#
		$w configure -$optTail $data($opt)
		foreach c [winfo children $w] {
		    $c configure -$optTail $data($opt)
		}
		if {$col == $data(arrowCol)} {
		    configCanvas $win
		}
		if {[info exists data($col$opt)]} {
		    unset data($col$opt)
		}
	    } else {
		#
		# Apply the given value to the col'th label and its
		# children (if any) and conditionally to the canvas
		# displaying an up- or down-arrow, and save the
		# properly formatted value of val in data($col$opt)
		#
		$w configure -$optTail $val
		foreach c [winfo children $w] {
		    $c configure -$optTail $val
		}
		if {$col == $data(arrowCol)} {
		    configCanvas $win
		}
		set data($col$opt) [$w cget -$optTail]
	    }
	}

	-labelborderwidth {
	    set w $data(hdrTxtFrLbl)$col
	    set optTail [string range $opt 6 end]	;# remove the -label
	    if {[string compare $val ""] == 0} {
		#
		# Apply the value of the corresponding widget configuration
		# option to the col'th label and unset data($col$opt)
		#
		configLabel $w -$optTail $data($opt)
		if {[info exists data($col$opt)]} {
		    unset data($col$opt)
		}
	    } else {
		#
		# Apply the given value to the col'th label and save the
		# properly formatted value of val in data($col$opt)
		#
		configLabel $w -$optTail $val
		set data($col$opt) [$w cget -$optTail]
	    }

	    #
	    # Adjust the columns (including the height of the header frame)
	    #
	    adjustColumns $win $col 1
	}

	-labelcommand -
	-sortcommand {
	    if {[string compare $val ""] == 0} {
		if {[info exists data($col$opt)]} {
		    unset data($col$opt)
		}
	    } else {
		#
		# Save val in data($col$opt)
		#
		set data($col$opt) $val
	    }
	}

	-labelfont {
	    set w $data(hdrTxtFrLbl)$col
	    set optTail [string range $opt 6 end]	;# remove the -label
	    if {[string compare $val ""] == 0} {
		#
		# Apply the value of the corresponding widget
		# configuration option to the col'th label and
		# its children (if any), and unset data($col$opt)
		#
		$w configure -$optTail $data($opt)
		foreach c [winfo children $w] {
		    $c configure -$optTail $data($opt)
		}
		if {[info exists data($col$opt)]} {
		    unset data($col$opt)
		}
	    } else {
		#
		# Apply the given value to the col'th label and
		# its children (if any), and save the properly
		# formatted value of val in data($col$opt)
		#
		$w configure -$optTail $val
		foreach c [winfo children $w] {
		    $c configure -$optTail $val
		}
		set data($col$opt) [$w cget -$optTail]
	    }

	    #
	    # Conditionally resize the canvas displaying an up- or down-arrow
	    # and adjust the columns (including the height of the header frame)
	    #
	    if {$col == $data(arrowCol)} {
		configCanvas $win
		drawArrows $win
	    }
	    adjustColumns $win $col 1
	}

	-labelforeground {
	    set w $data(hdrTxtFrLbl)$col
	    set optTail [string range $opt 6 end]	;# remove the -label
	    if {[string compare $val ""] == 0} {
		#
		# Apply the value of the corresponding widget
		# configuration option to the col'th label and
		# its children (if any), and unset data($col$opt)
		#
		$w configure -$optTail $data($opt)
		foreach c [winfo children $w] {
		    $c configure -$optTail $data($opt)
		}
		if {[info exists data($col$opt)]} {
		    unset data($col$opt)
		}
	    } else {
		#
		# Apply the given value to the col'th label and
		# its children (if any), and save the properly
		# formatted value of val in data($col$opt)
		#
		$w configure -$optTail $val
		foreach c [winfo children $w] {
		    $c configure -$optTail $val
		}
		set data($col$opt) [$w cget -$optTail]
	    }
	}

	-labelheight -
	-labelpady {
	    set w $data(hdrTxtFrLbl)$col
	    set optTail [string range $opt 6 end]	;# remove the -label
	    if {[string compare $val ""] == 0} {
		#
		# Apply the value of the corresponding widget configuration
		# option to the col'th label and unset data($col$opt)
		#
		configLabel $w -$optTail $data($opt)
		if {[info exists data($col$opt)]} {
		    unset data($col$opt)
		}
	    } else {
		#
		# Apply the given value to the col'th label and save the
		# properly formatted value of val in data($col$opt)
		#
		configLabel $w -$optTail $val
		set data($col$opt) [$w cget -$optTail]
	    }

	    #
	    # Adjust the height of the header frame
	    #
	    adjustHeaderHeight $win
	}

	-labelimage {
	    set w $data(hdrTxtFrLbl)$col
	    if {[string compare $val ""] == 0} {
		foreach c [winfo children $w] {
		    destroy $c
		}
		if {[info exists data($col$opt)]} {
		    unset data($col$opt)
		}
	    } else {
		if {![winfo exists $w.il]} {
		    variable configSpecs
		    variable configOpts
		    foreach c [list $w.il $w.tl] {	;# image and text labels
			#
			# Create the label $c
			#
			label $c -borderwidth 0 -height 0 \
				 -highlightthickness 0 -padx 0 \
				 -pady 0 -takefocus 0 -width 0

			#
			# Apply to it the current configuration options
			#
			foreach opt $configOpts {
			    if {[llength $configSpecs($opt)] != 1 &&
				[string compare [lindex $configSpecs($opt) 2] c]
				== 0} {
				$c configure $opt $data($opt)
			    }
			}
			foreach opt {-background -font -foreground} {
			    $c configure $opt [$w cget $opt]
			}
			foreach opt {-disabledforeground -state} {
			    catch {$c configure $opt [$w cget $opt]}
			}
			set opt -labelimage	;# restore the original value

			#
			# Define for the binding tag $c the binding scripts
			# obtained from those of $w by replacing the %x
			# field with [expr {%x + [winfo x $c]}] and the
			# %y field with [expr {%y + [winfo y $c]}]
			#
			set _c [list $c]
			foreach event [bind $w] {
			    set script [bind $w $event]
			    regsub -all %x $script {$tablelist::x} script
			    regsub -all %y $script {$tablelist::y} script

			    bind $c $event [format {
				set tablelist::x [expr {%%x + [winfo x %s]}]
				set tablelist::y [expr {%%y + [winfo y %s]}]
				%s
			    } $_c $_c $script]
			}
		    }
		}

		#
		# Display the specified image in the label
		# $w.il and save val in data($col$opt)
		#
		$w.il configure -image $val
		set data($col$opt) $val
	    }

	    #
	    # Adjust the columns (including the height of the header frame)
	    #
	    adjustColumns $win $col 1
	}

	-labelrelief {
	    set w $data(hdrTxtFrLbl)$col
	    set optTail [string range $opt 6 end]	;# remove the -label
	    if {[string compare $val ""] == 0} {
		#
		# Apply the value of the corresponding widget configuration
		# option to the col'th label and unset data($col$opt)
		#
		configLabel $w -$optTail $data($opt)
		if {[info exists data($col$opt)]} {
		    unset data($col$opt)
		}
	    } else {
		#
		# Apply the given value to the col'th label and save the
		# properly formatted value of val in data($col$opt)
		#
		configLabel $w -$optTail $val
		set data($col$opt) [$w cget -$optTail]
	    }
	}

	-resizable {
	    #
	    # Save the boolean value specified by val in data($col$opt)
	    #
	    set data($col$opt) [expr {$val ? 1 : 0}]
	}

	-selectbackground -
	-selectforeground {
	    set w $data(body)
	    set tag $col$opt
	    if {[string compare $val ""] == 0} {
		if {[info exists data($tag)]} {
		    $w tag delete $tag
		    unset data($tag)
		}
	    } else {
		#
		# Configure the tag $tag in the body text widget
		#
		set optTail [string range $opt 7 end]	;# remove the -select
		$w tag configure $tag -$optTail $val
		$w tag raise $tag select

		if {!$data($col-hide)} {
		    #
		    # Apply the tag to the elements of the given
		    # column in all currently selected rows
		    #
		    set selRange [$w tag nextrange select 1.0]
		    while {[llength $selRange] != 0} {
			set selStart [lindex $selRange 0]
			set selEnd [lindex $selRange 1]

			findCellTabs $win $selStart $col tabIdx1 tabIdx2
			$w tag add $tag $tabIdx1 $tabIdx2+1c

			set selRange [$w tag nextrange select $selEnd]
		    }
		}

		#
		# Save the properly formatted value of val in data($tag)
		#
		set data($tag) [$w tag cget $tag -$optTail]
	    }
	}

	-showarrow {
	    #
	    # Save the boolean value specified by val in data($col$opt) and
	    # conditionally unmanage the canvas displaying an up- or down-arrow
	    #
	    set data($col$opt) [expr {$val ? 1 : 0}]
	    if {!$data($col$opt) && $col == $data(arrowCol)} {
		place forget $data(hdrTxtFrCanv)
		set data(arrowCol) -1
		adjustColumns $win $col 1
	    }
	}

	-sortmode {
	    #
	    # Save the properly formatted value of val in data($col$opt)
	    #
	    variable sortModes
	    set data($col$opt) [mwutil::fullOpt "sort mode" $val $sortModes]
	}

	-title {
	    #
	    # Save the given value in the corresponding
	    # element of data(-columns) and adjust the columns
	    #
	    set idx [expr {3*$col + 1}]
	    set data(-columns) [lreplace $data(-columns) $idx $idx $val]
	    adjustColumns $win $col 1
	}

	-width {
	    #
	    # Set up and adjust the columns, and make sure the
	    # given column will be redisplayed at idle time
	    #
	    set idx [expr {3*$col}]
	    setupColumns $win [lreplace $data(-columns) $idx $idx $val] 0
	    adjustColumns $win $col 1
	    redisplayColWhenIdle $win $col
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::doRowConfig
#
# Applies the value val of the row configuration option opt to the row'th row
# of the tablelist widget win.
#------------------------------------------------------------------------------
proc tablelist::doRowConfig {row win opt val} {
    upvar ::tablelist::ns${win}::data data

    set w $data(body)

    switch -- $opt {
	-background -
	-foreground {
	    set item [lindex $data(itemList) $row]
	    set key [lindex $item end]
	    set tag $key$opt

	    if {[string compare $val ""] == 0} {
		if {[info exists data($tag)]} {
		    $w tag delete $tag
		    unset data($tag)
		}
	    } else {
		#
		# Configure the tag $tag in the body text widget and
		# apply it to the given row if it is not selected
		#
		$w tag configure $tag $opt $val
		$w tag lower $tag disabled
		for {set col 0} {$col < $data(colCount)} {incr col} {
		    if {[info exists data($key-$col$opt)]} {
			$w tag lower $key-$col$opt disabled
		    }
		}
		set line [expr {$row + 1}]
		if {[lsearch -exact [$w tag names $line.0] select] < 0} {
		    $w tag add $tag $line.0 $line.end
		}

		#
		# Save the properly formatted value of val in data($tag)
		#
		set data($tag) [$w tag cget $tag $opt]
	    }
	}

	-font {
	    #
	    # Save the current cell fonts in a temporary array
	    #
	    set item [lindex $data(itemList) $row]
	    set key [lindex $item end]
	    for {set col 0} {$col < $data(colCount)} {incr col} {
		set oldCellFonts($col) [cellFont $win $key $col]
	    }

	    set tag $key$opt
	    if {[string compare $val ""] == 0} {
		if {[info exists data($tag)]} {
		    $w tag delete $tag
		    unset data($tag)
		}
	    } else {
		#
		# Configure the tag $tag in the body text widget and
		# apply it to the given row if it is not selected
		#
		$w tag configure $tag $opt $val
		for {set col 0} {$col < $data(colCount)} {incr col} {
		    if {[info exists data($key-$col$opt)]} {
			$w tag raise $key-$col$opt
		    }
		}
		set line [expr {$row + 1}]
		$w tag add $tag $line.0 $line.end

		#
		# Save the properly formatted value of val in data($tag)
		#
		set data($tag) [$w tag cget $tag $opt]
	    }

	    set colWidthsChanged 0
	    set line [expr {$row + 1}]
	    set textIdx1 $line.1
	    set col 0
	    foreach text [lrange $item 0 $data(lastCol)] \
		    fmtCmdFlag $data(fmtCmdFlagList) \
		    {pixels alignment} $data(colList) {
		if {$data($col-hide)} {
		    incr col
		    continue
		}

		#
		# Adjust the cell text and the image width
		#
		if {$fmtCmdFlag} {
		    set text [doFormat $data($col-formatcommand) $text]
		}
		if {[info exists data($key-$col-image)]} {
		    set image $data($key-$col-image)
		    set imageWidth [image width $image]
		} else {
		    set image ""
		    set imageWidth 0
		}
		if {$pixels != 0} {		;# convention: static width
		    incr pixels $data($col-delta)
		}
		set cellFont [cellFont $win $key $col]
		adjustElem $win text imageWidth $cellFont $pixels $alignment

		#
		# Delete the old cell contents between the
		# two tabs, and insert the text and the image
		#
		set textIdx2 [$w search \t $textIdx1 $line.end]
		$w delete $textIdx1 $textIdx2
		insertElem $w $textIdx1 $text $image $imageWidth $alignment

		if {$pixels == 0} {		;# convention: dynamic width
		    #
		    # Check whether the width of the current column has changed
		    #
		    set textWidth [font measure $cellFont -displayof $win $text]
		    set newElemWidth [expr {$imageWidth + $textWidth}]
		    if {$newElemWidth > $data($col-width)} {
			set colWidthsChanged 1
		    } else {
			set oldTextWidth [font measure $oldCellFonts($col) \
					  -displayof $win $text]
			set oldElemWidth [expr {$imageWidth + $oldTextWidth}]
			if {$oldElemWidth == $data($col-width) &&
			    $newElemWidth < $oldElemWidth &&
			    [incr data($col-widestCnt) -1] == 0} {
			    set colWidthsChanged 1
			}
		    }
		}

		set textIdx1 [$w search \t $textIdx1 $line.end]+2c
		incr col
	    }

	    #
	    # Adjust the columns if necessary
	    #
	    if {$colWidthsChanged} {
		adjustColumns $win all 1
	    }
	}

	-selectable {
	    set val [expr {$val ? 1 : 0}]
	    set item [lindex $data(itemList) $row]
	    set key [lindex $item end]

	    if {$val} {
		if {[info exists data($key$opt)]} {
		    unset data($key$opt)
		}
	    } else {
		#
		# Set data($key$opt) to 0 and deselect the row
		#
		set data($key$opt) 0
		selectionSubCmd $win clear $row $row 2
	    }
	}

	-selectbackground -
	-selectforeground {
	    set item [lindex $data(itemList) $row]
	    set key [lindex $item end]
	    set tag $key$opt

	    if {[string compare $val ""] == 0} {
		if {[info exists data($tag)]} {
		    $w tag delete $tag
		    unset data($tag)
		}
	    } else {
		#
		# Configure the tag $tag in the body text widget
		# and apply it to the given row if it is selected
		#
		set optTail [string range $opt 7 end]	;# remove the -select
		$w tag configure $tag -$optTail $val
		$w tag lower $tag disabled
		for {set col 0} {$col < $data(colCount)} {incr col} {
		    if {[info exists data($key-$col$opt)]} {
			$w tag lower $key-$col$opt disabled
		    }
		}
		set line [expr {$row + 1}]
		if {[lsearch -exact [$w tag names $line.0] select] >= 0} {
		    $w tag add $tag $line.0 $line.end
		}

		#
		# Save the properly formatted value of val in data($tag)
		#
		set data($tag) [$w tag cget $tag -$optTail]
	    }
	}

	-text {
	    if {$data(isDisabled)} {
		return ""
	    }

	    set colWidthsChanged 0
	    set oldItem [lindex $data(itemList) $row]
	    set key [lindex $oldItem end]
	    set newItem [adjustItem $val $data(colCount)]
	    set line [expr {$row + 1}]
	    set textIdx1 $line.1
	    set col 0
	    foreach text $newItem \
		    fmtCmdFlag $data(fmtCmdFlagList) \
		    {pixels alignment} $data(colList) {
		if {$data($col-hide)} {
		    incr col
		    continue
		}

		#
		# Adjust the cell text and the image width
		#
		if {$fmtCmdFlag} {
		    set text [doFormat $data($col-formatcommand) $text]
		}
		if {[info exists data($key-$col-image)]} {
		    set image $data($key-$col-image)
		    set imageWidth [image width $image]
		} else {
		    set image ""
		    set imageWidth 0
		}
		if {$pixels != 0} {		;# convention: static width
		    incr pixels $data($col-delta)
		}
		set cellFont [cellFont $win $key $col]
		adjustElem $win text imageWidth $cellFont $pixels $alignment

		#
		# Delete the old cell contents between the
		# two tabs, and insert the text and the image
		#
		set textIdx2 [$w search \t $textIdx1 $line.end]
		$w delete $textIdx1 $textIdx2
		insertElem $w $textIdx1 $text $image $imageWidth $alignment

		if {$pixels == 0} {		;# convention: dynamic width
		    #
		    # Check whether the width of the current column has changed
		    #
		    set textWidth [font measure $cellFont -displayof $win $text]
		    set newElemWidth [expr {$imageWidth + $textWidth}]
		    if {$newElemWidth > $data($col-width)} {
			set colWidthsChanged 1
		    } else {
			set oldText [lindex $oldItem $col]
			if {$fmtCmdFlag} {
			    set oldText \
				[doFormat $data($col-formatcommand) $oldText]
			}
			adjustElem $win oldText imageWidth \
				   $cellFont $pixels $alignment
			set oldTextWidth \
			    [font measure $cellFont -displayof $win $oldText]
			set oldElemWidth [expr {$imageWidth + $oldTextWidth}]
			if {$oldElemWidth == $data($col-width) &&
			    $newElemWidth < $oldElemWidth &&
			    [incr data($col-widestCnt) -1] == 0} {
			    set colWidthsChanged 1
			}
		    }
		}

		set textIdx1 [$w search \t $textIdx1 $line.end]+2c
		incr col
	    }

	    #
	    # Replace the row contents in the list variable if present
	    #
	    if {$data(hasListVar)} {
		set traceCmd [list tablelist::traceProc $win]
		trace vdelete ::$data(-listvariable) wu $traceCmd
		upvar #0 $data(-listvariable) var
		set var [lreplace $var $row $row $newItem]
		trace variable ::$data(-listvariable) wu $traceCmd
	    }

	    #
	    # Replace the row contents in the internal list
	    #
	    lappend newItem [lindex $oldItem end]
	    set data(itemList) [lreplace $data(itemList) $row $row $newItem]

	    #
	    # Adjust the columns if necessary
	    #
	    if {$colWidthsChanged} {
		adjustColumns $win all 1
	    }
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::doCellConfig
#
# Applies the value val of the cell configuration option opt to the cell
# row,col of the tablelist widget win.
#------------------------------------------------------------------------------
proc tablelist::doCellConfig {row col win opt val} {
    upvar ::tablelist::ns${win}::data data

    set w $data(body)

    switch -- $opt {
	-background -
	-foreground {
	    set item [lindex $data(itemList) $row]
	    set key [lindex $item end]
	    set tag $key-$col$opt

	    if {[string compare $val ""] == 0} {
		if {[info exists data($tag)]} {
		    $w tag delete $tag
		    unset data($tag)
		}
	    } else {
		#
		# Configure the tag $tag in the body text widget
		#
		$w tag configure $tag $opt $val
		$w tag lower $tag disabled

		if {!$data($col-hide)} {
		    #
		    # Apply the tag to the given cell if it is not selected
		    #
		    set line [expr {$row + 1}]
		    if {[lsearch -exact [$w tag names $line.0] select] < 0} {
			findCellTabs $win $line.0 $col tabIdx1 tabIdx2
			$w tag add $tag $tabIdx1 $tabIdx2+1c
		    }
		}

		#
		# Save the properly formatted value of val in data($tag)
		#
		set data($tag) [$w tag cget $tag $opt]
	    }
	}

	-font {
	    #
	    # Save the current cell font
	    #
	    set item [lindex $data(itemList) $row]
	    set key [lindex $item end]
	    set oldCellFont [cellFont $win $key $col]

	    set tag $key-$col$opt
	    if {[string compare $val ""] == 0} {
		if {[info exists data($tag)]} {
		    $w tag delete $tag
		    unset data($tag)
		}
	    } else {
		#
		# Configure the tag $tag in the body text widget
		#
		$w tag configure $tag $opt $val
		$w tag raise $tag

		if {!$data($col-hide)} {
		    #
		    # Apply the tag to the given cell if it is not selected
		    #
		    findCellTabs $win [expr {$row + 1}].0 $col tabIdx1 tabIdx2
		    $w tag add $tag $tabIdx1 $tabIdx2+1c
		}

		#
		# Save the properly formatted value of val in data($tag)
		#
		set data($tag) [$w tag cget $tag $opt]
	    }

	    set pixels [lindex $data(colList) [expr {2*$col}]]
	    if {$pixels != 0} {			;# convention: static width
		incr pixels $data($col-delta)
	    }
	    set alignment [lindex $data(colList) [expr {2*$col + 1}]]

	    #
	    # Adjust the cell text and the image width
	    #
	    set text [lindex $item $col]
	    set fmtCmdFlag [lindex $data(fmtCmdFlagList) $col]
	    if {$fmtCmdFlag} {
		set text [doFormat $data($col-formatcommand) $text]
	    }
	    if {[info exists data($key-$col-image)]} {
		set image $data($key-$col-image)
		set imageWidth [image width $image]
	    } else {
		set image ""
		set imageWidth 0
	    }
	    set key [lindex $item end]
	    set cellFont [cellFont $win $key $col]
	    adjustElem $win text imageWidth $cellFont $pixels $alignment

	    if {!$data($col-hide)} {
		#
		# Delete the old cell contents between the
		# two tabs, and insert the text and the image
		#
		findCellTabs $win [expr {$row + 1}].0 $col tabIdx1 tabIdx2
		$w delete $tabIdx1+1c $tabIdx2
		insertElem $w $tabIdx1+1c $text $image $imageWidth $alignment
	    }

	    #
	    # Adjust the columns if necessary
	    #
	    if {$pixels == 0} {			;# convention: dynamic width
		set textWidth [font measure $cellFont -displayof $win $text]
		set newElemWidth [expr {$imageWidth + $textWidth}]
		if {$newElemWidth > $data($col-width)} {
		    set data($col-width) $newElemWidth
		    set data($col-widestCnt) 1
		    adjustColumns $win -1 1
		} else {
		    set oldTextWidth \
			[font measure $oldCellFont -displayof $win $text]
		    set oldElemWidth [expr {$imageWidth + $oldTextWidth}]
		    if {$oldElemWidth == $data($col-width) &&
			$newElemWidth < $oldElemWidth &&
			[incr data($col-widestCnt) -1] == 0} {
			adjustColumns $win $col 1
		    }
		}
	    }
	}

	-image {
	    if {$data(isDisabled)} {
		return ""
	    }

	    set pixels [lindex $data(colList) [expr {2*$col}]]
	    if {$pixels != 0} {			;# convention: static width
		incr pixels $data($col-delta)
	    }
	    set alignment [lindex $data(colList) [expr {2*$col + 1}]]

	    #
	    # Adjust the cell text and the image width
	    #
	    set item [lindex $data(itemList) $row]
	    set text [lindex $item $col]
	    set fmtCmdFlag [lindex $data(fmtCmdFlagList) $col]
	    if {$fmtCmdFlag} {
		set text [doFormat $data($col-formatcommand) $text]
	    }
	    set oldText $text			;# will be needed later
	    set image $val
	    if {[string compare $image ""] == 0} {
		set imageWidth 0
	    } else {
		set imageWidth [image width $image]
	    }
	    set key [lindex $item end]
	    set cellFont [cellFont $win $key $col]
	    adjustElem $win text imageWidth $cellFont $pixels $alignment

	    if {!$data($col-hide)} {
		#
		# Delete the old cell contents between the
		# two tabs, and insert the text and the image
		#
		findCellTabs $win [expr {$row + 1}].0 $col tabIdx1 tabIdx2
		$w delete $tabIdx1+1c $tabIdx2
		insertElem $w $tabIdx1+1c $text $image $imageWidth $alignment
	    }

	    #
	    # Save the old image width
	    #
	    set name $key-$col$opt
	    if {[info exists data($name)]} {
		set oldImageWidth [image width $data($name)]
	    } else {
		set oldImageWidth 0
	    }

	    #
	    # Delete data($name) or save the specified value in it
	    #
	    if {[string compare $val ""] == 0} {
		if {[info exists data($name)]} {
		    unset data($name)
		}
	    } else {
		set data($name) $val
	    }

	    #
	    # Adjust the columns if necessary
	    #
	    if {$pixels == 0} {			;# convention: dynamic width
		set textWidth [font measure $cellFont -displayof $win $text]
		set newElemWidth [expr {$imageWidth + $textWidth}]
		if {$newElemWidth > $data($col-width)} {
		    set data($col-width) $newElemWidth
		    set data($col-widestCnt) 1
		    adjustColumns $win -1 1
		} else {
		    adjustElem $win oldText oldImageWidth \
			       $cellFont $pixels $alignment
		    set oldTextWidth \
			[font measure $cellFont -displayof $win $oldText]
		    set oldElemWidth [expr {$oldImageWidth + $oldTextWidth}]
		    if {$oldElemWidth == $data($col-width) &&
			$newElemWidth < $oldElemWidth &&
			[incr data($col-widestCnt) -1] == 0} {
			adjustColumns $win $col 1
		    }
		}
	    }
	}

	-selectbackground -
	-selectforeground {
	    set item [lindex $data(itemList) $row]
	    set key [lindex $item end]
	    set tag $key-$col$opt

	    if {[string compare $val ""] == 0} {
		if {[info exists data($tag)]} {
		    $w tag delete $tag
		    unset data($tag)
		}
	    } else {
		#
		# Configure the tag $tag in the body text widget
		#
		set optTail [string range $opt 7 end]	;# remove the -select
		$w tag configure $tag -$optTail $val
		$w tag lower $tag disabled

		if {!$data($col-hide)} {
		    #
		    # Apply the tag to the given cell if it is selected
		    #
		    set line [expr {$row + 1}]
		    if {[lsearch -exact [$w tag names $line.0] select] >= 0} {
			findCellTabs $win $line.0 $col tabIdx1 tabIdx2
			$w tag add $tag $tabIdx1 $tabIdx2+1c
		    }
		}

		#
		# Save the properly formatted value of val in data($tag)
		#
		set data($tag) [$w tag cget $tag -$optTail]
	    }
	}

	-text {
	    if {$data(isDisabled)} {
		return ""
	    }

	    set pixels [lindex $data(colList) [expr {2*$col}]]
	    if {$pixels != 0} {			;# convention: static width
		incr pixels $data($col-delta)
	    }
	    set alignment [lindex $data(colList) [expr {2*$col + 1}]]

	    #
	    # Adjust the cell text and the image width
	    #
	    if {[string first \t $val] >= 0 || [string first \n $val] >= 0} {
		regsub -all "\t|\n" $val " " val
	    }
	    set text $val
	    set fmtCmdFlag [lindex $data(fmtCmdFlagList) $col]
	    if {$fmtCmdFlag} {
		set text [doFormat $data($col-formatcommand) $text]
	    }
	    set oldItem [lindex $data(itemList) $row]
	    set key [lindex $oldItem end]
	    if {[info exists data($key-$col-image)]} {
		set image $data($key-$col-image)
		set imageWidth [image width $image]
	    } else {
		set image ""
		set imageWidth 0
	    }
	    set cellFont [cellFont $win $key $col]
	    adjustElem $win text imageWidth $cellFont $pixels $alignment

	    if {!$data($col-hide)} {
		#
		# Delete the old cell contents between the
		# two tabs, and insert the text and the image
		#
		findCellTabs $win [expr {$row + 1}].0 $col tabIdx1 tabIdx2
		$w delete $tabIdx1+1c $tabIdx2
		insertElem $w $tabIdx1+1c $text $image $imageWidth $alignment
	    }

	    #
	    # Replace the cell contents in the internal list
	    #
	    set newItem [lreplace $oldItem $col $col $val]
	    set data(itemList) [lreplace $data(itemList) $row $row $newItem]

	    #
	    # Replace the cell contents in the list variable if present
	    #
	    if {$data(hasListVar)} {
		set traceCmd [list tablelist::traceProc $win]
		trace vdelete ::$data(-listvariable) wu $traceCmd
		upvar #0 $data(-listvariable) var
		set var [lreplace $var $row $row \
			 [lrange $newItem 0 $data(lastCol)]]
		trace variable ::$data(-listvariable) wu $traceCmd
	    }

	    #
	    # Adjust the columns if necessary
	    #
	    if {$pixels == 0} {			;# convention: dynamic width
		set textWidth [font measure $cellFont -displayof $win $text]
		set newElemWidth [expr {$imageWidth + $textWidth}]
		if {$newElemWidth > $data($col-width)} {
		    set data($col-width) $newElemWidth
		    set data($col-widestCnt) 1
		    adjustColumns $win -1 1
		} else {
		    set oldText [lindex $oldItem $col]
		    if {$fmtCmdFlag} {
			set oldText \
			    [doFormat $data($col-formatcommand) $oldText]
		    }
		    adjustElem $win oldText imageWidth \
			       $cellFont $pixels $alignment
		    set oldTextWidth \
			[font measure $cellFont -displayof $win $oldText]
		    set oldElemWidth [expr {$imageWidth + $oldTextWidth}]
		    if {$oldElemWidth == $data($col-width) &&
			$newElemWidth < $oldElemWidth &&
			[incr data($col-widestCnt) -1] == 0} {
			adjustColumns $win $col 1
		    }
		}
	    }
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::setupColumns
#
# Updates the value of the -colums configuration option for the tablelist
# widget win by using the width, title, and alignment specifications given in
# the columns argument, and creates the corresponding label (and separator)
# widgets if createLabels is true.
#------------------------------------------------------------------------------
proc tablelist::setupColumns {win columns createLabels} {
    variable configSpecs
    variable configOpts
    variable alignments
    upvar ::tablelist::ns${win}::data data

    set argCount [llength $columns]
    set colConfigVals {}

    #
    # Check the syntax of columns before performing any changes
    #
    if {$argCount == 0} {
	return -code error "expected at least one column specification"
    }
    for {set n 0} {$n < $argCount} {incr n} {
	#
	# Get the column width
	#
	set width [lindex $columns $n]
	set width [format %d $width]	;# integer check with error message

	#
	# Get the column title
	#
	if {[incr n] == $argCount} {
	    return -code error "column title missing"
	}
	set title [lindex $columns $n]

	#
	# Get the column alignment
	#
	set alignment left
	if {[incr n] < $argCount} {
	    set next [lindex $columns $n]
	    if {[catch {format %d $next}] == 0} {	;# integer check
		incr n -1
	    } else {
		set alignment [mwutil::fullOpt "alignment" $next $alignments]
	    }
	}

	#
	# Append the properly formatted values of width,
	# title, and alignment to the list colConfigVals
	#
	lappend colConfigVals $width $title $alignment
    }

    #
    # Save the value of colConfigVals in data(-columns)
    #
    set data(-columns) $colConfigVals

    #
    # Delete the labels and separators if requested
    #
    if {$createLabels} {
	set children [winfo children $data(hdrTxtFr)]
	foreach w [lrange [lsort $children] 1 end] {
	    destroy $w
	}
	foreach w [winfo children $win] {
	    if {[regexp {^sep[0-9]+$} [winfo name $w]]} {
		destroy $w
	    }
	}
	set data(fmtCmdFlagList) {}
    }

    #
    # Build the list data(colList), and create the labels if requested
    #
    set widgetFont $data(-font)
    set data(colList) {}
    set col 0
    foreach {width title alignment} $data(-columns) {
	#
	# Append the width in pixels and the
	# alignment to the list data(colList)
	#
	if {$width > 0} {		;# convention: width in characters
	    ### set str [string repeat 0 $count]
	    set str ""
	    for {set n 0} {$n < $width} {incr n} {
		append str 0
	    }
	    set pixels [font measure $widgetFont -displayof $win $str]
	} elseif {$width < 0} {		;# convention: width in pixels
	    set pixels [expr {(-1)*$width}]
	} else {			;# convention: dynamic width
	    set pixels 0
	}
	lappend data(colList) $pixels $alignment

	if {$createLabels} {
	    if {![info exists data($col-delta)]} {
		set data($col-delta) 0
	    }
	    if {![info exists data($col-hide)]} {
		set data($col-hide) 0
	    }
	    if {![info exists data($col-resizable)]} {
		set data($col-resizable) 1
	    }
	    if {![info exists data($col-showarrow)]} {
		set data($col-showarrow) 1
	    }
	    if {![info exists data($col-sortmode)]} {
		set data($col-sortmode) ascii
	    }
	    lappend data(fmtCmdFlagList) [info exists data($col-formatcommand)]

	    #
	    # Create the label
	    #
	    set w $data(hdrTxtFrLbl)$col
	    label $w -bitmap "" -highlightthickness 0 -image "" -takefocus 0 \
		     -text "" -textvariable "" -underline -1 -wraplength 0

	    #
	    # Apply to it the current configuration options
	    #
	    foreach opt $configOpts {
		if {[llength $configSpecs($opt)] != 1 &&
		    [regexp {[lc]} [lindex $configSpecs($opt) 2] optGrp]} {
		    if {[string compare $optGrp l] == 0} {
			set optTail [string range $opt 6 end]
			if {[info exists data($col$opt)]} {
			    $w configure -$optTail $data($col$opt)
			} else {
			    $w configure -$optTail $data($opt)
			}
		    } else {
			$w configure $opt $data($opt)
		    }
		}
	    }
	    catch {$w configure -state $data(-state)}

	    #
	    # Define some mouse bindings for the label, needed for resizing
	    #
	    bind $w <Enter>	[list tablelist::labelEnter	 $win $col %x]
	    bind $w <Motion>	[list tablelist::labelEnter	 $win $col %x]
	    bind $w <Button-1>	[list tablelist::labelB1Down	 $win $col %x]
	    bind $w <B1-Motion>	[list tablelist::labelB1Motion	 $win %x %y]
	    bind $w <B1-Enter>	[list tablelist::labelB1Enter	 $win]
	    bind $w <B1-Leave>	[list tablelist::labelB1Leave	 $win %x %y]
	    bind $w <ButtonRelease-1> [list tablelist::labelB1Up $win]
	    bind $w <<Button3>>	[list tablelist::labelB3Down	 $win $col]

	    if {[info exists data($col-labelimage)]} {
		doColConfig $col $win -labelimage $data($col-labelimage)
	    }
	}

	incr col
    }

    #
    # Save the number of columns in data(colCount)
    #
    set oldColCount $data(colCount)
    set data(colCount) $col
    set data(lastCol) [expr {$col - 1}]

    if {$data(colCount) < $oldColCount} {
	#
	# Clean up the data associated with the deleted columns
	#
	for {set col $data(colCount)} {$col < $oldColCount} {incr col} {
	    if {[info exists data($col-redispId)]} {
		after cancel $data($col-redispId)
	    }
	    foreach name [array names data $col-*] {
		unset data($name)
	    }
	}
    }

    #
    # Create the separators if needed
    #
    if {$createLabels && $data(-showseparators)} {
	createSeps $win
    }
}

#------------------------------------------------------------------------------
# tablelist::createSeps
#
# Creates and manages the separator frames in the tablelist widget win.
#------------------------------------------------------------------------------
proc tablelist::createSeps win {
    upvar ::tablelist::ns${win}::data data

    for {set col 0} {$col < $data(colCount)} {incr col} {
	#
	# Create the col'th separator frame and attach it
	# to the right edge of the col'th header label
	#
	set w $data(sep)$col
	frame $w -background $data(-background) -borderwidth 1 \
		 -colormap . -container 0 -highlightthickness 0 \
		 -relief raised -takefocus 0 -width 2
	place $w -in $data(hdrTxtFrLbl)$col -anchor ne -bordermode outside \
		 -relx 1.0

	#
	# Replace the binding tag Frame with TablelistBody
	# in the list of binding tags of the separator frame
	#
	bindtags $w [list $w TablelistBody [winfo toplevel $w] all]
    }
    
    adjustSepsWhenIdle $win
}

#------------------------------------------------------------------------------
# tablelist::adjustSepsWhenIdle
#
# Arranges for the height and vertical position of each separator frame in the
# tablelist widget win to be adjusted at idle time.
#------------------------------------------------------------------------------
proc tablelist::adjustSepsWhenIdle win {
    upvar ::tablelist::ns${win}::data data

    if {[info exists data(sepsId)]} {
	return ""
    }

    set data(sepsId) [after idle [list tablelist::adjustSeps $win]]
}

#------------------------------------------------------------------------------
# tablelist::adjustSeps
#
# Adjusts the height and vertical position of each separator frame in the
# tablelist widget win.
#------------------------------------------------------------------------------
proc tablelist::adjustSeps win {
    upvar ::tablelist::ns${win}::data data

    if {[info exists data(sepsId)]} {
	unset data(sepsId)
    }

    #
    # Get the height to be applied to the separator frames
    #
    set w $data(body)
    set textIdx [$w index @0,[winfo height $w]]
    set dlineinfo [$w dlineinfo $textIdx]
    if {$data(itemCount) == 0 || [string compare $dlineinfo ""] == 0} {
	set sepHeight 1
    } else {
	foreach {x y width height baselinePos} $dlineinfo {
	    set sepHeight [expr {$y + $height}]
	}
    }

    #
    # Set the height and vertical position of each separator frame
    #
    foreach w [winfo children $win] {
	if {[regexp {^sep[0-9]+$} [winfo name $w]]} {
	    $w configure -height $sepHeight
	    if {$data(-showlabels)} {
		place configure $w -rely 1.0 -y 0
	    } else {
		place configure $w -rely 0.0 -y 1
	    }
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::adjustColumns
#
# Applies some configuration options to the labels of the tablelist widget win,
# places them in the header frame, computes and sets the tab stops for the body
# text widget, and adjusts the width and height of the header frame.  The
# colOfUnknownWidth argument specifies the dynamic-width column(s) whose width
# is to be computed when performing these operations (a number or "all").  The
# stretchCols argument specifies whether to stretch the stretchable columns.
#------------------------------------------------------------------------------
proc tablelist::adjustColumns {win colOfUnknownWidth stretchCols} {
    upvar ::tablelist::ns${win}::data data

    #
    # Configure the labels, place them in the header frame, and compute
    # the positions of the tab stops to be set in the body text widget
    #
    set data(hdrPixels) 0
    set tabs {}
    set col 0
    set x 0
    foreach {pixels alignment} $data(colList) {
	set w $data(hdrTxtFrLbl)$col
	if {$data($col-hide)} {
	    place forget $w
	    incr col
	    continue
	}

	#
	# Adjust the col`th label
	#
	if {$pixels != 0} {
	    incr pixels $data($col-delta)
	}
	if {[info exists data($col-labelalign)]} {
	    set labelAlignment $data($col-labelalign)
	} else {
	    set labelAlignment $alignment
	}
	adjustLabel $win $col $pixels $labelAlignment

	if {$pixels == 0} {			;# convention: dynamic width
	    #
	    # Compute the column width if requested
	    #
	    if {[string compare $colOfUnknownWidth all] == 0 ||
		$colOfUnknownWidth == $col} {
		computeColWidth $win $col
	    }

	    set pixels $data($col-width)
	    incr pixels $data($col-delta)
	}

	if {$col == $data(arrowCol)} {
	    #
	    # Place the canvas to the left side of the label if the
	    # latter is right-justified and to its right side otherwise
	    #
	    set canvas $data(hdrTxtFrCanv)
	    if {[string compare $labelAlignment right] == 0} {
		place $canvas -in $w -anchor w -bordermode outside \
			      -relx 0.0 -x $data(charWidth) -rely 0.5
	    } else {
		place $canvas -in $w -anchor e -bordermode outside \
			      -relx 1.0 -x -$data(charWidth) -rely 0.5
	    }
	    raise $canvas
	}

	#
	# Place the label in the header frame
	#
	set labelPixels [expr {$pixels + 2*$data(charWidth)}]
	place $w -x $x -relheight 1.0 -width $labelPixels
	incr x $labelPixels

	#
	# Append a tab stop and the alignment to the tabs list
	#
	incr data(hdrPixels) $data(charWidth)
	switch $alignment {
	    left {
		lappend tabs $data(hdrPixels) left
		incr data(hdrPixels) $pixels
	    }
	    right {
		incr data(hdrPixels) $pixels
		lappend tabs $data(hdrPixels) right
	    }
	    center {
		lappend tabs [expr {$data(hdrPixels) + $pixels/2}] center
		incr data(hdrPixels) $pixels
	    }
	}
	incr data(hdrPixels) $data(charWidth)
	lappend tabs $data(hdrPixels) left

	incr col
    }
    place $data(hdrLbl) -x $data(hdrPixels)

    #
    # Apply the value of tabs to the body text widget
    #
    $data(body) configure -tabs $tabs

    #
    # Adjust the width and height of the frames data(hdrTxtFr) and data(hdr)
    #
    $data(hdrTxtFr) configure -width $data(hdrPixels)
    if {$data(-width) <= 0} {
	if {$stretchCols} {
	    $data(hdr) configure -width $data(hdrPixels)
	}
    } else {
	$data(hdr) configure -width 0
    }
    adjustHeaderHeight $win

    #
    # Stretch the stretchable columns if requested
    #
    if {$stretchCols} {
	stretchColumnsWhenIdle $win
    }
}

#------------------------------------------------------------------------------
# tablelist::adjustLabel
#
# Applies some configuration options to the col'th label of the tablelist
# widget win as well as to the label's children (if any), and places the
# children.
#------------------------------------------------------------------------------
proc tablelist::adjustLabel {win col pixels alignment} {
    upvar ::tablelist::ns${win}::data data

    #
    # Apply some configuration options to the label and its children (if any)
    #
    set w $data(hdrTxtFrLbl)$col
    switch $alignment {
	left	{ set anchor w }
	right	{ set anchor e }
	center	{ set anchor center }
    }
    set padX [expr {$data(charWidth) - [$w cget -borderwidth]}]
    $w configure -anchor $anchor -justify $alignment -padx $padX
    if {[info exists data($col-labelimage)]} {
	set imageWidth [image width $data($col-labelimage)]
	if {[string compare $alignment right] == 0} {
	    $w.il configure -anchor e -width 0
	} else {
	    $w.il configure -anchor w -width 0
	}
	$w.tl configure -anchor $anchor -justify $alignment
    } else {
	set imageWidth 0
    }

    #
    # Make room for the canvas displaying an an up- or down-arrow if needed
    #
    set title [lindex $data(-columns) [expr {3*$col + 1}]]
    set labelFont [$w cget -font]
    if {$col == $data(arrowCol)} {
	if {[font metrics $labelFont -displayof $w -fixed]} {
	    set spaces "   "				;# 3 spaces
	} else {
	    set spaces "     "				;# 5 spaces
	}
    } else {
	set spaces ""
    }
    set spacePixels [font measure $labelFont -displayof $w $spaces]

    if {$pixels == 0} {				;# convention: dynamic width
	#
	# Set the label text
	#
	if {$imageWidth == 0} {				;# no image
	    if {[string compare $title ""] == 0} {
		set text $spaces
	    } else {
		set lines {}
		foreach line [split $title \n] {
		    if {[string compare $alignment right] == 0} {
			lappend lines $spaces$line
		    } else {
			lappend lines $line$spaces
		    }
		}
		set text [join $lines \n]
	    }
	    $w configure -text $text
	} elseif {[string compare $title ""] == 0} {	;# image w/o text
	    $w configure -text ""
	    set text ""
	    $w.il configure -width [expr {$imageWidth + $spacePixels}]
	} else {					;# both image and text
	    $w configure -text ""
	    set lines {}
	    foreach line [split $title \n] {
		if {[string compare $alignment right] == 0} {
		    lappend lines $spaces$line
		} else {
		    lappend lines $line$spaces
		}
	    }
	    set text [join $lines \n]
	    $w.tl configure -text $text
	    set colFont [lindex $data(colFontList) $col]
	    set gap [font measure $colFont -displayof $win " "]
	    $w.il configure -width [expr {$imageWidth + $gap}]
	}
    } else {
	#
	# Clip each line of title according to pixels and alignment
	#
	set lessPixels [expr {$pixels - $spacePixels}]
	if {$imageWidth == 0} {				;# no image
	    if {[string compare $title ""] == 0} {
		set text $spaces
	    } else {
		set lines {}
		foreach line [split $title \n] {
		    set line [strRangeExt $win $line \
			      $labelFont $lessPixels $alignment]
		    if {[string compare $alignment right] == 0} {
			lappend lines $spaces$line
		    } else {
			lappend lines $line$spaces
		    }
		}
		set text [join $lines \n]
	    }
	    $w configure -text $text
	} elseif {[string compare $title ""] == 0} {	;# image w/o text
	    $w configure -text ""
	    set text ""
	    if {$imageWidth <= $lessPixels} {
		$w.il configure -width [expr {$imageWidth + $spacePixels}]
	    } else {
		set imageWidth 0		;# can't display the image
	    }
	} else {					;# both image and text
	    $w configure -text ""
	    set colFont [lindex $data(colFontList) $col]
	    set gap [font measure $colFont -displayof $win " "]
	    if {$imageWidth + $gap <= $lessPixels} {
		incr lessPixels -[expr {$imageWidth + $gap}]
		set lines {}
		foreach line [split $title \n] {
		    set line [strRangeExt $win $line \
			      $labelFont $lessPixels $alignment]
		    if {[string compare $alignment right] == 0} {
			lappend lines $spaces$line
		    } else {
			lappend lines $line$spaces
		    }
		}
		set text [join $lines \n]
		$w.tl configure -text $text
		$w.il configure -width [expr {$imageWidth + $gap}]
	    } elseif {$imageWidth <= $lessPixels} {	
		set text ""			;# can't display the text
		$w.il configure -width [expr {$imageWidth + $spacePixels}]
	    } else {
		set imageWidth 0		;# can't display the image
		set text ""			;# can't display the text
	    }
	}
    }

    #
    # Place the label's children (if any)
    #
    if {$imageWidth == 0} {
	if {[info exists data($col-labelimage)]} {
	    place forget $w.il
	    place forget $w.tl
	}
    } else {
	if {[string compare $text ""] == 0} {
	    place forget $w.tl
	}

	switch $alignment {
	    left {
		place $w.il -anchor nw -relx 0.0 -x $padX -relheight 1.0
		if {[string compare $text ""] != 0} {
		    set textX [expr {$padX + [winfo reqwidth $w.il]}]
		    place $w.tl -anchor nw -relx 0.0 -x $textX -relheight 1.0
		}
	    }

	    right {
		place $w.il -anchor ne -relx 1.0 -x -$padX -relheight 1.0
		if {[string compare $text ""] != 0} {
		    set textX [expr {-$padX - [winfo reqwidth $w.il]}]
		    place $w.tl -anchor ne -relx 1.0 -x $textX -relheight 1.0
		}
	    }

	    center {
		if {[string compare $text ""] == 0} {
		    place $w.il -anchor n -relx 0.5 -x 0 -relheight 1.0
		} else {
		    set halfWidth [expr {([winfo reqwidth $w.il] + \
					  [winfo reqwidth $w.tl]) / 2}]
		    place $w.il -anchor nw -relx 0.5 -x -$halfWidth \
				-relheight 1.0
		    place $w.tl -anchor ne -relx 0.5 -x $halfWidth \
				-relheight 1.0
		}
	    }
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::computeColWidth
#
# Computes the width of the col'th column of the tablelist widget win to be just
# large enough to hold all the elements of the column (including its label).
#------------------------------------------------------------------------------
proc tablelist::computeColWidth {win col} {
    upvar ::tablelist::ns${win}::data data

    set fmtCmdFlag [lindex $data(fmtCmdFlagList) $col]
    set colFont [lindex $data(colFontList) $col]

    set data($col-width) 0
    set data($col-widestCnt) 0

    #
    # Column elements
    #
    foreach item $data(itemList) {
	if {$col >= [llength $item] - 1} {
	    continue
	}

	set text [lindex $item $col]
	if {$fmtCmdFlag} {
	    set text [doFormat $data($col-formatcommand) $text]
	}
	set key [lindex $item end]
	if {[info exists data($key-$col-image)]} {
	    set imageWidth [image width $data($key-$col-image)]
	} else {
	    set imageWidth 0
	}
	if {[info exists data($key-$col-font)]} {
	    set cellFont $data($key-$col-font)
	} elseif {[info exists data($key-font)]} {
	    set cellFont $data($key-font)
	} else {
	    set cellFont $colFont
	}
	adjustElem $win text imageWidth $cellFont 0 left
	set textWidth [font measure $cellFont -displayof $win $text]
	set elemWidth [expr {$imageWidth + $textWidth}]
	if {$elemWidth > $data($col-width)} {
	    set data($col-width) $elemWidth
	    set data($col-widestCnt) 1
	} elseif {$elemWidth == $data($col-width)} {
	    incr data($col-widestCnt)
	}
    }

    #
    # Column label
    #
    set w $data(hdrTxtFrLbl)$col
    if {[info exists data($col-labelimage)]} {
	set title [lindex $data(-columns) [expr {3*$col + 1}]]
	if {[string compare $title ""] == 0} {		;# image w/o text
	    set netLabelWidth [winfo reqwidth $w.il]
	} else {					;# both image and text
	    set netLabelWidth [expr {[winfo reqwidth $w.il] +
				     [winfo reqwidth $w.tl]}]
	}
    } else {						;# no image
	set netLabelWidth [expr {[winfo reqwidth $w] - 2*$data(charWidth)}]
    }
    if {$netLabelWidth > $data($col-width)} {
	set data($col-width) $netLabelWidth
	set data($col-widestCnt) 1
    } elseif {$netLabelWidth == $data($col-width)} {
	incr data($col-widestCnt)
    }
}

#------------------------------------------------------------------------------
# tablelist::adjustHeaderHeight
#
# Sets the height of the header frame of the tablelist widget win to the max.
# height of its children and adjusts the y-coordinate of the canvas containing
# an up- or down-arrow.
#------------------------------------------------------------------------------
proc tablelist::adjustHeaderHeight win {
    upvar ::tablelist::ns${win}::data data

    #
    # Compute the max. label height
    #
    set maxLabelHeight 0
    set children [winfo children $data(hdrTxtFr)]
    foreach w [lrange [lsort $children] 1 end] {
	if {[string compare [winfo manager $w] ""] == 0} {
	    continue
	}

	set reqHeight [winfo reqheight $w]
	if {$reqHeight > $maxLabelHeight} {
	    set maxLabelHeight $reqHeight
	}

	foreach c [winfo children $w] {
	    if {[string compare [winfo manager $c] ""] == 0} {
		continue
	    }

	    set reqHeight \
		[expr {[winfo reqheight $c] + 2*[$w cget -borderwidth]}]
	    if {$reqHeight > $maxLabelHeight} {
		set maxLabelHeight $reqHeight
	    }
	}
    }

    #
    # Set the height of the header frame and adjust the separators
    #
    $data(hdrTxtFr) configure -height $maxLabelHeight
    if {$data(-showlabels)} {
	$data(hdr) configure -height $maxLabelHeight
    } else {
	$data(hdr) configure -height 1
    }
    adjustSepsWhenIdle $win
}

#------------------------------------------------------------------------------
# tablelist::stretchColumnsWhenIdle
#
# Arranges for the stretchable columns of the tablelist widget win to be
# stretched at idle time.
#------------------------------------------------------------------------------
proc tablelist::stretchColumnsWhenIdle win {
    upvar ::tablelist::ns${win}::data data

    if {[info exists data(stretchId)]} {
	return ""
    }

    set data(stretchId) [after idle [list tablelist::stretchColumns $win]]
}

#------------------------------------------------------------------------------
# tablelist::stretchColumns
#
# Stretches the stretchable columns to fill the tablelist window win
# horizontally.
#------------------------------------------------------------------------------
proc tablelist::stretchColumns win {
    upvar ::tablelist::ns${win}::data data

    if {[info exists data(stretchId)]} {
	unset data(stretchId)
    }

    set forceAdjust $data(forceAdjust)
    set data(forceAdjust) 0

    if {$data(hdrPixels) == 0 || $data(-width) <= 0} {
	return ""
    }

    #
    # Compute the total number delta of pixels
    # by which the columns are to be stretched
    #
    set delta [winfo width $data(hdr)]
    set col 0
    foreach {pixels alignment} $data(colList) {
	if {$data($col-hide)} {
	    incr col
	    continue
	}
	if {$pixels == 0} {			;# convention: dynamic width
	    set pixels $data($col-width)
	}
	incr delta -[expr {$pixels + 2*$data(charWidth)}]

	incr col
    }
    if {$delta < 0} {
	set delta 0
    }

    #
    # Get the list of the numerical indices of the stretchable columns
    #
    set stretchableCols {}
    if {[string compare $data(-stretch) all] == 0} {
	for {set col 0} {$col < $data(colCount)} {incr col} {
	    lappend stretchableCols $col
	}
    } else {
	foreach col $data(-stretch) {
	    lappend stretchableCols [colIndex $win $col 0]
	}
    }

    #
    # Get the number of columns to be stretched and the mean value of delta
    #
    set count 0
    set lastColToStretch -1
    for {set col 0} {$col < $data(colCount)} {incr col} {
	if {[lsearch -exact $stretchableCols $col] >= 0 && !$data($col-hide)} {
	    incr count
	    set lastColToStretch $col
	}
    }
    if {$count == 0} {
	set meanVal 0
	set rest 0
    } else {
	set meanVal [expr {$delta / $count}]
	set rest [expr {$delta % $count}]
    }
    if {$count == 0 && !$forceAdjust} {
	return ""
    }

    #
    # Add the mean value of delta to the widths of the stretchable
    # columns and the rest to the width of the last such column
    #
    set col 0
    foreach {pixels alignment} $data(colList) {
	if {[lsearch -exact $stretchableCols $col] >= 0 && !$data($col-hide)} {
	    set oldDelta $data($col-delta)
	    set data($col-delta) $meanVal
	    if {$col == $lastColToStretch} {
		incr data($col-delta) $rest
	    }
	    if {$pixels != 0 && $data($col-delta) != $oldDelta} {
		redisplayColWhenIdle $win $col
	    }
	} else {
	    set data($col-delta) 0
	}

	incr col
    }

    #
    # Adjust the columns
    #
    adjustColumns $win -1 0
    update idletasks
}

#------------------------------------------------------------------------------
# tablelist::redisplayWhenIdle
#
# Arranges for the items of the tablelist widget win to be redisplayed at idle
# time.
#------------------------------------------------------------------------------
proc tablelist::redisplayWhenIdle win {
    upvar ::tablelist::ns${win}::data data

    if {[info exists data(redispId)] || $data(itemCount) == 0} {
	return ""
    }

    set data(redispId) [after idle [list tablelist::redisplay $win 0 end]]

    #
    # Cancel the execution of all delayed redisplayCol commands
    #
    foreach name [array names data *-redispId] {
	after cancel $data($name)
	unset data($name)
    }
}

#------------------------------------------------------------------------------
# tablelist::redisplay
#
# Redisplays the items of the tablelist widget win, in the range specified by
# first and last.
#------------------------------------------------------------------------------
proc tablelist::redisplay {win first last} {
    upvar ::tablelist::ns${win}::data data

    if {$first == 0 && [string first $last end] == 0 &&
	[info exists data(redispId)]} {
	unset data(redispId)
    }

    if {$first < 0} {
	return ""
    }
    if {[string first $last end] == 0} {
	set last $data(lastRow)
    }

    set w $data(body)
    for {set idx $first; set line [expr {$first + 1}]} {$idx <= $last} \
	{incr idx; incr line} {
	#
	# Check whether the line is selected
	#
	set tagNames [$w tag names $line.0]
	if {[lsearch -exact $tagNames select] >= 0} {
	    set selected 1
	} else {
	    set selected 0
	}

	#
	# Empty the line, clip the elements if necessary,
	# and insert them with the corresponding tags
	#
	$w delete $line.0 $line.end
	set item [lindex $data(itemList) $idx]
	set keyIdx [expr {[llength $item] - 1}]
	set key [lindex $item end]
	set rowTags {}
	foreach opt {-background -font -foreground} {
	    if {[info exists data($key$opt)]} {
		lappend rowTags $key$opt
	    }
	}
	set newItem {}
	set col 0
	foreach colFont $data(colFontList) \
		colTags $data(colTagsList) \
		fmtCmdFlag $data(fmtCmdFlagList) \
		{pixels alignment} $data(colList) {
	    if {$col < $keyIdx} {
		set text [lindex $item $col]
	    } else {
		set text ""
	    }
	    lappend newItem $text

	    if {$data($col-hide)} {
		incr col
		continue
	    }

	    #
	    # Adjust the cell text and the image width
	    #
	    if {$fmtCmdFlag} {
		set text [doFormat $data($col-formatcommand) $text]
	    }
	    if {[info exists data($key-$col-image)]} {
		set image $data($key-$col-image)
		set imageWidth [image width $image]
	    } else {
		set image ""
		set imageWidth 0
	    }
	    if {$pixels != 0} {			;# convention: static width
		incr pixels $data($col-delta)
	    }
	    if {[info exists data($key-$col-font)]} {
		set cellFont $data($key-$col-font)
	    } elseif {[info exists data($key-font)]} {
		set cellFont $data($key-font)
	    } else {
		set cellFont $colFont
	    }
	    adjustElem $win text imageWidth $cellFont $pixels $alignment

	    #
	    # Insert the text and the image
	    #
	    set tagNames {}
	    eval lappend tagNames $colTags $rowTags
	    foreach opt {-background -font -foreground} {
		if {[info exists data($key-$col$opt)]} {
		    lappend tagNames $key-$col$opt
		}
	    }
	    if {$imageWidth == 0} {
		$w insert $line.end \t$text\t $tagNames
	    } else {
		$w insert $line.end \t\t $tagNames
		insertElem $w $line.end-1c $text $image $imageWidth $alignment
	    }

	    incr col
	}
	lappend newItem $key
	set data(itemList) [lreplace $data(itemList) $idx $idx $newItem]

	#
	# Select the item if it was selected before
	#
	if {$selected} {
	    selectionSubCmd $win set $idx $idx 2
	}
    }

    #
    # Restore the stripes in the body text widget
    #
    makeStripesWhenIdle $win
}

#------------------------------------------------------------------------------
# tablelist::redisplayColWhenIdle
#
# Arranges for the elements of the col'th column of the tablelist widget win to
# be redisplayed at idle time.
#------------------------------------------------------------------------------
proc tablelist::redisplayColWhenIdle {win col} {
    upvar ::tablelist::ns${win}::data data

    if {[info exists data($col-redispId)] || [info exists data(redispId)] ||
	$data(itemCount) == 0} {
	return ""
    }

    set data($col-redispId) \
	[after idle [list tablelist::redisplayCol $win $col 0 end]]
}

#------------------------------------------------------------------------------
# tablelist::redisplayCol
#
# Redisplays the elements of the col'th column of the tablelist widget win, in
# the range specified by first and last.
#------------------------------------------------------------------------------
proc tablelist::redisplayCol {win col first last} {
    upvar ::tablelist::ns${win}::data data

    if {$first == 0 && [string first $last end] == 0 &&
	[info exists data($col-redispId)]} {
	unset data($col-redispId)
    }

    if {$data($col-hide) || $first < 0} {
	return ""
    }
    if {[string first $last end] == 0} {
	set last $data(lastRow)
    }

    set fmtCmdFlag [lindex $data(fmtCmdFlagList) $col]
    set colFont [lindex $data(colFontList) $col]

    set w $data(body)
    set pixels [lindex $data(colList) [expr {2*$col}]]
    if {$pixels != 0} {				;# convention: static width
	incr pixels $data($col-delta)
    }
    set alignment [lindex $data(colList) [expr {2*$col + 1}]]

    for {set idx $first; set line [expr {$first + 1}]} {$idx <= $last} \
	{incr idx; incr line} {
	#
	# Adjust the cell text and the image width
	#
	set item [lindex $data(itemList) $idx]
	set text [lindex $item $col]
	if {$fmtCmdFlag} {
	    set text [doFormat $data($col-formatcommand) $text]
	}
	set key [lindex $item end]
	if {[info exists data($key-$col-image)]} {
	    set image $data($key-$col-image)
	    set imageWidth [image width $image]
	} else {
	    set image ""
	    set imageWidth 0
	}
	if {[info exists data($key-$col-font)]} {
	    set cellFont $data($key-$col-font)
	} elseif {[info exists data($key-font)]} {
	    set cellFont $data($key-font)
	} else {
	    set cellFont $colFont
	}
	adjustElem $win text imageWidth $cellFont $pixels $alignment

	#
	# Delete the old cell contents between the
	# two tabs, and insert the text and the image
	#
	findCellTabs $win $line.0 $col tabIdx1 tabIdx2
	$w delete $tabIdx1+1c $tabIdx2
	insertElem $w $tabIdx1+1c $text $image $imageWidth $alignment
    }
}

#------------------------------------------------------------------------------
# tablelist::makeStripesWhenIdle
#
# Arranges for the stripes in the body of the tablelist widget win to be
# redrawn at idle time.
#------------------------------------------------------------------------------
proc tablelist::makeStripesWhenIdle win {
    upvar ::tablelist::ns${win}::data data

    if {[info exists data(stripesId)] || $data(itemCount) == 0} {
	return ""
    }

    set data(stripesId) [after idle [list tablelist::makeStripes $win]]
}

#------------------------------------------------------------------------------
# tablelist::makeStripes
#
# Redraws the stripes in the body of the tablelist widget win.
#------------------------------------------------------------------------------
proc tablelist::makeStripes win {
    upvar ::tablelist::ns${win}::data data

    if {[info exists data(stripesId)]} {
	unset data(stripesId)
    }

    set w $data(body)
    $w tag remove stripe 1.0 end
    if {[string compare $data(-stripebackground) ""] == 0 &&
	[string compare $data(-stripeforeground) ""] == 0} {
	return ""
    }

    set step [expr {2*$data(-stripeheight)}]
    for {set n [expr {$data(-stripeheight) + 1}]} {$n <= $step} {incr n} {
	for {set line $n} {$line <= $data(itemCount)} {incr line $step} {
	    $w tag add stripe $line.0 $line.end
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::makeListVar
#
# Arranges for the global variable specified by varName to become the list
# variable associated with the tablelist widget win.
#------------------------------------------------------------------------------
proc tablelist::makeListVar {win varName} {
    upvar ::tablelist::ns${win}::data data

    if {[string compare $varName ""] == 0} {
	#
	# If there is an old list variable associated with the
	# widget then remove the trace set on this variable
	#
	if {$data(hasListVar)} {
	    synchronize $win
	    trace vdelete ::$data(-listvariable) wu \
		    [list tablelist::traceProc $win]
	}
	return ""
    }

    #
    # The list variable may be an array element but must not be an array
    #
    if {![regexp {^(.*)\((.*)\)$} $varName dummy name1 name2]} {
	if {[array exists ::$varName]} {
	    return -code error "variable \"$varName\" is array"
	}
	set name1 $varName
	set name2 ""
    }

    if {[info exists ::$varName]} {
	#
	# Invoke the trace procedure associated with the new list variable
	#
	traceProc $win $name1 $name2 w
    } else {
	#
	# Set ::$varName according to the value of data(itemList)
	#
	set ::$varName {}
	foreach item $data(itemList) {
	    lappend ::$varName [lrange $item 0 $data(lastCol)]
	}
    }

    #
    # If there is an old list variable associated with the
    # widget then remove the trace set on this variable,
    # and in any case set a trace on the new list variable
    #
    set traceCmd [list tablelist::traceProc $win]
    if {$data(hasListVar)} {
	trace vdelete ::$data(-listvariable) wu $traceCmd
    }
    trace variable ::$varName wu $traceCmd
}

#------------------------------------------------------------------------------
# tablelist::traceProc
#
# This procedure is executed whenever the global variable specified by varName
# is written or unset.  It makes sure that the contents of the widget will be
# synchronized with the value of the variable at idle time, and that the
# variable is recreated if it was unset.
#------------------------------------------------------------------------------
proc tablelist::traceProc {win varName index op} {
    upvar ::tablelist::ns${win}::data data

    if {[string compare $index ""] != 0} {
	set varName ${varName}($index)
    }

    switch $op {
	w {
	    if {![info exists data(syncId)]} {
		#
		# Arrange for the contents of the widget to be synchronized
		# with the value of the variable ::$varName at idle time.
		#
		set data(syncId) [after idle [list tablelist::synchronize $win]]

		#
		# Cancel the execution of all delayed redisplay and
		# redisplayCol commands, to make sure that the synchronize
		# command will be invoked first; the latter will then
		# schedule a redisplay command for execution at idle time
		#
		foreach name [array names data *redispId] {
		    after cancel $data($name)
		    unset data($name)
		}
	    }
	}

	u {
	    #
	    # Recreate the variable ::$varName by setting it according to
	    # the value of data(itemList), and set the trace on it again
	    #
	    set ::$varName {}
	    foreach item $data(itemList) {
		lappend ::$varName [lrange $item 0 $data(lastCol)]
	    }
	    trace variable ::$varName wu [list tablelist::traceProc $win]
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::synchronize
#
# This procedure is invoked either as an idle callback after the list variable
# associated with the tablelist widget win was written, or directly, upon
# execution of some widget commands.  It makes sure that the contents of the
# widget is synchronized with the value of the list variable.
#------------------------------------------------------------------------------
proc tablelist::synchronize win {
    upvar ::tablelist::ns${win}::data data

    #
    # Nothing to do if the list variable was not written
    #
    if {![info exists data(syncId)]} {
	return ""
    }

    #
    # Here we are in the case that the procedure was scheduled for
    # execution at idle time.  However, it might have been invoked
    # directly, before the idle time occured; in this case we should
    # cancel the execution of the previously scheduled idle callback.
    #
    after cancel $data(syncId)	;# no harm if data(syncId) is no longer valid
    unset data(syncId)

    upvar #0 $data(-listvariable) var
    set newCount [llength $var]
    if {$newCount < $data(itemCount)} {
	#
	# Delete the items with indices >= newCount from the widget
	#
	set updateCount $newCount
	deleteSubCmd $win $newCount $data(lastRow) 0
    } elseif {$newCount > $data(itemCount)} {
	#
	# Insert the items of var with indices
	# >= data(itemCount) into the widget
	#
	set updateCount $data(itemCount)
	insertSubCmd $win $data(itemCount) \
		     [lrange $var $data(itemCount) end] 0
    } else {
	set updateCount $newCount
    }

    #
    # Update the first updateCount items of the internal list
    #
    set itemsChanged 0
    for {set idx 0} {$idx < $updateCount} {incr idx} {
	set oldItem [lindex $data(itemList) $idx]
	set newItem [adjustItem [lindex $var $idx] $data(colCount)]
	lappend newItem [lindex $oldItem end]

	if {[string compare $oldItem $newItem] != 0} {
	    set data(itemList) [lreplace $data(itemList) $idx $idx $newItem]
	    set itemsChanged 1
	}
    }

    #
    # If necessary, adjust the columns and make sure
    # that the items will be redisplayed at idle time
    #
    if {$itemsChanged} {
	adjustColumns $win all 1
	redisplayWhenIdle $win
    }
}

#
# Private procedures implementing the tablelist widget command
# ============================================================
#

#------------------------------------------------------------------------------
# tablelist::tablelistWidgetCmd
#
# This procedure is invoked to process the Tcl command corresponding to a
# tablelist widget.
#------------------------------------------------------------------------------
proc tablelist::tablelistWidgetCmd {win argList} {
    variable cmdOpts
    upvar ::tablelist::ns${win}::data data

    set argCount [llength $argList]
    if {$argCount == 0} {
	mwutil::wrongNumArgs "$win option ?arg arg ...?"
    }

    set cmd [mwutil::fullOpt "option" [lindex $argList 0] $cmdOpts]
    switch $cmd {
	activate {
	    if {$argCount != 2} {
		mwutil::wrongNumArgs "$win $cmd index"
	    }

	    synchronize $win
	    set index [rowIndex $win [lindex $argList 1] 0]
	    if {$data(isDisabled)} {
		return ""
	    } else {
		return [activateSubCmd $win $index]
	    }
	}

	attrib {
	    return [mwutil::attribSubCmd $win [lrange $argList 1 end]]
	}

	bbox {
	    if {$argCount != 2} {
		mwutil::wrongNumArgs "$win $cmd index"
	    }

	    synchronize $win
	    set index [rowIndex $win [lindex $argList 1] 0]
	    return [bboxSubCmd $win $index]
	}

	bodypath {
	    if {$argCount != 1} {
		mwutil::wrongNumArgs "$win $cmd"
	    }

	    return $data(body)
	}

	cellcget {
	    if {$argCount != 3} {
		mwutil::wrongNumArgs "$win $cmd cellIndex option"
	    }

	    #
	    # Check the cell index
	    #
	    synchronize $win
	    set cell [lindex $argList 1]
	    scan [cellIndex $win $cell] %d,%d row col
	    if {$row < 0 || $row >= $data(itemCount) ||
		$col < 0 || $col >= $data(colCount)} {
		return -code error \
		       "cell index \"$cell\" out of range"
	    }

	    return [cellcgetSubCmd $win $row $col [lindex $argList 2]]
	}

	cellconfigure {
	    if {$argCount < 2} {
		mwutil::wrongNumArgs "$win $cmd cellIndex ?option? ?value?\
				      ?option value ...?"
	    }

	    #
	    # Check the cell index
	    #
	    synchronize $win
	    set cell [lindex $argList 1]
	    scan [cellIndex $win $cell] %d,%d row col
	    if {$row < 0 || $row >= $data(itemCount) ||
		$col < 0 || $col >= $data(colCount)} {
		return -code error \
		       "cell index \"$cell\" out of range"
	    }

	    return [cellconfigureSubCmd $win $row $col [lrange $argList 2 end]]
	}

	cellindex {
	    if {$argCount != 2} {
		mwutil::wrongNumArgs "$win $cmd cellIndex"
	    }

	    synchronize $win
	    return [cellIndex $win [lindex $argList 1]]
	}

	cget {
	    if {$argCount != 2} {
		mwutil::wrongNumArgs "$win $cmd option"
	    }

	    #
	    # Return the value of the specified configuration option
	    #
	    variable configSpecs
	    set opt [mwutil::fullConfigOpt [lindex $argList 1] configSpecs]
	    return $data($opt)
	}

	columncget {
	    if {$argCount != 3} {
		mwutil::wrongNumArgs "$win $cmd columnIndex option"
	    }

	    #
	    # Check the column index
	    #
	    synchronize $win
	    set colArg [lindex $argList 1]
	    set col [colIndex $win $colArg 1]

	    return [columncgetSubCmd $win $col [lindex $argList 2]]
	}

	columnconfigure {
	    if {$argCount < 2} {
		mwutil::wrongNumArgs "$win $cmd columnIndex ?option? ?value?\
				      ?option value ...?"
	    }

	    #
	    # Check the column index
	    #
	    synchronize $win
	    set colArg [lindex $argList 1]
	    set col [colIndex $win $colArg 1]

	    return [columnconfigureSubCmd $win $col [lrange $argList 2 end]]
	}

	columncount {
	    if {$argCount != 1} {
		mwutil::wrongNumArgs "$win $cmd"
	    }

	    return $data(colCount)
	}

	columnindex {
	    if {$argCount != 2} {
		mwutil::wrongNumArgs "$win $cmd columnIndex"
	    }

	    synchronize $win
	    return [colIndex $win [lindex $argList 1] 0]
	}

	configure {
	    variable configSpecs
	    return [mwutil::configSubCmd $win configSpecs data \
		    tablelist::doConfig [lrange $argList 1 end]]
	}

	curselection {
	    if {$argCount != 1} {
		mwutil::wrongNumArgs "$win $cmd"
	    }

	    synchronize $win
	    return [curselectionSubCmd $win]
	}

	delete {
	    if {$argCount < 2 || $argCount > 3} {
		mwutil::wrongNumArgs "$win $cmd firstIndex ?lastIndex?"
	    }

	    synchronize $win
	    set first [rowIndex $win [lindex $argList 1] 0]
	    if {$argCount == 3} {
		set last [rowIndex $win [lindex $argList 2] 0]
	    } else {
		set last $first
	    }
	    if {$data(isDisabled)} {
		return ""
	    } else {
		return [deleteSubCmd $win $first $last 1]
	    }
	}

	get {
	    if {$argCount < 2 || $argCount > 3} {
		mwutil::wrongNumArgs "$win $cmd firstIndex ?lastIndex?"
	    }

	    synchronize $win
	    set first [rowIndex $win [lindex $argList 1] 0]
	    if {$argCount == 3} {
		set last [rowIndex $win [lindex $argList 2] 0]
		set idxCount 2
	    } else {
		set last $first
		set idxCount 1
	    }
	    return [getSubCmd $win $first $last $idxCount]
	}

	index {
	    if {$argCount != 2} {
		mwutil::wrongNumArgs "$win $cmd index"
	    }

	    synchronize $win
	    return [rowIndex $win [lindex $argList 1] 1]
	}

	insert {
	    if {$argCount < 2} {
		mwutil::wrongNumArgs "$win $cmd index ?item item ...?"
	    }

	    synchronize $win
	    set index [rowIndex $win [lindex $argList 1] 1]
	    if {$data(isDisabled)} {
		return ""
	    } else {
		return [insertSubCmd $win $index [lrange $argList 2 end] 1]
	    }
	}

	insertlist {
	    if {$argCount != 3} {
		mwutil::wrongNumArgs "$win $cmd index list"
	    }

	    synchronize $win
	    set index [rowIndex $win [lindex $argList 1] 1]
	    if {$data(isDisabled)} {
		return ""
	    } else {
		return [insertSubCmd $win $index [lindex $argList 2] 1]
	    }
	}

	labelpath {
	    if {$argCount != 2} {
		mwutil::wrongNumArgs "$win $cmd columnIndex"
	    }

	    #
	    # Check the column index
	    #
	    synchronize $win
	    set colArg [lindex $argList 1]
	    set col [colIndex $win $colArg 1]

	    return $data(hdrTxtFrLbl)$col
	}

	labels {
	    if {$argCount != 1} {
		mwutil::wrongNumArgs "$win $cmd"
	    }

	    set children [winfo children $data(hdrTxtFr)]
	    return [lrange [lsort $children] 1 end]
	}

	nearest {
	    if {$argCount != 2} {
		mwutil::wrongNumArgs "$win $cmd y"
	    }

	    set y [lindex $argList 1]
	    format %d $y		;# integer check with error message
	    synchronize $win
	    return [rowIndex $win @0,$y 0]
	}

	nearestcell {
	    if {$argCount != 3} {
		mwutil::wrongNumArgs "$win $cmd x y"
	    }

	    set x [lindex $argList 1]
	    format %d $x		;# integer check with error message
	    set y [lindex $argList 2]
	    format %d $y		;# integer check with error message
	    synchronize $win
	    return [cellIndex $win @$x,$y]
	}

	nearestcolumn {
	    if {$argCount != 2} {
		mwutil::wrongNumArgs "$win $cmd x"
	    }

	    set x [lindex $argList 1]
	    format %d $x		;# integer check with error message
	    synchronize $win
	    return [colIndex $win @$x,0 0]
	}

	resetsortinfo {
	    if {$argCount != 1} {
		mwutil::wrongNumArgs "$win $cmd"
	    }

	    set data(sortCol) -1
	    set data(sortOrder) {}

	    place forget $data(hdrTxtFrCanv)
	    set colOfUnknownWidth $data(arrowCol)
	    set data(arrowCol) -1
	    synchronize $win
	    adjustColumns $win $colOfUnknownWidth 1
	    return ""
	}

	rowcget {
	    if {$argCount != 3} {
		mwutil::wrongNumArgs "$win $cmd index option"
	    }

	    #
	    # Check the row index
	    #
	    synchronize $win
	    set rowArg [lindex $argList 1]
	    set row [rowIndex $win $rowArg 0]
	    if {$row < 0 || $row >= $data(itemCount)} {
		return -code error \
		       "row index \"$rowArg\" out of range"
	    }

	    return [rowcgetSubCmd $win $row [lindex $argList 2]]
	}

	rowconfigure {
	    if {$argCount < 2} {
		mwutil::wrongNumArgs "$win $cmd index ?option? ?value?\
				      ?option value ...?"
	    }

	    #
	    # Check the row index
	    #
	    synchronize $win
	    set rowArg [lindex $argList 1]
	    set row [rowIndex $win $rowArg 0]
	    if {$row < 0 || $row >= $data(itemCount)} {
		return -code error \
		       "row index \"$rowArg\" out of range"
	    }

	    return [rowconfigureSubCmd $win $row [lrange $argList 2 end]]
	}

	scan {
	    if {$argCount != 4} {
		mwutil::wrongNumArgs "$win $cmd mark|dragto x y"
	    }

	    set x [lindex $argList 2]
	    set y [lindex $argList 3]
	    format %d $x		;# integer check with error message
	    format %d $y		;# integer check with error message
	    variable scanCmdOpts
	    set opt [mwutil::fullOpt "option" [lindex $argList 1] $scanCmdOpts]
	    synchronize $win
	    return [scanSubCmd $win $opt $x $y]
	}

	see {
	    if {$argCount != 2} {
		mwutil::wrongNumArgs "$win $cmd index"
	    }

	    synchronize $win
	    set index [rowIndex $win [lindex $argList 1] 0]
	    return [seeSubCmd $win $index]
	}

	selection {
	    if {$argCount < 3 || $argCount > 4} {
		mwutil::wrongNumArgs "$win $cmd option index ?index?"
	    }

	    synchronize $win
	    set first [rowIndex $win [lindex $argList 2] 0]
	    if {$argCount == 4} {
		set last [rowIndex $win [lindex $argList 3] 0]
		set idxCount 2
	    } else {
		set last $first
		set idxCount 1
	    }
	    variable selCmdOpts
	    set opt [mwutil::fullOpt "option" [lindex $argList 1] $selCmdOpts]
	    if {$data(isDisabled)} {
		return ""
	    } else {
		return [selectionSubCmd $win $opt $first $last $idxCount]
	    }
	}

	separatorpath {
	    if {$argCount != 2} {
		mwutil::wrongNumArgs "$win $cmd columnIndex"
	    }

	    #
	    # Check the column index
	    #
	    synchronize $win
	    set colArg [lindex $argList 1]
	    set col [colIndex $win $colArg 1]

	    if {$data(-showseparators)} {
		return $data(sep)$col
	    } else {
		return ""
	    }
	}

	separators {
	    if {$argCount != 1} {
		mwutil::wrongNumArgs "$win $cmd"
	    }

	    set sepList {}
	    foreach w [winfo children $win] {
		if {[regexp {^sep[0-9]+$} [winfo name $w]]} {
		    lappend sepList $w
		}
	    }
	    return $sepList
	}

	size {
	    if {$argCount != 1} {
		mwutil::wrongNumArgs "$win $cmd"
	    }

	    synchronize $win
	    return $data(itemCount)
	}

	sort {
	    if {$argCount < 1 || $argCount > 2} {
		mwutil::wrongNumArgs "$win $cmd  ?-increasing|-decreasing?"
	    }

	    if {$argCount == 1} {
		set order -increasing
	    } else {
		variable sortOrders
		set order [mwutil::fullOpt "option" \
			   [lindex $argList 2] $sortOrders]
	    }
	    synchronize $win
	    return [sortSubCmd $win -1 $order]
	}

	sortbycolumn {
	    if {$argCount < 2 || $argCount > 3} {
		mwutil::wrongNumArgs "$win $cmd columnIndex\
				      ?-increasing|-decreasing?"
	    }

	    #
	    # Check the column index
	    #
	    synchronize $win
	    set colArg [lindex $argList 1]
	    set col [colIndex $win $colArg 1]

	    if {$argCount == 2} {
		set order -increasing
	    } else {
		variable sortOrders
		set order [mwutil::fullOpt "option" \
			   [lindex $argList 2] $sortOrders]
	    }
	    return [sortSubCmd $win $col $order]
	}

	sortcolumn {
	    if {$argCount != 1} {
		mwutil::wrongNumArgs "$win $cmd"
	    }

	    return $data(sortCol)
	}

	sortorder {
	    if {$argCount != 1} {
		mwutil::wrongNumArgs "$win $cmd"
	    }

	    return $data(sortOrder)
	}

	xview {
	    synchronize $win
	    return [xviewSubCmd $win [lrange $argList 1 end]]
	}

	yview {
	    synchronize $win
	    return [yviewSubCmd $win [lrange $argList 1 end]]
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::activateSubCmd
#
# This procedure is invoked to process the tablelist activate subcommand.
#------------------------------------------------------------------------------
proc tablelist::activateSubCmd {win index} {
    upvar ::tablelist::ns${win}::data data

    #
    # Adjust the index to fit within the existing items
    #
    if {$index >= $data(itemCount)} {
	set index $data(lastRow)
    }
    if {$index < 0} {
	set index 0
    }

    #
    # If the focus is currently on the body text widget
    # then move the "active" tag to the corresponding line
    #
    set w $data(body)
    if {[string compare [focus -displayof $win] $w] == 0} {
	set line [expr {$data(activeIdx) + 1}]
	$w tag remove active $line.0 $line.end

	set line [expr {$index + 1}]
	$w tag add active $line.0 $line.end
    }

    set data(activeIdx) $index
    return ""
}

#------------------------------------------------------------------------------
# tablelist::bboxSubCmd
#
# This procedure is invoked to process the tablelist bbox subcommand.
#------------------------------------------------------------------------------
proc tablelist::bboxSubCmd {win index} {
    upvar ::tablelist::ns${win}::data data

    set w $data(body)
    set dlineinfo [$w dlineinfo [expr {$index + 1}].0]
    if {$data(itemCount) == 0 || [string compare $dlineinfo ""] == 0} {
	return {}
    }

    foreach {x y width height baselinePos} $dlineinfo {
	lappend bbox [expr {$x + [winfo x $w]}] [expr {$y + [winfo y $w]}] \
		     $width [expr {$height - 2*$data(-selectborderwidth) - 1}]
    }
    return $bbox
}

#------------------------------------------------------------------------------
# tablelist::cellcgetSubCmd
#
# This procedure is invoked to process the tablelist cellcget subcommand.
#------------------------------------------------------------------------------
proc tablelist::cellcgetSubCmd {win row col opt} {
    variable cellConfigSpecs
    upvar ::tablelist::ns${win}::data data

    #
    # Return the value of the specified cell configuration option
    #
    switch -- [mwutil::fullConfigOpt $opt cellConfigSpecs] {
	-text {
	    set item [lindex $data(itemList) $row]
	    return [lindex $item $col]
	}

	default {
	    set item [lindex $data(itemList) $row]
	    set key [lindex $item end]
	    if {[info exists data($key-$col$opt)]} {
		return $data($key-$col$opt)
	    } else {
		return ""
	    }
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::cellconfigureSubCmd
#
# This procedure is invoked to process the tablelist cellconfigure subcommand.
#------------------------------------------------------------------------------
proc tablelist::cellconfigureSubCmd {win row col argList} {
    variable cellConfigSpecs
    upvar ::tablelist::ns${win}::data data

    #
    # Build an array from the cell configuration options and their values
    #
    set item [lindex $data(itemList) $row]
    set cellConfigVals(-text) [lindex $item $col]
    set key [lindex $item end]
    foreach opt {-background -font -foreground -image
		 -selectbackground -selectforeground} {
	if {[info exists data($key-$col$opt)]} {
	    set cellConfigVals($opt) $data($key-$col$opt)
	} else {
	    set cellConfigVals($opt) ""
	}
    }

    return [mwutil::configSubCmd $win cellConfigSpecs cellConfigVals \
	    "tablelist::doCellConfig $row $col" $argList]
}

#------------------------------------------------------------------------------
# tablelist::columncgetSubCmd
#
# This procedure is invoked to process the tablelist columncget subcommand.
#------------------------------------------------------------------------------
proc tablelist::columncgetSubCmd {win col opt} {
    variable colConfigSpecs
    upvar ::tablelist::ns${win}::data data

    #
    # Return the value of the specified column configuration option
    #
    switch -- [mwutil::fullConfigOpt $opt colConfigSpecs] {
	-align {
	    return [lindex $data(-columns) [expr {3*$col + 2}]]
	}

	-title {
	    return [lindex $data(-columns) [expr {3*$col + 1}]]
	}

	-width {
	    return [lindex $data(-columns) [expr {3*$col}]]
	}

	default {
	    if {[info exists data($col$opt)]} {
		return $data($col$opt)
	    } else {
		return ""
	    }
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::columnconfigureSubCmd
#
# This procedure is invoked to process the tablelist columnconfigure subcommand.
#------------------------------------------------------------------------------
proc tablelist::columnconfigureSubCmd {win col argList} {
    variable colConfigSpecs
    upvar ::tablelist::ns${win}::data data

    #
    # Build an array from the column configuration options and their values
    #
    set n [expr {3*$col}]
    foreach opt {-width -title -align} {
	set colConfigVals($opt) [lindex $data(-columns) $n]
	incr n
    }
    foreach opt {-background -font -foreground -formatcommand -hide
		 -labelalign -labelbackground -labelborderwidth -labelcommand
		 -labelfont -labelforeground -labelheight -labelimage
		 -labelpady -labelrelief -resizable -selectbackground
		 -selectforeground -showarrow -sortcommand -sortmode} {
	if {[info exists data($col$opt)]} {
	    set colConfigVals($opt) $data($col$opt)
	} else {
	    set colConfigVals($opt) ""
	}
    }

    return [mwutil::configSubCmd $win colConfigSpecs colConfigVals \
	    "tablelist::doColConfig $col" $argList]
}

#------------------------------------------------------------------------------
# tablelist::curselectionSubCmd
#
# This procedure is invoked to process the tablelist curselection subcommand.
#------------------------------------------------------------------------------
proc tablelist::curselectionSubCmd win {
    upvar ::tablelist::ns${win}::data data

    #
    # Find the selected lines of the body text widget
    #
    set result {}
    set w $data(body)
    set selRange [$w tag nextrange select 1.0]
    while {[llength $selRange] != 0} {
	set selStart [lindex $selRange 0]
	set selEnd [lindex $selRange 1]
	lappend result [expr {int($selStart) - 1}]

	set selRange [$w tag nextrange select $selEnd]
    }
    return $result
}

#------------------------------------------------------------------------------
# tablelist::deleteSubCmd
#
# This procedure is invoked to process the tablelist delete subcommand.
#------------------------------------------------------------------------------
proc tablelist::deleteSubCmd {win first last updateListVar} {
    upvar ::tablelist::ns${win}::data data

    #
    # Adjust the range to fit within the existing items
    #
    if {$first < 0} {
	set first 0
    }
    if {$last >= $data(itemCount)} {
	set last $data(lastRow)
    }
    set count [expr {$last - $first + 1}]
    if {$count <= 0} {
	return ""
    }

    #
    # Check whether the width of any dynamic-width
    # column might be affected by the deletion
    #
    set w $data(body)
    if {$count == $data(itemCount)} {
	set colWidthsChanged 1				;# just to save time
    } else {
	set colWidthsChanged 0
	for {set idx $first} {$idx <= $last} {incr idx} {
	    set item [lindex $data(itemList) $idx]
	    set key [lindex $item end]
	    set col 0
	    foreach text [lrange $item 0 $data(lastCol)] \
		    colFont $data(colFontList) \
		    fmtCmdFlag $data(fmtCmdFlagList) \
		    {pixels alignment} $data(colList) {
		if {$data($col-hide) || $pixels != 0} {
		    incr col
		    continue
		}

		if {$fmtCmdFlag} {
		    set text [doFormat $data($col-formatcommand) $text]
		}
		if {[info exists data($key-$col-image)]} {
		    set imageWidth [image width $data($key-$col-image)]
		} else {
		    set imageWidth 0
		}
		if {[info exists data($key-$col-font)]} {
		    set cellFont $data($key-$col-font)
		} elseif {[info exists data($key-font)]} {
		    set cellFont $data($key-font)
		} else {
		    set cellFont $colFont
		}
		adjustElem $win text imageWidth $cellFont $pixels $alignment
		set textWidth [font measure $cellFont -displayof $win $text]
		set elemWidth [expr {$imageWidth + $textWidth}]
		if {$elemWidth == $data($col-width) &&
		    [incr data($col-widestCnt) -1] == 0} {
		    set colWidthsChanged 1
		    break
		}

		incr col
	    }

	    if {$colWidthsChanged} {
		break
	    }
	}
    }

    #
    # Delete the given items and their tags from the body text widget.
    # Interestingly, for a large number of items it is much more efficient to
    # delete each line individually than to invoke a global delete command.
    #
    set textIdx1 [expr {$first + 1}].0
    set textIdx2 [expr {$first + 2}].0
    for {set idx $first} {$idx <= $last} {incr idx} {
	$w delete $textIdx1 $textIdx2

	set item [lindex $data(itemList) $idx]
	set key [lindex $item end]

	foreach opt {-background -font -foreground
		     -selectbackground -selectforeground} {
	    set tag $key$opt
	    if {[info exists data($tag)]} {
		$w tag delete $tag
		unset data($tag)
	    }
	}

	for {set col 0} {$col < $data(colCount)} {incr col} {
	    foreach opt {-background -font -foreground
			 -selectbackground -selectforeground} {
		set tag $key-$col$opt
		if {[info exists data($tag)]} {
		    $w tag delete $tag
		    unset data($tag)
		}
	    }
	    if {[info exists data($key-$col-image)]} {
		unset data($key-$col-image)
	    }
	}
    }

    #
    # Delete the given items from the internal list
    #
    set data(itemList) [lreplace $data(itemList) $first $last]
    incr data(itemCount) -$count
    incr data(lastRow) -$count

    #
    # Delete the given items from the list variable if needed
    #
    if {$data(hasListVar) && $updateListVar} {
	set traceCmd [list tablelist::traceProc $win]
	trace vdelete ::$data(-listvariable) wu $traceCmd
	upvar #0 $data(-listvariable) var
	set var [lreplace $var $first $last]
	trace variable ::$data(-listvariable) wu $traceCmd
    }

    #
    # Adjust the height of the body text widget if necessary
    #
    if {$data(-height) <= 0} {
	$w configure -height $data(itemCount)
    }

    #
    # Adjust the columns if necessary, adjust the separators,
    # and redraw the stripes in the body text widget
    #
    if {$colWidthsChanged} {
	adjustColumns $win all 1
    }
    adjustSepsWhenIdle $win
    makeStripesWhenIdle $win

    #
    # Update the indices anchorIdx and activeIdx
    #
    if {$first <= $data(anchorIdx)} {
	incr data(anchorIdx) -$count
	if {$data(anchorIdx) < $first} {
	    set data(anchorIdx) $first
	}
    }
    if {$last < $data(activeIdx)} {
	incr data(activeIdx) -$count
    } elseif {$first <= $data(activeIdx)} {
	set data(activeIdx) $first
	if {$data(activeIdx) >= $data(itemCount) && $data(itemCount) > 0} {
	    set data(activeIdx) $data(lastRow)
	}
    }

    return ""
}

#------------------------------------------------------------------------------
# tablelist::getSubCmd
#
# This procedure is invoked to process the tablelist get subcommand.
#------------------------------------------------------------------------------
proc tablelist::getSubCmd {win first last idxCount} {
    upvar ::tablelist::ns${win}::data data

    #
    # Adjust the range to fit within the existing items
    #
    if {$first >= $data(itemCount)} {
	return {}
    }
    if {$last >= $data(itemCount)} {
	set last $data(lastRow)
    }
    if {$first < 0} {
	set first 0
    }
    if {$first > $last} {
	return {}
    }

    #
    # Get the given items from the internal list
    #
    if {$idxCount == 1} {
	set item [lindex $data(itemList) $first]
	return [lrange $item 0 $data(lastCol)]
    } else {
	set result {}
	for {set idx $first} {$idx <= $last} {incr idx} {
	    set item [lindex $data(itemList) $idx]
	    lappend result [lrange $item 0 $data(lastCol)]
	}
	return $result
    }
}

#------------------------------------------------------------------------------
# tablelist::insertSubCmd
#
# This procedure is invoked to process the tablelist insert subcommand.
#------------------------------------------------------------------------------
proc tablelist::insertSubCmd {win index argList updateListVar} {
    upvar ::tablelist::ns${win}::data data

    set argCount [llength $argList]
    if {$argCount == 0} {
	return ""
    }

    if {$index < 0} {
	set index 0
    }

    #
    # Insert the items into the body text widget and into the internal list
    #
    set updateListVar [expr {$data(hasListVar) && $updateListVar}]
    set w $data(body)
    set savedCount $data(itemCount)
    set colWidthsChanged 0
    set idx $index
    set line [expr {$index + 1}]
    foreach item $argList {
	set item [adjustItem $item $data(colCount)]
	if {$data(itemCount) != 0} {
	    $w insert $line.0 \n
	}
	set insertArgs {}

	set col 0
	foreach text $item \
		colFont $data(colFontList) \
		colTags $data(colTagsList) \
		fmtCmdFlag $data(fmtCmdFlagList) \
		{pixels alignment} $data(colList) {
	    if {$data($col-hide)} {
		incr col
		continue
	    }

	    #
	    # Update the column width or clip the element if necessary
	    #
	    if {$fmtCmdFlag} {
		set text [doFormat $data($col-formatcommand) $text]
	    }
	    if {$pixels == 0} {			;# convention: dynamic width
		set textWidth [font measure $colFont -displayof $win $text]
		if {$textWidth > $data($col-width)} {
		    set data($col-width) $textWidth
		    set data($col-widestCnt) 1
		    set colWidthsChanged 1
		}
	    } else {
		incr pixels $data($col-delta)
		set text [strRangeExt $win $text $colFont $pixels $alignment]
	    }

	    lappend insertArgs \t$text\t $colTags
	    incr col
	}

	#
	# Insert the item into the body text widget
	#
	eval [list $w insert $line.0] $insertArgs

	#
	# Insert the item into the list variable if needed
	#
	if {$updateListVar} {
	    set traceCmd [list tablelist::traceProc $win]
	    trace vdelete ::$data(-listvariable) wu $traceCmd
	    upvar #0 $data(-listvariable) var
	    if {$idx == $data(itemCount)} {
		lappend var $item		;# this works much faster
	    } else {
		set var [linsert $var $idx $item]
	    }
	    trace variable ::$data(-listvariable) wu $traceCmd
	}

	#
	# Insert the item into the internal list
	#
	lappend item k[incr data(seqNum)]
	if {$idx == $data(itemCount)} {
	    lappend data(itemList) $item	;# this works much faster
	} else {
	    set data(itemList) [linsert $data(itemList) $idx $item]
	}

	incr idx
	incr line
	incr data(itemCount)
    }
    set data(lastRow) [expr {$data(itemCount) - 1}]

    #
    # Adjust the height of the body text widget if necessary
    #
    if {$data(-height) <= 0} {
	$w configure -height $data(itemCount)
    }

    #
    # Adjust the horizontal view in the body text 
    # widget if the tablelist was previously empty
    #
    if {$savedCount == 0} {
	$w xview moveto [lindex [$data(hdrTxt) xview] 0]
    }

    #
    # Adjust the columns if necessary, adjust the separators,
    # and redraw the stripes in the body text widget
    #
    if {$colWidthsChanged} {
	adjustColumns $win -1 1
    }
    adjustSepsWhenIdle $win
    makeStripesWhenIdle $win

    #
    # Update the indices anchorIdx and activeIdx
    #
    if {$index <= $data(anchorIdx)} {
	incr data(anchorIdx) $argCount
    }
    if {$index <= $data(activeIdx)} {
	incr data(activeIdx) $argCount
	if {$data(activeIdx) >= $data(itemCount) && $data(itemCount) > 0} {
	    set data(activeIdx) $data(lastRow)
	}
    }

    return ""
}

#------------------------------------------------------------------------------
# tablelist::rowcgetSubCmd
#
# This procedure is invoked to process the tablelist rowcget subcommand.
#------------------------------------------------------------------------------
proc tablelist::rowcgetSubCmd {win row opt} {
    variable rowConfigSpecs
    upvar ::tablelist::ns${win}::data data

    #
    # Return the value of the specified row configuration option
    #
    switch -- [mwutil::fullConfigOpt $opt rowConfigSpecs] {
	-text {
	    set item [lindex $data(itemList) $row]
	    return [lrange $item 0 $data(lastCol)]
	}

	-selectable {
	    set item [lindex $data(itemList) $row]
	    set key [lindex $item end]
	    if {[info exists data($key$opt)]} {
		return $data($key$opt)
	    } else {
		return 1
	    }
	}

	default {
	    set item [lindex $data(itemList) $row]
	    set key [lindex $item end]
	    if {[info exists data($key$opt)]} {
		return $data($key$opt)
	    } else {
		return ""
	    }
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::rowconfigureSubCmd
#
# This procedure is invoked to process the tablelist rowconfigure subcommand.
#------------------------------------------------------------------------------
proc tablelist::rowconfigureSubCmd {win row argList} {
    variable rowConfigSpecs
    upvar ::tablelist::ns${win}::data data

    #
    # Build an array from the row configuration options and their values
    #
    set item [lindex $data(itemList) $row]
    set rowConfigVals(-text) [lrange $item 0 $data(lastCol)]
    set key [lindex $item end]
    if {[info exists data($key-selectable)]} {
	set rowConfigVals(-selectable) $data($key-selectable)
    } else {
	set rowConfigVals(-selectable) 1
    }
    foreach opt {-background -font -foreground
		 -selectbackground -selectforeground} {
	if {[info exists data($key$opt)]} {
	    set rowConfigVals($opt) $data($key$opt)
	} else {
	    set rowConfigVals($opt) ""
	}
    }

    return [mwutil::configSubCmd $win rowConfigSpecs rowConfigVals \
	    "tablelist::doRowConfig $row" $argList]
}

#------------------------------------------------------------------------------
# tablelist::scanSubCmd
#
# This procedure is invoked to process the tablelist scan subcommand.
#------------------------------------------------------------------------------
proc tablelist::scanSubCmd {win opt x y} {
    upvar ::tablelist::ns${win}::data data

    set w $data(body)
    incr x -[winfo x $w]
    incr y -[winfo y $w]

    $w scan $opt $x $y
    $data(hdrTxt) scan $opt $x $y
    return ""
}

#------------------------------------------------------------------------------
# tablelist::seeSubCmd
#
# This procedure is invoked to process the tablelist see subcommand.
#------------------------------------------------------------------------------
proc tablelist::seeSubCmd {win index} {
    upvar ::tablelist::ns${win}::data data

    #
    # Adjust the view in the body text widget
    #
    set w $data(body)
    set fraction [lindex [$w xview] 0]
    $w see [expr {$index + 1}].0
    $w xview moveto $fraction
    return ""
}

#------------------------------------------------------------------------------
# tablelist::selectionSubCmd
#
# This procedure is invoked to process the tablelist selection subcommand.
#------------------------------------------------------------------------------
proc tablelist::selectionSubCmd {win opt first last idxCount} {
    upvar ::tablelist::ns${win}::data data

    switch $opt {
	anchor {
	    if {$idxCount != 1} {
		mwutil::wrongNumArgs "$win selection $opt index"
	    }

	    #
	    # Adjust the index to fit within the existing items
	    #
	    if {$first >= $data(itemCount)} {
		set first $data(lastRow)
	    }
	    if {$first < 0} {
		set first 0
	    }

	    set data(anchorIdx) $first
	    return ""
	}

	clear {
	    #
	    # Swap the indices if necessary
	    #
	    if {$last < $first} {
		set tmp $first
		set first $last
		set last $tmp
	    }

	    #
	    # Find the selected lines of the body text widget
	    # in the text range specified by the two indices
	    #
	    set w $data(body)
	    set firstTextIdx [expr {$first + 1}].0
	    set lastTextIdx [expr {$last + 1}].end
	    set selRange [$w tag nextrange select $firstTextIdx $lastTextIdx]
	    while {[llength $selRange] != 0} {
		set selStart [lindex $selRange 0]
		set selEnd [lindex $selRange 1]

		$w tag remove select $selStart $selEnd

		#
		# Handle the -(select)background and -(select)foreground cell
		# and column configuration options for each element of the row
		#
		set item [lindex $data(itemList) [expr {int($selStart) - 1}]]
		set key [lindex $item end]
		set textIdx1 $selStart
		for {set col 0} {$col < $data(colCount)} {incr col} {
		    if {$data($col-hide)} {
			continue
		    }

		    set textIdx2 \
			[$w search \t $textIdx1+1c "$selStart lineend"]+1c
		    foreach optTail {background foreground} {
			foreach tag [list $col-select$optTail \
				     $key-select$optTail \
				     $key-$col-select$optTail] {
			    if {[info exists data($tag)]} {
				$w tag remove $tag $textIdx1 $textIdx2
			    }
			}
			foreach tag [list $col-$optTail $key-$optTail \
				     $key-$col-$optTail] {
			    if {[info exists data($tag)]} {
				$w tag add $tag $textIdx1 $textIdx2
			    }
			}
		    }
		    set textIdx1 $textIdx2
		}

		set selRange [$w tag nextrange select $selEnd $lastTextIdx]
	    }

	    return ""
	}

	includes {
	    if {$idxCount != 1} {
		mwutil::wrongNumArgs "$win selection $opt index"
	    }

	    set tagNames [$data(body) tag names [expr {$first + 1}].0]
	    if {[lsearch -exact $tagNames select] >= 0} {
		return 1
	    } else {
		return 0
	    }
	}

	set {
	    #
	    # Swap the indices if necessary and adjust
	    # the range to fit within the existing items
	    #
	    if {$last < $first} {
		set tmp $first
		set first $last
		set last $tmp
	    }
	    if {$first < 0} {
		set first 0
	    }
	    if {$last >= $data(itemCount)} {
		set last $data(lastRow)
	    }

	    set w $data(body)
	    for {set idx $first; set line [expr {$first + 1}]} \
		{$idx <= $last} {incr idx; incr line} {
		#
		# Nothing to do if the row is already selected
		#
		if {[lsearch -exact [$w tag names $line.0] select] >= 0} {
		    continue
		}

		#
		# Check whether the row is selectable
		#
		set item [lindex $data(itemList) $idx]
		set key [lindex $item end]
		if {[info exists data($key-selectable)]} {     ;# <==> not sel.
		    continue
		}

		$w tag add select $line.0 $line.end

		#
		# Handle the -(select)background and -(select)foreground cell
		# and column configuration options for each element of the row
		#
		set textIdx1 $line.0
		for {set col 0} {$col < $data(colCount)} {incr col} {
		    if {$data($col-hide)} {
			continue
		    }

		    set textIdx2 [$w search \t $textIdx1+1c $line.end]+1c
		    foreach optTail {background foreground} {
			foreach tag [list $col-select$optTail \
				     $key-select$optTail \
				     $key-$col-select$optTail] {
			    if {[info exists data($tag)]} {
				$w tag add $tag $textIdx1 $textIdx2
			    }
			}
			foreach tag [list $col-$optTail $key-$optTail \
				     $key-$col-$optTail] {
			    if {[info exists data($tag)]} {
				$w tag remove $tag $textIdx1 $textIdx2
			    }
			}
		    }
		    set textIdx1 $textIdx2
		}
	    }

	    #
	    # If the selection is exported and there are any selected
	    # rows in the widget then make win the new owner of the
	    # PRIMARY selection and register a callback to be invoked
	    # when it loses ownership of the PRIMARY selection
	    #
	    if {$data(-exportselection) &&
		[llength [$w tag nextrange select 1.0]] != 0} {
		selection own -command \
			[list ::tablelist::lostSelection $win] $win
	    }

	    return ""
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::sortSubCmd
#
# This procedure is invoked to process the tablelist sort and sortbycolumn
# subcommands.
#------------------------------------------------------------------------------
proc tablelist::sortSubCmd {win col order} {
    upvar ::tablelist::ns${win}::data data

    #
    # Save the keys corresponding to anchorIdx and activeIdx 
    #
    foreach type {anchor active} {
	set item [lindex $data(itemList) $data(${type}Idx)]
	set ${type}Key [lindex $item end]
    }

    #
    # Save the keys of the selected items
    #
    set selKeys {}
    set w $data(body)
    set selRange [$w tag nextrange select 1.0]
    while {[llength $selRange] != 0} {
	set selStart [lindex $selRange 0]
	set selEnd [lindex $selRange 1]
	set item [lindex $data(itemList) [expr {int($selStart) - 1}]]
	lappend selKeys [lindex $item end]

	set selRange [$w tag nextrange select $selEnd]
    }

    #
    # Sort the item list and update the sort info
    #
    if {$col < 0} {				;# not sorting by a column
	if {[string compare $data(-sortcommand) ""] == 0} {
	    return -code error \
		   "value of the -sortcommand option is empty"
	}

	set data(itemList) \
	    [lsort -command $data(-sortcommand) $order $data(itemList)]
    } else {					;# sorting by a column
	if {[string compare $data($col-sortmode) command] == 0} {
	    if {[info exists data($col-sortcommand)]} {
		set data(itemList) [lsort -command $data($col-sortcommand) \
				    -index $col $order $data(itemList)]
	    } else {
		return -code error \
		       "value of the -sortcommand option for\
			column $col is missing or empty"
	    }
	} else {
	    set data(itemList) [lsort -$data($col-sortmode) \
				-index $col $order $data(itemList)]
	}
    }
    set data(sortCol) $col
    set data(sortOrder) [string range $order 1 end]

    #
    # Update anchorIdx and activeIdx
    #
    foreach type {anchor active} {
	upvar 0 ${type}Key key
	if {[string compare $key ""] != 0} {
	    set data(${type}Idx) [lsearch $data(itemList) "* $key"]
	}
    }

    #
    # Unmanage the canvas and adjust the columns
    #
    set canvas $data(hdrTxtFrCanv)
    place forget $canvas
    set colOfUnknownWidth $data(arrowCol)
    set data(arrowCol) -1
    adjustColumns $win $colOfUnknownWidth 1

    #
    # Check whether an up- or down-arrow is to be displayed
    #
    if {$col >= 0 && $data(-showarrow) && $data($col-showarrow)} {
	#
	# Configure the canvas and draw the arrows
	#
	set data(arrowCol) $col
	configCanvas $win
	drawArrows $win

	#
	# Make sure the arrow will fit into the static-width
	# column by enlarging the latter if necessary
	#
	set idx [expr {2*$col}]
	set pixels [lindex $data(colList) $idx]
	if {$pixels != 0 && $pixels < $data(arrowWidth)} {
	    set data(colList) \
		[lreplace $data(colList) $idx $idx $data(arrowWidth)]
	    set idx [expr {3*$col}]
	    set data(-columns) \
		[lreplace $data(-columns) $idx $idx -$data(arrowWidth)]
	}

	#
	# Adjust the columns; this will also place the canvas into the label
	#
	adjustColumns $win $col 1

	#
	# Define for the binding tag $canvas the binding scripts
	# obtained from those of $label by replacing the %x field with
	# [expr {%x + [winfo x $canvas] - [winfo x $label]}] and the %y
	# field with [expr {%y + [winfo y $canvas] - [winfo y $label]}]
	# 
	set label $data(hdrTxtFrLbl)$col
	set _canvas [list $canvas]
	set _label  [list $label]
	foreach event [bind $label] {
	    set script [bind $label $event]
	    regsub -all %x $script {$tablelist::x} script
	    regsub -all %y $script {$tablelist::y} script

	    bind $canvas $event [format {
		set tablelist::x [expr {%%x + [winfo x %s] - [winfo x %s]}]
		set tablelist::y [expr {%%y + [winfo y %s] - [winfo y %s]}]
		%s
	    } $_canvas $_label $_canvas $_label $script]
	}
    }

    #
    # Delete the items from the body text widget and insert the sorted ones.
    # Interestingly, for a large number of items it is much more efficient
    # to empty each line individually than to invoke a global delete command.
    #
    set line 1
    foreach item $data(itemList) {
	#
	# Empty the line, clip the elements if necessary,
	# and insert them with the corresponding tags
	#
	$w delete $line.0 $line.end
	set key [lindex $item end]
	set rowTags {}
	foreach opt {-background -font -foreground} {
	    if {[info exists data($key$opt)]} {
		lappend rowTags $key$opt
	    }
	}
	set col 0
	foreach text [lrange $item 0 $data(lastCol)] \
		colFont $data(colFontList) \
		colTags $data(colTagsList) \
		fmtCmdFlag $data(fmtCmdFlagList) \
		{pixels alignment} $data(colList) {
	    if {$data($col-hide)} {
		incr col
		continue
	    }

	    #
	    # Adjust the cell text and the image width
	    #
	    if {$fmtCmdFlag} {
		set text [doFormat $data($col-formatcommand) $text]
	    }
	    if {[info exists data($key-$col-image)]} {
		set image $data($key-$col-image)
		set imageWidth [image width $image]
	    } else {
		set image ""
		set imageWidth 0
	    }
	    if {$pixels != 0} {			;# convention: static width
		incr pixels $data($col-delta)
	    }
	    if {[info exists data($key-$col-font)]} {
		set cellFont $data($key-$col-font)
	    } elseif {[info exists data($key-font)]} {
		set cellFont $data($key-font)
	    } else {
		set cellFont $colFont
	    }
	    adjustElem $win text imageWidth $cellFont $pixels $alignment

	    #
	    # Insert the text and the image
	    #
	    set tagNames {}
	    eval lappend tagNames $colTags $rowTags
	    foreach opt {-background -font -foreground} {
		if {[info exists data($key-$col$opt)]} {
		    lappend tagNames $key-$col$opt
		}
	    }
	    if {$imageWidth == 0} {
		$w insert $line.end \t$text\t $tagNames
	    } else {
		$w insert $line.end \t\t $tagNames
		insertElem $w $line.end-1c $text $image $imageWidth $alignment
	    }

	    incr col
	}

	incr line
    }

    #
    # Restore the stripes in the body text widget
    #
    makeStripesWhenIdle $win

    #
    # Select the items that were selected before
    #
    foreach key $selKeys {
	set idx [lsearch $data(itemList) "* $key"]
	selectionSubCmd $win set $idx $idx 2
    }

    #
    # Disable the body text widget if it was disabled before
    #
    if {$data(isDisabled)} {
	$w tag add disabled 1.0 end
	$w tag configure select -borderwidth 0
    }

    #
    # Replace the contents of the list variable if present
    #
    if {$data(hasListVar)} {
	set traceCmd [list tablelist::traceProc $win]
	trace vdelete ::$data(-listvariable) wu $traceCmd
	upvar #0 $data(-listvariable) var
	set var {}
	foreach item $data(itemList) {
	    lappend var [lrange $item 0 $data(lastCol)]
	}
	trace variable ::$data(-listvariable) wu $traceCmd
    }

    return ""
}

#------------------------------------------------------------------------------
# tablelist::xviewSubCmd
#
# This procedure is invoked to process the tablelist xview subcommand.
#------------------------------------------------------------------------------
proc tablelist::xviewSubCmd {win argList} {
    upvar ::tablelist::ns${win}::data data

    switch [llength $argList] {
	0 {
	    #
	    # Command: $win xview
	    #
	    return [$data(hdrTxt) xview]
	}

	1 {
	    #
	    # Command: $win xview units
	    #
	    set units [lindex $argList 0]
	    format %d $units		;# integer check with error message
	    foreach w [list $data(hdrTxt) $data(body)] {
		$w xview moveto 0
		$w xview scroll $units units
	    }
	    return ""
	}

	default {
	    #
	    # Command: $win xview moveto fraction
	    #	       $win xview scroll number what
	    #
	    foreach w [list $data(hdrTxt) $data(body)] {
		eval [list $w xview] $argList
	    }
	    return ""
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::yviewSubCmd
#
# This procedure is invoked to process the tablelist yview subcommand.
#------------------------------------------------------------------------------
proc tablelist::yviewSubCmd {win argList} {
    upvar ::tablelist::ns${win}::data data

    set w $data(body)
    switch [llength $argList] {
	0 {
	    #
	    # Command: $win yview
	    #
	    return [$w yview]
	}

	1 {
	    #
	    # Command: $win yview index
	    #
	    set index [rowIndex $win [lindex $argList 0] 0]
	    set fraction [lindex [$w xview] 0]
	    $w yview [expr {$index + 1}].0
	    $w xview moveto $fraction
	    return ""
	}

	default {
	    #
	    # Command: $win yview moveto fraction
	    #	       $win yview scroll number what
	    #
	    # We must check the word following "yview" before passing
	    # the arguments to the body text widget because of some
	    # additional options supported by its yview subcommand.
	    #
	    set opt [lindex $argList 0]
	    if {[string first $opt moveto] == 0 ||
		[string first $opt scroll] == 0} {
		return [eval [list $w yview] $argList]
	    } else {
		return -code error \
		       "unknown option \"$opt\": must be moveto or scroll"
	    }
	}
    }
}

#
# Private callback procedures
# ===========================
#

#------------------------------------------------------------------------------
# tablelist::focusCtrl
#
# Determines whether the body text child w of a tablelist widget will receive
# the input focus during keyboard traversal.
#------------------------------------------------------------------------------
proc tablelist::focusCtrl w {
    set win [winfo parent $w]
    upvar ::tablelist::ns${win}::data data
    set val $data(-takefocus)

    switch -- $val {
	0 - 1 - "" {
	    return $val
	}

	default {
	    return [uplevel #0 $val [list $win]]
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::fetchSelection
#
# This procedure is invoked when the PRIMARY selection is owned by the
# tablelist widget win and someone attempts to retrieve it as a STRING.  It
# returns part or all of the selection, as given by offset and maxChars.  The
# string which is to be (partially) returned is built by joining all of the
# elements of the selected rows together with tabs and the rows themselves with
# newlines.
#------------------------------------------------------------------------------
proc tablelist::fetchSelection {win offset maxChars} {
    upvar ::tablelist::ns${win}::data data

    if {!$data(-exportselection)} {
	return ""
    }

    set selection ""
    set gotItem false
    foreach idx [curselectionSubCmd $win] {
	if {$gotItem} {
	    append selection \n
	}

	append selection [join [getSubCmd $win $idx $idx 1] \t]
	set gotItem true
    }

    return [string range $selection $offset [expr {$offset + $maxChars - 1}]]
}

#------------------------------------------------------------------------------
# tablelist::lostSelection
#
# This procedure is invoked when the tablelist widget win loses ownership of
# the PRIMARY selection.  It deselects all items of the widget with the aid of
# the selectionSubCmd procedure if the selection is exported.
#------------------------------------------------------------------------------
proc tablelist::lostSelection win {
    upvar ::tablelist::ns${win}::data data

    if {$data(-exportselection)} {
	selectionSubCmd $win clear 0 $data(lastRow) 2
    }
}

#
# Private procedures used in bindings
# ===================================
#

#------------------------------------------------------------------------------
# tablelist::addActiveTag
#
# This procedure is invoked when the tablelist widget win gains the keyboard
# focus.  It adds the "active" tag to the line that displays the active element
# of the widget in its body text child.
#------------------------------------------------------------------------------
proc tablelist::addActiveTag win {
    upvar ::tablelist::ns${win}::data data

    set line [expr {$data(activeIdx) + 1}]
    $data(body) tag add active $line.0 $line.end
}

#------------------------------------------------------------------------------
# tablelist::removeActiveTag
#
# This procedure is invoked when the tablelist widget win loses the keyboard
# focus.  It removes the "active" tag from the line that displays the active
# element of the widget in its body text child.
#------------------------------------------------------------------------------
proc tablelist::removeActiveTag win {
    upvar ::tablelist::ns${win}::data data

    set line [expr {$data(activeIdx) + 1}]
    $data(body) tag remove active $line.0 $line.end
}

#------------------------------------------------------------------------------
# tablelist::cleanup
#
# This procedure is invoked when the tablelist widget win is destroyed.  It
# executes some cleanup operations.
#------------------------------------------------------------------------------
proc tablelist::cleanup win {
    upvar ::tablelist::ns${win}::data data

    #
    # Cancel the execution of all delayed adjustSeps, makeStripes,
    # stretchColumns, synchronize, redisplay, and redisplayCol commands
    #
    foreach afterId {sepsId stripesId stretchId syncId} {
	if {[info exists data($afterId)]} {
	    after cancel $data($afterId)
	}
    }
    foreach name [array names data *redispId] {
	after cancel $data($name)
    }

    #
    # If there is a list variable associated with the
    # widget then remove the trace set on this variable
    #
    if {$data(hasListVar)} {
	trace vdelete ::$data(-listvariable) wu [list tablelist::traceProc $win]
    }

    namespace delete ::tablelist::ns$win
    catch {rename ::$win ""}
}

#------------------------------------------------------------------------------
# tablelist::tablelistAutoScan
#
# This is a modified version of the Tk library procedure tk(::)ListboxAutoScan.
# It is invoked when the mouse leaves the body text child of a tablelist
# widget.  It scrolls the child and reschedules itself as an after command so
# that the child continues to scroll until the mouse moves back into the window
# or the mouse button is released.
#------------------------------------------------------------------------------
proc tablelist::tablelistAutoScan win {
    if {![winfo exists $win]} {
	return ""
    }

    if {[array exists ::tk::Priv]} {
	set x $::tk::Priv(x)
	set y $::tk::Priv(y)
    } else {
	set x $::tkPriv(x)
	set y $::tkPriv(y)
    }

    set w [::$win bodypath]
    set _x [expr {$x - [winfo x $w]}]
    set _y [expr {$y - [winfo y $w]}]

    if {$_y >= [winfo height $w]} {
	::$win yview scroll 1 units
    } elseif {$_y < 0} {
	::$win yview scroll -1 units
    } elseif {$_x >= [winfo width $w]} {
	::$win xview scroll 2 units
    } elseif {$_x < 0} {
	::$win xview scroll -2 units
    } else {
	return ""
    }

    if {[array exists ::tk::Priv]} {
	tk::ListboxMotion $win [::$win index @$x,$y]
	set ::tk::Priv(afterId) \
	    [after 50 [list tablelist::tablelistAutoScan $win]]
    } else {
	tkListboxMotion $win [::$win index @$x,$y]
	set ::tkPriv(afterId) \
	    [after 50 [list tablelist::tablelistAutoScan $win]]
    }
}

#------------------------------------------------------------------------------
# tablelist::labelEnter
#
# This procedure is invoked when the mouse pointer enters the col'th label of
# the tablelist widget win, or is moving within that label.  It updates the
# cursor, depending on whether the pointer is on the right border of the label
# or not.
#------------------------------------------------------------------------------
proc tablelist::labelEnter {win col x} {
    upvar ::tablelist::ns${win}::data data

    set w $data(hdrTxtFrLbl)$col
    configLabel $w -cursor $data(-cursor)
    if {$data(isDisabled)} {
	return ""
    }

    if {$data(-resizablecolumns) && $data($col-resizable) &&
	$x >= [winfo width $w] - [$w cget -borderwidth] - 4} {
	configLabel $w -cursor $data(-resizecursor)
    }
}

#------------------------------------------------------------------------------
# tablelist::labelB1Down
#
# This procedure is invoked when mouse button 1 is pressed in the col'th label
# of the tablelist widget win.  If the pointer is on the right border of the
# label then the procedure records its x-coordinate relative to the label, the
# width of the column, and some other data needed later.  Otherwise it saves
# the label's relief so it can be restored later, and changes the relief to
# sunken.
#------------------------------------------------------------------------------
proc tablelist::labelB1Down {win col x} {
    upvar ::tablelist::ns${win}::data data

    if {$data(isDisabled) ||
	[info exists data(x)]} {		;# resize operation in progress
	return ""
    }

    set w $data(hdrTxtFrLbl)$col
    set labelWidth [winfo width $w]

    if {$data(-resizablecolumns) && $data($col-resizable) &&
	$x >= $labelWidth - [$w cget -borderwidth] - 4} {
	set data(clickedLblCol) $col
	set data(x) $x

	set data(curColWidth) [expr {$labelWidth - 2*$data(charWidth)}]
	set data(configColWidth) [lindex $data(-columns) [expr {3*$col}]]

	if {$col == $data(arrowCol)} {
	    set data(minColWidth) $data(arrowWidth)
	} else {
	    set data(minColWidth) 1
	}

	set topWin [winfo toplevel $win]
	set data(topBindEsc) [bind $topWin <Escape>]
	bind $topWin <Escape> [list tablelist::escape $win]
    } elseif {[info exists data($col-labelcommand)] ||
	      [string compare $data(-labelcommand) ""] != 0} {
	set data(clickedLblCol) $col
	set data(inClickedLabel) 1
	set data(relief) [$w cget -relief]
	configLabel $w -relief sunken
    }
}

#------------------------------------------------------------------------------
# tablelist::labelB1Motion
#
# This procedure is invoked to process mouse motion events in a label of the
# tablelist widget win while button 1 is down.  If this event occured during a
# column resize operation then the procedure computes the difference between
# the pointer's new x-coordinate relative to that label and the one recorded by
# the last invocation of labelB1Down, and adjusts the width of the
# corresponding column accordingly.  This column is only redisplayed in the
# currently visible rows of the widget.
#------------------------------------------------------------------------------
proc tablelist::labelB1Motion {win x y} {
    upvar ::tablelist::ns${win}::data data

    if {$data(clickedLblCol) < 0} {
	return ""
    }

    set col $data(clickedLblCol)

    if {[info exists data(x)]} {		;# resize operation in progress
	set width [expr {$data(curColWidth) + $x - $data(x)}]
	if {$width >= $data(minColWidth)} {
	    set idx [expr {3*$col}]
	    set data(-columns) [lreplace $data(-columns) $idx $idx -$width]
	    set idx [expr {2*$col}]
	    set data(colList) [lreplace $data(colList) $idx $idx $width]
	    set data($col-delta) 0
	    adjustColumns $win -1 0
	    update idletasks
	    redisplayCol $win $col [rowIndex $win @0,0 0] \
				   [rowIndex $win @0,[winfo height $win] 0]
	}
    } else {
	#
	# The following code is needed because the event can also
	# occur in the canvas displaying an up- or down-arrow
	#
	set w $data(hdrTxtFrLbl)$col
	if {$x >= 0 && $x < [winfo width $w] &&
	    $y >= 0 && $y < [winfo height $w]} {
	    configLabel $w -relief sunken
	    set data(inClickedLabel) 1
	} else {
	    configLabel $w -relief $data(relief)
	    set data(inClickedLabel) 0
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::labelB1Enter
#
# This procedure is invoked when the mouse pointer enters a label of the
# tablelist widget win while mouse button 1 is down.  If no label was
# previously clicked then nothing happens.  Otherwise, if this event occured
# during a column resize operation then the procedure updates the mouse cursor
# accordingly.  Otherwise it changes the label's relief to sunken.
#------------------------------------------------------------------------------
proc tablelist::labelB1Enter win {
    upvar ::tablelist::ns${win}::data data

    if {$data(clickedLblCol) < 0} {
	return ""
    }

    set w $data(hdrTxtFrLbl)$data(clickedLblCol)
    configLabel $w -cursor $data(-cursor)

    if {[info exists data(x)]} {		;# resize operation in progress
	configLabel $w -cursor $data(-resizecursor)
	return ""
    }

    configLabel $w -relief sunken
    set data(inClickedLabel) 1
}

#------------------------------------------------------------------------------
# tablelist::labelB1Leave
#
# This procedure is invoked when the mouse pointer leaves a label of the
# tablelist widget win while mouse button 1 is down.  If no label was
# previously clicked then nothing happens.  Otherwise, if no column resize
# operation is in progress then the procedure restores the label's relief.
#------------------------------------------------------------------------------
proc tablelist::labelB1Leave {win x y} {
    upvar ::tablelist::ns${win}::data data

    if {$data(clickedLblCol) < 0 ||
	[info exists data(x)]} {		;# resize operation in progress
	return ""
    }

    set w $data(hdrTxtFrLbl)$data(clickedLblCol)

    #
    # The following code is needed because the event can also
    # occur in the canvas displaying an up- or down-arrow
    #
    if {$x >= 0 && $x < [winfo width $w] &&
	$y >= 0 && $y < [winfo height $w]} {
	return ""
    }

    configLabel $w -relief $data(relief)
    set data(inClickedLabel) 0
}

#------------------------------------------------------------------------------
# tablelist::labelB1Up
#
# This procedure is invoked when mouse button 1 is released in a previously
# clicked label of the tablelist widget win.  If this event occured during a
# column resize operation then the procedure redisplays that column.  Otherwise
# it restores the label's relief and invokes the command specified with the
# -labelcommand configuration option, passing to it the widget name and the
# column number as arguments.
#------------------------------------------------------------------------------
proc tablelist::labelB1Up win {
    upvar ::tablelist::ns${win}::data data

    if {$data(clickedLblCol) < 0} {
	return ""
    }

    set col $data(clickedLblCol)
    set w $data(hdrTxtFrLbl)$col

    if {[info exists data(x)]} {		;# resize operation in progress
	configLabel $w -cursor $data(-cursor)
	bind [winfo toplevel $win] <Escape> $data(topBindEsc)
	redisplayColWhenIdle $win $col
	if {$data(-width) <= 0} {
	    $data(hdr) configure -width $data(hdrPixels)
	}
	stretchColumnsWhenIdle $win
	unset data(x)
    } elseif {$data(inClickedLabel)} {
	configLabel $w -relief $data(relief)
	if {[info exists data($col-labelcommand)]} {
	    uplevel #0 $data($col-labelcommand) [list $win $col]
	} elseif {[string compare $data(-labelcommand) ""] != 0} {
	    uplevel #0 $data(-labelcommand) [list $win $col]
	}
    }

    set data(clickedLblCol) -1
}

#------------------------------------------------------------------------------
# tablelist::labelB3Down
#
# This procedure is invoked when mouse button 3 is pressed in the col'th label
# of the tablelist widget win.  It configures the width of the given column to
# be just large enough to hold all the elements (including the label).
#------------------------------------------------------------------------------
proc tablelist::labelB3Down {win col} {
    upvar ::tablelist::ns${win}::data data

    if {!$data(isDisabled) &&
	$data(-resizablecolumns) && $data($col-resizable)} {
	doColConfig $col $win -width 0
    }
}

#------------------------------------------------------------------------------
# tablelist::escape
#
# This procedure is invoked to process <Escape> events in the top-level window
# containing the tablelist widget win during a column resize operation.  The
# procedure cancels this operation and restores the initial width of the
# respective column.
#------------------------------------------------------------------------------
proc tablelist::escape win {
    upvar ::tablelist::ns${win}::data data

    set col $data(clickedLblCol)
    set w $data(hdrTxtFrLbl)$col
    configLabel $w -cursor $data(-cursor)
    update idletasks
    bind [winfo toplevel $win] <Escape> $data(topBindEsc)
    set idx [expr {3*$col}]
    setupColumns $win [lreplace $data(-columns) $idx $idx \
				$data(configColWidth)] 0
    adjustColumns $win $col 1
    redisplayCol $win $col [rowIndex $win @0,0 0] \
			   [rowIndex $win @0,[winfo height $win] 0]

    unset data(x)
    set data(clickedLblCol) -1
}

#
# Private helper procedures
# =========================
#

#------------------------------------------------------------------------------
# tablelist::rowIndex
#
# Checks the row index idx and returns either its numerical value or an error.
# endIsSize must be a boolean value: if true, end refers to the number of items
# in the tablelist, i.e., to the element just after the last one; if false, end
# refers to 1 less than the number of items, i.e., to the last element in the
# tablelist.
#------------------------------------------------------------------------------
proc tablelist::rowIndex {win idx endIsSize} {
    upvar ::tablelist::ns${win}::data data

    set idxLen [string length $idx]
    if {[string first $idx active] == 0 && $idxLen >= 2} {
	return $data(activeIdx)
    } elseif {[string first $idx anchor] == 0 && $idxLen >= 2} {
	return $data(anchorIdx)
    } elseif {[string first $idx end] == 0} {
	if {$endIsSize} {
	    return $data(itemCount)
	} else {
	    return $data(lastRow)
	}
    } elseif {[string compare [string index $idx 0] @] == 0} {
	if {[catch {$data(body) index $idx}] == 0} {
	    if {$data(itemCount) == 0} {
		return -1
	    } else {
		scan $idx @%d,%d x y
		incr x -[winfo x $data(body)]
		incr y -[winfo y $data(body)]
		set bodyIdx [$data(body) index @$x,$y]
		return [expr {int($bodyIdx) - 1}]
	    }
	} else {
	    return -code error \
		   "bad row index \"$idx\": must be active, anchor,\
		    end, @x,y, or a number"
	}
    } elseif {[catch {format %d $idx} index] == 0} {
	return $index
    } else {
	return -code error \
	       "bad row index \"$idx\": must be active, anchor,\
	        end, @x,y, or a number"
    }
}

#------------------------------------------------------------------------------
# tablelist::colIndex
#
# Checks the column index idx and returns either its numerical value or an
# error.  checkRange must be a boolean value: if true, it is additionally
# checked whether the numerical value corresponding to idx is at least 0 and
# less than the number of columns.
#------------------------------------------------------------------------------
proc tablelist::colIndex {win idx checkRange} {
    upvar ::tablelist::ns${win}::data data

    if {[string first $idx end] == 0} {
	return $data(lastCol)
    } elseif {[string compare [string index $idx 0] @] == 0} {
	if {[catch {$data(body) index $idx}] == 0} {
	    scan $idx @%d x
	    incr x -[winfo x $data(body)]
	    set bodyWidth [winfo width $data(body)]
	    if {$x >= $bodyWidth} {
		set x [expr {$bodyWidth - 1}]
	    } elseif {$x < 0} {
		set x 0
	    }
	    set x [expr {$x + [winfo rootx $data(body)]}]

	    set lastVisibleCol -1
	    for {set col 0} {$col < $data(colCount)} {incr col} {
		if {$data($col-hide)} {
		    continue
		}
		set lastVisibleCol $col
		set w $data(hdrTxtFrLbl)$col
		set wX [winfo rootx $w]
		if {$x >= $wX && $x < $wX + [winfo width $w]} {
		    return $col
		}
	    }
	    set index $lastVisibleCol
	} else {
	    return -code error \
		   "bad column index \"$idx\": must be end, @x,y, or a number"
	}
    } elseif {[catch {format %d $idx} index] != 0} {
	return -code error \
	       "bad column index \"$idx\": must be end, @x,y, or a number"
    }

    if {$checkRange && ($index < 0 || $index >= $data(colCount))} {
	return -code error "column index \"$idx\" out of range"
    } else {
	return $index
    }
}

#------------------------------------------------------------------------------
# tablelist::cellIndex
#
# Checks the cell index idx and returns either its value in the form row,col
# or an error.
#------------------------------------------------------------------------------
proc tablelist::cellIndex {win idx} {
    if {[string first $idx end] == 0} {
	return [rowIndex $win $idx 0],[colIndex $win $idx 0]
    } elseif {[string compare [string index $idx 0] @] == 0} {
	if {[catch {rowIndex $win $idx 0} row] == 0 &&
	    [catch {colIndex $win $idx 0} col] == 0} {
	    return $row,$col
	} else {
	    return -code error \
		   "bad cell index \"$idx\": must be end, @x,y, or\
		    row,col, where row must be active, anchor, end,\
		    or a number, and col must be end or a number"
	}
    } else {
	set lst [split $idx ,]
	if {[llength $lst] == 2 &&
	    [catch {rowIndex $win [lindex $lst 0] 0} row] == 0 &&
	    [catch {colIndex $win [lindex $lst 1] 0} col] == 0} {
	    return $row,$col
	} else {
	    return -code error \
		   "bad cell index \"$idx\": must be end, @x,y, or\
		    row,col, where row must be active, anchor, end,\
		    or a number, and col must be end or a number"
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::makeColFontAndTagLists
#
# Builds the lists data(colFontList) of the column fonts and data(colTagsList)
# of the column tag names.
#------------------------------------------------------------------------------
proc tablelist::makeColFontAndTagLists win {
    upvar ::tablelist::ns${win}::data data

    set widgetFont $data(-font)
    set data(colFontList) {}
    set data(colTagsList) {}

    for {set col 0} {$col < $data(colCount)} {incr col} {
	set tagNames {}

	if {[info exists data($col-font)]} {
	    lappend data(colFontList) $data($col-font)
	    lappend tagNames $col-font
	} else {
	    lappend data(colFontList) $widgetFont
	}

	foreach opt {-background -foreground} {
	    if {[info exists data($col$opt)]} {
		lappend tagNames $col$opt
	    }
	}

	lappend data(colTagsList) $tagNames
    }
}

#------------------------------------------------------------------------------
# tablelist::cellFont
#
# Returns the font to be used in the tablelist cell specified by win, key, and
# col.
#------------------------------------------------------------------------------
proc tablelist::cellFont {win key col} {
    upvar ::tablelist::ns${win}::data data

    if {[info exists data($key-$col-font)]} {
	return $data($key-$col-font)
    } elseif {[info exists data($key-font)]} {
	return $data($key-font)
    } else {
	return [lindex $data(colFontList) $col]
    }
}

#------------------------------------------------------------------------------
# tablelist::findCellTabs
#
# Searches for the tab characters within the col'th cell in the body text child
# of the tablelist widget win, starting at startIdx.  Assigns the index of the
# first tab to $idx1Name and the index of the second tab to $idx2Name.
#------------------------------------------------------------------------------
proc tablelist::findCellTabs {win startIdx col idx1Name idx2Name} {
    upvar ::tablelist::ns${win}::data data
    upvar $idx1Name idx1 $idx2Name idx2

    set w $data(body)
    set endIdx "$startIdx lineend"
    set idx1 $startIdx
    for {set n 0} {$n < $col} {incr n} {
	if {$data($n-hide)} {
	    continue
	}
	set idx1 [$w search \t $idx1+1c $endIdx]+1c
    }
    set idx2 [$w search \t $idx1+1c $endIdx]

    return ""
}

#------------------------------------------------------------------------------
# tablelist::create3DArrows
#
# Creates the items to be used later for drawing two up- or down-arrows with
# sunken relief and 3-D borders in the canvas w.
#------------------------------------------------------------------------------
proc tablelist::create3DArrows w {
    foreach state {normal disabled} {
	$w create polygon 0 0 0 0 0 0 -tags ${state}Triangle
	$w create line    0 0 0 0     -tags ${state}DarkLine
	$w create line    0 0 0 0     -tags ${state}LightLine
    }
}

#------------------------------------------------------------------------------
# tablelist::configCanvas
#
# Sets the background, width, and height of the canvas displaying an up- or
# down-arrow, fills the two arrows contained in the canvas, and saves its width
# in data(arrowWidth).
#------------------------------------------------------------------------------
proc tablelist::configCanvas win {
    upvar ::tablelist::ns${win}::data data

    set w $data(hdrTxtFrLbl)$data(arrowCol)
    set labelBg [$w cget -background]
    set labelFont [$w cget -font]
    if {[font metrics $labelFont -displayof $w -fixed]} {
	set spaces " "
    } else {
	set spaces "  "
    }

    set size [expr {[font measure $labelFont -displayof $w $spaces] + 2}]
    if {$size % 2 == 0} {
	incr size
    }

    set w $data(hdrTxtFrCanv)
    $w configure -background $labelBg -height $size -width $size
    fillArrow $w normal   $data(-arrowcolor)
    fillArrow $w disabled $data(-arrowdisabledcolor)

    set data(arrowWidth) $size
}

#------------------------------------------------------------------------------
# tablelist::drawArrows
#
# Draws the two arrows contained in the canvas associated with the tablelist
# widget win.
#------------------------------------------------------------------------------
proc tablelist::drawArrows win {
    upvar ::tablelist::ns${win}::data data

    switch $data(-incrarrowtype) {
	up {
	    switch $data(sortOrder) {
		increasing { set arrowType up }
		decreasing { set arrowType down }
	    }
	}

	down {
	    switch $data(sortOrder) {
		increasing { set arrowType down }
		decreasing { set arrowType up }
	    }
	}
    }

    set w $data(hdrTxtFrCanv)
    set maxX [expr {[$w cget -width] - 1}]
    set maxY [expr {[$w cget -height] - 1}]
    set midX [expr {$maxX / 2}]

    switch $arrowType {
	up {
	    foreach state {normal disabled} {
		$w coords ${state}Triangle  0 $maxY $maxX $maxY $midX 0
		$w coords ${state}DarkLine  $midX 0 0 $maxY
		$w coords ${state}LightLine 0 $maxY $maxX $maxY $midX 0
	    }
	}

	down {
	    foreach state {normal disabled} {
		$w coords ${state}Triangle  $maxX 0 0 0 $midX $maxY
		$w coords ${state}DarkLine  $maxX 0 0 0 $midX $maxY
		$w coords ${state}LightLine $midX $maxY $maxX 0
	    }
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::raiseArrow
#
# Raises one of the two arrows contained in the canvas w, according to the
# state argument.
#------------------------------------------------------------------------------
proc tablelist::raiseArrow {w state} {
    $w raise ${state}Triangle
    $w raise ${state}DarkLine
    $w raise ${state}LightLine
}

#------------------------------------------------------------------------------
# tablelist::fillArrow
#
# Fills one of the two arrows contained in the canvas w with the given color,
# or with (a slightly darker color than) the background color of the canvas if
# color is an empty string.  Also fills the arrow's borders with the
# corresponding 3-D shadow colors.  The state argument specifies the arrow to
# be processed.  Returns the properly formatted value of color.
#------------------------------------------------------------------------------
proc tablelist::fillArrow {w state color} {
    if {[string compare $color ""] == 0} {
	set origColor $color
	set color [$w cget -background]

	#
	# To get a better contrast, make the color slightly
	# darker by cutting 5% from each of its components
	#
	set maxIntens [lindex [winfo rgb $w white] 0]
	set len [string length [format %x $maxIntens]]
	foreach comp [winfo rgb $w $color] {
	    lappend rgb [expr {95*$comp/100}]
	}
	set color [eval format "#%0${len}x%0${len}x%0${len}x" $rgb]
    }

    getShadows $w $color darkColor lightColor

    $w itemconfigure ${state}Triangle  -fill $color
    $w itemconfigure ${state}DarkLine  -fill $darkColor
    $w itemconfigure ${state}LightLine -fill $lightColor

    if {[info exists origColor]} {
	return $origColor
    } else {
	return [$w itemcget ${state}Triangle -fill]
    }
}

#------------------------------------------------------------------------------
# tablelist::getShadows
#
# Computes the shadow colors for a 3-D border from a given (background) color.
# This is a modified Tcl-counterpart of the function TkpGetShadows() in the
# Tk distribution file unix/tkUnix3d.c.
#------------------------------------------------------------------------------
proc tablelist::getShadows {w color darkColorName lightColorName} {
    upvar $darkColorName darkColor $lightColorName lightColor

    set maxIntens [lindex [winfo rgb $w white] 0]
    set len [string length [format %x $maxIntens]]

    set rgb [winfo rgb $w $color]
    foreach {r g b} $rgb {}

    #
    # Compute the dark shadow color
    #
    if {[string compare $::tk_patchLevel 8.3.1] >= 0 &&
	$r*0.5*$r + $g*1.0*$g + $b*0.28*$b < $maxIntens*0.05*$maxIntens} {
	#
	# The background is already very dark: make the dark
	# color a little lighter than the background by increasing
	# each color component 1/4th of the way to $maxIntens
	#
	foreach comp $rgb {
	    lappend darkRGB [expr {($maxIntens + 3*$comp)/4}]
	}
    } else {
	#
	# Compute the dark color by cutting 45% from
	# each of the background color components.
	#
	foreach comp $rgb {
	    lappend darkRGB [expr {55*$comp/100}]
	}
    }
    set darkColor [eval format "#%0${len}x%0${len}x%0${len}x" $darkRGB]

    #
    # Compute the light shadow color
    #
    if {[string compare $::tk_patchLevel 8.3.1] >= 0 && $g > $maxIntens*0.95} {
	#
	# The background is already very bright: make the
	# light color a little darker than the background
	# by reducing each color component by 10%
	#
	foreach comp $rgb {
	    lappend lightRGB [expr {9*$comp/10}]
	}
    } else {
	#
	# Compute the light color by boosting each background
	# color component by 45% or half-way to white, whichever
	# is greater (the first approach works better for
	# unsaturated colors, the second for saturated ones)
	#
	foreach comp $rgb {
	    set comp1 [expr {145*$comp/100}]
	    if {$comp1 > $maxIntens} {
		set comp1 $maxIntens
	    }
	    set comp2 [expr {($maxIntens + $comp)/2}]
	    lappend lightRGB [expr {($comp1 > $comp2) ? $comp1 : $comp2}]
	}
    }
    set lightColor [eval format "#%0${len}x%0${len}x%0${len}x" $lightRGB]
}

#------------------------------------------------------------------------------
# tablelist::configLabel
#
# This procedure is invoked to configure the label widget w according to the
# options and their values given in args.  It is needed for label widgets with
# children, managed by the place geometry manager, because - strangely enough -
# by just configuring the label causes its children to become invisible on
# Windows (but not on UNIX).  The procedure solves this problem by using a
# trick: after configuring the label, it applies a constant configuration
# value to its children, which makes them visible again.
#------------------------------------------------------------------------------
proc tablelist::configLabel {w args} {
    eval [list $w configure] $args

    foreach c [winfo children $w] {
	$c configure -borderwidth 0
    }
}

#------------------------------------------------------------------------------
# tablelist::strRange
#
# Returns the largest initial (for alignment = left or center) or final (for
# alignment = right) range of characters from str whose width, when displayed
# in the given font, is no greater than pixels.
#------------------------------------------------------------------------------
proc tablelist::strRange {win str font pixels alignment} {
    if {[font measure $font -displayof $win $str] <= $pixels} {
	return $str
    }

    set halfLen [expr {[string length $str] / 2}]
    if {$halfLen == 0} {
	return ""
    }

    if {[string compare $alignment right] == 0} {
	set rightStr [string range $str $halfLen end]
	set width [font measure $font -displayof $win $rightStr]
	if {$width == $pixels} {
	    return $rightStr
	} elseif {$width > $pixels} {
	    return [strRange $win $rightStr $font $pixels $alignment]
	} else {
	    set str [string range $str 0 [expr {$halfLen - 1}]]
	    return [strRange $win $str $font \
		    [expr {$pixels - $width}] $alignment]$rightStr
	}
    } else {
	set leftStr [string range $str 0 [expr {$halfLen - 1}]]
	set width [font measure $font -displayof $win $leftStr]
	if {$width == $pixels} {
	    return $leftStr
	} elseif {$width > $pixels} {
	    return [strRange $win $leftStr $font $pixels $alignment]
	} else {
	    set str [string range $str $halfLen end]
	    return $leftStr[strRange $win $str $font \
			    [expr {$pixels - $width}] $alignment]
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::strRangeExt
#
# Invokes strRange with the given arguments and returns a string with up to
# three trailing (for alignment = left or center) or leading (for alignment =
# right) dots built from its result.
#------------------------------------------------------------------------------
proc tablelist::strRangeExt {win str font pixels alignment} {
    set subStr [strRange $win $str $font $pixels $alignment]
    set len [string length $subStr]
    if {$pixels < 0 || $len == [string length $str]} {
	return $subStr
    }

    if {[string compare $alignment right] == 0} {
	set extSubStr ...$subStr
	while {[font measure $font -displayof $win $extSubStr] > $pixels} {
	    if {$len > 0} {
		set subStr [string range $subStr 1 end]
		incr len -1
		set extSubStr ...$subStr
	    } else {
		set extSubStr [string range $extSubStr 1 end]
	    }
	}
    } else {
	set last [expr {$len - 1}]
	set extSubStr $subStr...
	while {[font measure $font -displayof $win $extSubStr] > $pixels} {
	    if {$last >= 0} {
		incr last -1
		set subStr [string range $subStr 0 $last]
		set extSubStr $subStr...
	    } else {
		set extSubStr [string range $extSubStr 1 end]
	    }
	}
    }

    return $extSubStr
}

#------------------------------------------------------------------------------
# tablelist::adjustItem
#
# Returns the list obtained by adjusting the list specified by item to the
# length expLen after replacing all \t and \n characters with spaces.
#------------------------------------------------------------------------------
proc tablelist::adjustItem {item expLen} {
    if {[string first \t $item] >= 0 || [string first \n $item] >= 0} {
	regsub -all "\t|\n" $item " " item
    }

    set len [llength $item]
    if {$len == $expLen} {
	return $item
    } elseif {$len < $expLen} {
	for {set n $len} {$n < $expLen} {incr n} {
	    lappend item ""
	}
	return $item
    } else {
	return [lrange $item 0 [expr {$expLen - 1}]]
    }
}

#------------------------------------------------------------------------------
# tablelist::adjustElem
#
# Prepares the text specified by $textName and the image width specified by
# $imageWidthName for insertion into a cell of the tablelist widget win.
#------------------------------------------------------------------------------
proc tablelist::adjustElem {win textName imageWidthName font pixels alignment} {
    upvar $textName text $imageWidthName imageWidth

    if {$pixels == 0} {				;# convention: dynamic width
	if {$imageWidth != 0 && [string compare $text ""] != 0} {
	    if {[string compare $alignment right] == 0} {
		set text "$text "
	    } else {
		set text " $text"
	    }
	}
    } elseif {$imageWidth == 0} {		;# no image
	set text [strRangeExt $win $text $font $pixels $alignment]
    } elseif {[string compare $text ""] == 0} {	;# image w/o text
	if {$imageWidth > $pixels} {
	    set imageWidth 0			;# can't display the image
	}
    } else {					;# both image and text
	set gap [font measure $font -displayof $win " "]
	if {$imageWidth + $gap <= $pixels} {
	    incr pixels -[expr {$imageWidth + $gap}]
	    set text [strRangeExt $win $text $font $pixels $alignment]
	    if {[string compare $alignment right] == 0} {
		set text "$text "
	    } else {
		set text " $text"
	    }
	} elseif {$imageWidth <= $pixels} {
	    set text ""				;# can't display the text
	} else {
	    set imageWidth 0			;# can't display the image
	    set text ""				;# can't display the text
	}
    }
}

#------------------------------------------------------------------------------
# tablelist::insertElem
#
# Inserts the given text and image into the text widget w, just before the
# character position specified by index.  The image will follow the text if
# alignment is "right", and will precede it otherwise.
#------------------------------------------------------------------------------
proc tablelist::insertElem {w index text image imageWidth alignment} {
    set index [$w index $index]

    if {$imageWidth == 0} {
	$w insert $index $text
    } elseif {[string compare $alignment right] == 0} {
	$w image create $index -image $image
	$w insert $index $text
    } else {
	$w insert $index $text
	$w image create $index -image $image
    }
}

#------------------------------------------------------------------------------
# tablelist::doFormat
#
# Invokes the command fmtCmd with val as parameter and returns the result after
# replacing all \t and \n characters with spaces.
#------------------------------------------------------------------------------
proc tablelist::doFormat {fmtCmd val} {
    set str [uplevel #0 $fmtCmd [list $val]]
    if {[string first \t $str] >= 0 || [string first \n $str] >= 0} {
	regsub -all "\t|\n" $str " " str
    }

    return $str
}
