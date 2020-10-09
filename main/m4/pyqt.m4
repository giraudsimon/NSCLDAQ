dnl
dnl - AX_PYQT
dnl
dnl  Tests for the existence of Pyqt.
dnl  This test depends on a successful completion of AM_PATH_PYTHON to establish
dnl  the PYTHON variable.
dnl
AC_DEFUN([AX_PYQT],[
   AC_MSG_CHECKING([for pyqt5])
   $PYTHON -c "from PyQt5 import *" >/dev/null 2>/dev/null
   if test "$?" = 0
   then
	AC_MSG_RESULT([found])
   else
        AC_MSG_ERROR([NSCLDAQ requires the PyQt5 bindings to Qt5])
   fi


])
