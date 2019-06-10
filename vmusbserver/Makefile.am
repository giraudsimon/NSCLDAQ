lib_LTLIBRARIES=libvmusbServer.la libvmusbclient.la 


noinst_HEADERS = daqdatatypes.h TCLVersionHacks.h CDuplicateSingleton.h \
VariableTraceCallback.h \
CInvalidArgumentException.h \
CNoSuchObjectException.h \
CSocket.h \
CTCPBadSocketState.h \
CTCPConnectionFailed.h \
CTCPConnectionLost.h \
CTCPNoSuchHost.h \
CTCPNoSuchService.h \
CVMUSB.h \
CVMUSBModule.h \
CVMUSBReadoutList.h \
ErrnoException.h \
Exception.h \
MonitorException.h \
RangeError.h \
StateException.h \
StreamIOError.h \
URIFormatException.h \
CTCLChannelCommander.h \
CTCLServer.h \
CTCLStdioCommander.h \
CTCLTcpServerInstance.h \
TCLChannel.h \
TCLCommandPackage.h \
TCLCompatibilityProcessor.h \
TCLException.h \
TCLInterpreter.h \
TCLInterpreterObject.h \
TCLList.h \
TCLObject.h \
TCLObjectPackage.h \
TCLObjectProcessor.h \
TCLPackagedObjectProcessor.h \
TCLProcessor.h \
TCLResult.h \
TCLString.h \
TCLTimer.h \
TCLTracedVariable.h \
TCLVariable.h \
CImmediateListCommand.h


INCLUDES = @TCL_FLAGS@ @USBSWITCHES@

libvmusbServer_la_LIBADD = @TCL_LDFLAGS@ @USB_LIBS@

libvmusbServer_la_SOURCES = CDuplicateSingleton.cpp \
CInvalidArgumentException.cpp \
CNoSuchObjectException.cpp \
CSocket.cpp \
CTCPBadSocketState.cpp \
CTCPConnectionFailed.cpp \
CTCPConnectionLost.cpp \
CTCPNoSuchHost.cpp \
CTCPNoSuchService.cpp \
CVMUSB.cpp \
CVMUSBModule.cpp \
CVMUSBReadoutList.cpp \
ErrnoException.cpp \
Exception.cpp \
MonitorException.cpp \
RangeError.cpp \
StateException.cpp \
StreamIOError.cpp \
URIFormatException.cpp \
CTCLChannelCommander.cpp \
CTCLServer.cpp \
CTCLStdioCommander.cpp \
CTCLTcpServerInstance.cpp \
TCLChannel.cpp \
TCLCommandPackage.cpp \
TCLCompatibilityProcessor.cpp \
TCLException.cpp \
TCLInterpreter.cpp \
TCLInterpreterObject.cpp \
TCLList.cpp \
TCLObject.cpp \
TCLObjectPackage.cpp \
TCLObjectProcessor.cpp \
TCLPackagedObjectProcessor.cpp \
TCLProcessor.cpp \
TCLResult.cpp \
TCLString.cpp \
TCLTimer.cpp \
TCLTracedVariable.cpp \
TCLVariable.cpp \
CImmediateListCommand.cpp \
CVMUSBPackage.cpp



libvmusbclient_la_SOURCES = CVMUSBRemote.cpp \
	CVMUSBReadoutList.cpp CSocket.cpp

include_HEADERS = CVMUSBRemote.h \
	CVMUSBReadoutList.h

install-exec-local:
	$(mkinstalldirs) $(bindir) $(libdir)
	$(INSTALL_PROGRAM) server.tcl $(bindir)/server
	$(INSTALL_DATA)    pkgIndex.tcl $(libdir)

EXTRA_DIST = server.tcl pkgIndex.tcl
