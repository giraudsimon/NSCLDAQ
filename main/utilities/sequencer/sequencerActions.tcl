package require ReadoutGUIPanel

proc epicsSet {name value} {
    puts "Setting $name -> $value"
    return 0
}
proc epicsAccess name {
    puts "Setting up access to $name"
}
proc setTitle {name title} {
    ReadoutGUIPanel::setTitle $title
    return 0
}

puts "Actions sourced in"