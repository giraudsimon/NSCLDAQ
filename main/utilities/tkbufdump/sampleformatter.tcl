

#
#  Here's a sample packet formatter that formats a body
#  that consists of a set of CAEN32 digitizers.  It regiseters
#  itself to format 0x792 packets.
#

# Extraction functions.. these should probably be hidden in a namespace.
#

# Geo graphical address field

proc caenGeo long {
    return [expr {$long >> 27}]
}
# type field.
proc caenType long {
    return [expr {($long >> 24) & 7}]
}

# crate field from header:

proc caenCrate long {
    return [expr {($long >> 16) & 0xff}]
}

#  Count field from header.

proc caenCount long {
    return [expr {($long >> 8) & 0x3f}]
}

# Channel number from data:

proc caenChan long {
    return [expr {($long >> 16) & 0x3f}]
}
# Valid field from data

proc caenValid long {
    return [expr {($long & 0x4000) != 0}]
}
# Underflow field from data
proc caenUnderflow long {
    return [expr {($long & 0x2000) != 0}]
    
}
# Overfloew bit from data:
proc caenOverflow long {
    return [expr {($long & 0x1000) != 0}]
}

# Value from data:

proc caenValue long {
    return [expr {($long & 0xfff)}]
}

# Event counter from trailer:
proc caenEventNum long {
    return [expr {$long & 0x3fffff}]
}
#
#  The formatter.
#
proc format0792 body {
    
    # The module is made up of longwords described in 4.5 of the CAEN 792 manual.
    # Assume these are big endian as they come in the buffer
    
    set result [list]
    while {[llength $body] > 0} {
        set high [lindex $body 0]
        set low  [lindex $body 1]
        set value [expr $high << 16 | $low]
        set body [lrange $body 2 end]
        
        # Decode value
        
        set type [caenType $value]
        append result [format "0x%08x : " $value]
        switch -- $type {
            2 {
                #  Header
                
                set geo     [caenGeo   $value]
                set crate   [caenCrate $value]
                set count   [caenCount $value]
                
                append result "Header for Crate $crate Geo $geo with $count channels\n"
            }
            0 {
                # datum
                
                set channel [caenChan $value]
                set valid   [caenValid $value]
                set under   [caenUnderflow $value]
                set over    [caenOverflow  $value]
                set value   [caenValue     $value]
                
                append result "Conversion for channle $channel : $value "
                if {$valid} {
                    append result "Valid set "
                }
                if {$under} {
                    append result "Underflow set "
                }
                if {$over} {
                    append result "Overflow set"
                }
                append result "\n"
            }
            4 {
                # Trailer.
                
                append result "Trailer for event number [caenEventNum $value]\n"
            }
            6 {
                # Invalid
                
                append result "Data not valid\n"
            }
        }
    }
    return $result
}
#
#  Register the unpacker.
#

registerPacketFormatter 0x792 format0792