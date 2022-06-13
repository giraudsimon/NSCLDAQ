proc lost {con} {
    $con destroy
    tk_messageBox -message {Lost connection with readout}
    exit
}

source client.tcl
source gdgwidget.tcl
source gdgcontrol.tcl

set con [controlClient %AUTO%]
set w   [gdgwidget .g -title {GDG 1 device}]

set full [gdgcontrol %AUTO% -widget $w -connection $con -name gdg1 -onlost lost]
$full UpdateValues

pack $w
