dnl
dnl - AX_EPICS
dnl    Test for the EPICS control system.
dnl    Provides:
dnl     --with-epics-rootdir - to request a specific epics installation directory tree
dnl                            this overrides any automated tests.
dnl     AC_SUBST's the following in Makefiles:
dnl     EPICS_INCLUDES - Compilation flags to get the headers #included.
dnl     EPICS_LDFLAGS  - Link time flags to incorporate the libraries and a linker -
dnl                      rpath so they actually load
dnl     EPICS_BIN      - Path to the EPICS runables. Among other things, this can be used
dnl                      to locate/run caRepeater
dnl    
dnl   No attempt is made to determine an appropriate set of env variables as I don't think
dnl   that's even possible

AC_DEFUN([AX_EPICS], [
epicstestdirs="/usr/local/epics /opt/epics /usr/lib/epics"
haveepics="no"


AC_MSG_CHECKING([ for epics includes, libs and binaries])

#  Check for --with-epics-rootdir....

AC_ARG_WITH(epics-rootdir,
	[  --with-epics-rootdir=top level directory of EPICS install (requires --enable-epics-tools)],
	[haveepics="yes"
	 epicsroot="$withval"],
	[haveepics="no"])

#
#  Figure out which architecture specific directory to use.
#
#   For now we're just going to support various types of linux:
#   linux-x86_64 (64 bit linux) and linxu-x86 (32 bit linux).
#   Later this can be expanded to other os's and archs.
#
bits=`arch`
kernel=`uname |tr [A-Z] [a-z]`

case $bits in
   i686 | i386)
   	bits=x86
   ;;
esac

epicsarch=$kernel-$bits


if test $haveepics == "no"
then
   for dir in $epicstestdirs
   do
      if test $haveepics == "no"
      then
        AC_CHECK_FILE([${dir}/include/cadef.h],
	              [haveepics="yes"
                       epicsroot=$dir])
      fi
   done
fi

#  If we found epics we can build the software that needs it.
#  otherwise it's optional.

if test $haveepics == "yes"
then
	# Have epics:
        # Note for now assume gcc is the compiler

   EPICS_INCLUDES="-I$epicsroot/include -I$epicsroot/include/os/Linux -I$epicsroot/include/compiler/gcc"
   EPICS_LDFLAGS="-L$epicsroot/lib/$epicsarch -lca -Wl,\"-rpath=$epicsroot/lib/$epicsarch\""
   EPICS_BIN="-DEPICS_BIN=\"\\\"$epicsroot/bin/$epicsarch\\\"\""


   AC_SUBST(EPICS_INCLUDES)
   AC_SUBST(EPICS_LDFLAGS)
   AC_SUBST(EPICS_BIN)

else
   AC_MSG_ERROR([ Cannot locate the EPICS root directory, help me out with --with-epics-rootdir])
fi

])