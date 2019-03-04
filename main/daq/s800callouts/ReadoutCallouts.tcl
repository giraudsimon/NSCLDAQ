source s800rctl.tcl
source s800rdocallouts.tcl


s800::Initialize [DAQParameters::getSourceHost]


proc OnBegin {run} {
    ::s800::OnBegin
}


proc OnEnd {run} {
    ::s800::OnEnd
}
