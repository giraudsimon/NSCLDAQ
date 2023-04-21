SUBDIRS=configuration xiatest booter format ddasdumper nscope readout sorter readoutdriver pixieplugin simple_setups qtscope

ifeq(@XIAAPI_VERSION@, 2)
SUBDIRS += dumpsetfile xml/tinyxml xml tclconfig

# readout-extclock - external clock handling is subsumed into readout.
# traiter - legacy trace iterator class. Not installed for now.
