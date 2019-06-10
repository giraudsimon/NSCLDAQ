#===================================================================
# class XXUSB
#===================================================================
package provide xxusb 1.0

package require Itcl

itcl::class XXUSB {
	protected variable event
	protected variable scaler
	protected variable self
	
	constructor {xxusb} {
		set event ""
		set scaler ""
#		set self [string trimleft $this :]
		set self $xxusb
	}
	
	destructor {}
	
	public method GetVariable {v} {set $v}
	public method AddToStack {stack command}
#	public method FinishStack {stack}
	public method ClearStack {stack} {set $stack ""}
#	public method LoadStack {stack}
#	public method LoadStacks {stacks}
	public method StopDAQ {}
	public method StartDAQ {}
	public method USBTrigger {}
	public method ScalerDump {}
#	public method Flash {}
}

itcl::body XXUSB::AddToStack {stack command} {
	foreach c $command {
		lappend $stack $c
	}
}

#itcl::body XXUSB::FinishStack {stack} {
## First element of stack is total length
## Add a 0 if VM-USB to match 32 bit structure
#	if {[string first VM $self] == 0} {
#		set pile [expr [llength $stack]+1]
#		lappend pile 0
#	} else {
#		set pile [llength $stack]
#	}
#	foreach s $stack {lappend pile [expr $s]}
#	return $pile
#}

#itcl::body XXUSB::LoadStack {stack} {
## First element of stack is total length
## Add a 0 if VM-USB to match 32 bit structure
#	if {[string first VM $self] == 0} {
#		set pile [expr [llength [set $stack]]+1]
#		lappend pile 0
#	} else {
#		set pile [llength [set $stack]]
#	}
#	foreach s [set $stack] {lappend pile [expr $s]}
#	if {[string equal $stack event]} {set s 2}
#	if {[string equal $stack scaler]} {set s 3}
#	if {![info exist s]} {
#		tk_messageBox -icon error -message "Error while loading stack in $self\n\
#		unknown stack: $stack ; must be either event or scaler"
#		return
#	}
#	set ret [::XXUSBWriteStack $self $s $pile]
#	set check [::XXUSBReadStack $self $s]
#	for {set i 0} {$i < [llength $pile]} {incr i} {
#		if {![expr [lindex $pile $i] == [lindex $check $i]]} {
#			tk_messageBox -icon error -message "Error loading $stack stack in $self!"
#			return
#		}
#	}
#}

# This procedure loads all stacks specified in the list "stacks" one after another
# This is required in particular in the VMUSB otherwise stacks gets overwritten
#itcl::body XXUSB::LoadStacks {stacks} {
#	set index 0
#	foreach stack $stacks {
## First element of stack is total length
## Add the starting address if VM-USB for which stacks can be contiguous
#		if {[string first VM $self] == 0} {
#			set pile [expr [llength [set $stack]]+1]
#			lappend pile $index
#		} else {
#			set pile [llength [set $stack]]
#		}
#		foreach s [set $stack] {lappend pile [expr $s]}
#		if {[string equal $stack event]} {set s 2}
#		if {[string equal $stack scaler]} {set s 3}
#		if {[string equal $stack id2]} {set s 18}
#		if {[string equal $stack id3]} {set s 19}
#		if {[string equal $stack id4]} {set s 34}
#		if {[string equal $stack id5]} {set s 35}
#		if {[string equal $stack id6]} {set s 50}
#		if {[string equal $stack id7]} {set s 51}
#		if {![info exist s]} {
#			tk_messageBox -icon error -message "Error while loading stack in $self\n\
#			unknown stack: $stack ; must be event, scaler or id2 to id7"
#			return
#		}
#		incr index [llength $pile]
#		set ret [::XXUSBWriteStack $self $s $pile]
#	}
#}

itcl::body XXUSB::StopDAQ {} {
	$self writeActionRegister 0
}

itcl::body XXUSB::StartDAQ {} {
	$self writeActionRegister [expr 0x1]
}

itcl::body XXUSB::USBTrigger {} {
	$self writeActionRegister [expr 0x2]
}

itcl::body XXUSB::ScalerDump {} {
	$self writeActionRegister [expr 0x10]
}

#itcl::body XXUSB::Flash {} {
#	set filename [tk_getOpenFile -title "Select xxusb firmware file" -filetypes {{"bit firmware file" {.bit}}}]
#	if {[string length $filename] == 0} {return}
#	set file [open $filename r]
#	fconfigure $file -translation binary
#	set bin [read $file]
#	close $file
#	set size [file size $filename]
#	binary scan $bin cu$size bytes
#	if {[string first VM $self] == 0} {set blocks 830} else {set blocks 512}
#	for {set i 0} {$i < $size} {incr i} {lappend lbytes [lindex $bytes $i]}
#	XXUSBFlashProgram $self $lbytes $blocks
#}
