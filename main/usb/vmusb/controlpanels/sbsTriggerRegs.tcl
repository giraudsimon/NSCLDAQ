#
#  Manage trigger registers (insulate from interface)
#
package require Vme

proc setBaseAddress base {
    catch {vme delete trigger_unit}
    vme create trigger_unit -device extended  $base 0x2000
}


proc setShortGate value {
  trigger_unit set -l 0x1010 $value
}
proc getShortGate {} {
    return [trigger_unit get -l 0x1010]
}

proc setLongGate value {
    trigger_unit set -l 0x1014 $value
}
proc getLongGate {} {
    return [trigger_unit get -l 0x1014]
}


proc setMask value {
    trigger_unit set -l 0x1018 $value
}
proc getMask {} {
    trigger_unit get -l 0x1018
}

proc getGates {} {
    return [trigger_unit get -l 0x101c]
}

proc setG0 which {
    set which [expr $which & 0xf]
    set current [trigger_unit get -l 0x101c]
    set current [expr ($current & 0xfffffff0) | $which]
    trigger_unit set -l 0x101c $current
}
proc setG1 which {
    set which [expr $which & 0xf]
    set current [trigger_unit get -l 0x101c]
    set current [expr ($current & 0xffffff0f) | ($which << 4)]
    trigger_unit set -l 0x101c $current
}

proc ttl {} {
    set current [trigger_unit get -l 0x101c]
    set current [expr ($current & 0x000000ff) | (0x100)]
    trigger_unit set -l 0x101c $current
}
proc nim {} {
    set current [trigger_unit get -l 0x101c]
    set current [expr ($current & 0x000000ff)]
    trigger_unit set -l 0x101c $current

}