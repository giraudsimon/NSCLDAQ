#
#  To override the default service name add a line like:
#
#   set ServiceName MyReadoutRESTService
package require ReadoutREST

##
#  This little bit of iffery is needed when DDASReadout
#  is run under control of the ddasReadout script.
#  it thinks that if the pipe to the readout program
#  closes the program exits and does nasty things in
#  response.
#
#  When running ddasreadout with the REST interface,
# define the  RDOREST_KEEPSTDIN environment variable
# to be any old thing to prevent stdin being closed.
#
#
if {[array names ::env RDOREST_KEEPSTDIN] eq ""} {
    close stdin
}

