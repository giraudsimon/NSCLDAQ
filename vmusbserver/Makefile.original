TCLVERSION=8.5


CLIENTOBJS=CVMUSBRemote.o CVMUSBReadoutList.o

SERVEROBJS=CDuplicateSingleton.o \
CInvalidArgumentException.o \
CNoSuchObjectException.o \
CSocket.o \
CTCPBadSocketState.o \
CTCPConnectionFailed.o \
CTCPConnectionLost.o \
CTCPNoSuchHost.o \
CTCPNoSuchService.o \
CVMUSB.o \
CVMUSBModule.o \
CVMUSBReadoutList.o \
ErrnoException.o \
Exception.o \
MonitorException.o \
RangeError.o \
StateException.o \
StreamIOError.o \
URIFormatException.o \
CTCLChannelCommander.o \
CTCLServer.o \
CTCLStdioCommander.o \
CTCLTcpServerInstance.o \
TCLChannel.o \
TCLCommandPackage.o \
TCLCompatibilityProcessor.o \
TCLException.o \
TCLInterpreter.o \
TCLInterpreterObject.o \
TCLList.o \
TCLObject.o \
TCLObjectPackage.o \
TCLObjectProcessor.o \
TCLPackagedObjectProcessor.o \
TCLProcessor.o \
TCLResult.o \
TCLString.o \
TCLTimer.o \
TCLTracedVariable.o \
TCLVariable.o \
CImmediateListCommand.o \
CVMUSBPackage.o


CXXFLAGS=-I. -I/usr/include/tcl$(TCLVERSION) -g

COMMON_LDFLAGS= -lusb -g
SERVER_LDFLAGS=
CLIENT_LDFLAGS=

all: libvmusb.so libvmusbclient.so pkgIndex.tcl


libvmusb.so: $(SERVEROBJS)
	$(CXX) -shared -o libvmusb.so $(SERVEROBJS) $(COMMON_LDFLAGS) $(SERVER_LDFLAGS)

pkgIndex.tcl: libvmusb.so
	echo pkg_mkIndex -verbose . "*.so" | tclsh$(TCLVERSION)

libvmusbclient.so: $(CLIENTOBJS)
	$(CXX) -shared -o libvmusbclient.so $(CLIENTOBJS) $(COMMON_LDFLAGS) $(SERVER_LDFLAGS)

clean:
	rm -f *.o *.so
