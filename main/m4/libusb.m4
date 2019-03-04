dnl
dnl - AX_LIBUSB
dnl
dnl   libusb can be in several places including architecture specific directories.
dnl   This test determines if libusb and its headers exist or not and
dnl   AC_SUBSTs:
dnl   USBSWITCHES  - compilations switches
dnl   USB_LIBS     - link time switches.
dnl  
dnl   Provides assists via:
dnl     --with-usb-headerdir  - path to usb.h
dnl     --with-usb-libdir     - path to libusb.a

AC_DEFUN([AX_LIBUSB],[
	headerdirs="/usr/include /usr/local/include"
	libdirs="/lib /usr/lib /usr/local/lib /usr/lib/${host_cpu}-${host_os}"

	usbheaderdir=""
	usblibdir=""

	AC_MSG_CHECKING([for libusb headers])
	AC_ARG_WITH(usb-headerdir,
		[   --with-usb-headerdir=Path to usb.h],
		[usbheaderdir=$withval],[])

	if test "$usbheaderdir" == ""
	then
	   	for dir in $headerdirs
   		do
			if test "$usbheaderdir" == ""
      			then
				AC_CHECK_FILE([$dir/usb.h],
					[usbheaderdir=$dir])
			fi
   		done
	fi

	if test "$usbheaderdir" != ""
	then
		USBSWITCHES="-I${usbheaderdir}"
		AC_MSG_RESULT([found in $usbheaderdir])
	else
		AC_MSG_ERROR([Can't find usb.h install libusb development or try using --with-usb-header-dir to help me find it])
	fi

	AC_MSG_CHECKING([for libusb library]) 
	AC_ARG_WITH(usb-libdir,
		[   --with-usb-libdir=Path to libusb.a],
		[usblibdir=$withval],[])

	if test "$usblibdir" == ""
	then	
   		for dir in $libdirs
   		do	
      			if test "$usblibdir" == ""
      			then
				AC_CHECK_FILE([$dir/libusb.a],
					[usblibdir=$dir])
      			fi
   		done
	fi

	if test "$usblibdir" != "" 
	then
		USB_LIBS="-L${usblibdir} -lusb"
  		 AC_MSG_RESULT([found in $usblibdir])
	else
		AC_MSG_ERROR([Can't find libusb.a install libusb or try using --with-usb-libdir to tell me where to find it])
	fi 
	#  If we got this far the symbos can be exported to the makefiles:

	AC_SUBST(USBSWITCHES)
	AC_SUBST(USB_LIBS)
])