AC_DEFUN([AX_TCL], [
dnl shamelessly stolen from http://www.opensource.apple.com/source/passwordserver_sasl-164/cyrus_sasl/cmulocal/tcl.m4
# --- BEGIN CMU_TCL ---
dnl To link against Tcl, configure does several things to make my life
dnl "easier".
dnl
dnl * maybe ask the user where they think Tcl lives, and try to find it
dnl * maybe ask the user what "tclsh" is called this week (i.e., "tclsh8.0")
dnl * run tclsh, ask it for a path, then run that path through sed
dnl * sanity check its result (many installs are a little broken)
dnl * try to figure out where Tcl is based on this result
dnl * try to guess where the Tcl include files are
dnl
dnl Notes from previous incarnations:
dnl > XXX MUST CHECK FOR TCL BEFORE KERBEROS V4 XXX
dnl > This is because some genius at MIT named one of the Kerberos v4
dnl > library functions log().  This of course conflicts with the
dnl > logarithm function in the standard math library, used by Tcl.
dnl
dnl > Checking for Tcl first puts -lm before -lkrb on the library list.
dnl

dnl Check for some information from the user on what the world looks like

dnl TCL locational arguments:


AC_ARG_WITH(tclconfig, 
            AS_HELP_STRING([--with-tclconfig=PATH],[Use tcl defined by provided tclConfig.sh]),
          	TCLCONFIG="${withval}")

AC_ARG_WITH(tkconfig, 
            AS_HELP_STRING([--with-tkconfig=PATH],[Use tk defined by provided tkConfig.sh]),
          	TKCONFIG="${withval}")


dnl if tclconfig not defined
if test -z "${TCLCONFIG}"; then

  dnl user did not provide us information about where tcl lives... try to find it
  if test -z "$TCLSH"; then
    dnl Try and find tclsh.  Any tclsh.
    dnl If a new version of tcl comes out and unfortunately adds another
    dnl filename, it should be safe to add it (to the front of the line --
    dnl somef vendors have older, badly installed tclshs that we want to avoid
    dnl if we can)
    AC_PATH_PROGS(TCLSH, [tclsh8.6 tclsh8.5 tclsh8.4 tclsh8.3 tclsh8.2 tclsh8.1 tclsh8.0 tclsh], "unknown")
  fi

  dnl Did we succeed in finding tclsh?
  if test "${TCLSH}" != "unknown"; then
    dnl we succeeded...
    AC_MSG_CHECKING([where Tcl says it lives])
    dnl
    dnl See if Tcl can tell us where it is:
    dnl
    tclClaims=`echo puts \\\$tcl_library | ${TCLSH} | sed -e 's,[^/]*$,,'`
    AC_MSG_CHECKING([$tclClaims])
    if test -f $tclClaims/tclConfig.sh; then
      TclLibBase=${tclClaims}
    else
      dnl If not try /usr/lib/tcl$version where some (debian e.g.) put it:

      version=`echo puts \\\$tcl_version | ${TCLSH}`
      libloc=/usr/lib/tcl${version}
      AC_MSG_CHECKING($libloc)
      if test -f $libloc/tclConfig.sh; then
         TclLibBase=$libloc
      else 
         AC_MSG_ERROR([Can't find tclConfig.sh you'll need to use --with-tclconfig to tell me where it is])
      fi
    fi
    AC_MSG_RESULT($TclLibBase)
  fi

  dnl did we find a candidate location for tclConfig.sh? 
  if test -z "$TclLibBase" ; then
    dnl no we didn't
    AC_MSG_RESULT([can't find tclsh])
    AC_MSG_ERROR([Can't find Tcl installation.])
  else
    dnl yes we found a candidate to look for tclConfig.sh
    dnl 
    AC_MSG_CHECKING([for tclConfig.sh])
    dnl Check a list of places where the tclConfig.sh file might be.
    for tcldir in "${TclLibBase}" \
                  "${TclLibBase}/.." \
                "${TclLibBase}"`echo ${TCLSH} | sed s/sh//` ; do
      if test -f "${tcldir}/tclConfig.sh"; then
        TclLibBase="${tcldir}"
        break
      fi
    done

    if test -z "${TclLibBase}" ; then
      dnl failed to find tclConfig.sh in TclLibBase
      AC_MSG_RESULT("unknown")
      AC_MSG_ERROR([can't find Tcl configuration; use of Tcl disabled.])
    else
      AC_MSG_RESULT(${TclLibBase}/)
    fi

    dnl if we got here then we found tclConfig.sh
    TCLCONFIG=${TclLibBase}/tclConfig.sh
  fi 
else 
  dnl define TclLibBase for tkConfig.sh search
  TclLibBase=$( echo ${TCLCONFIG} | sed -e 's:\(/.*\)/.*:\1:' )
fi

dnl if tclconfig defined
if test \! -z "${TCLCONFIG}"; then

  AC_MSG_CHECKING([Tcl configuration on what Tcl needs to compile])
  dnl check that it is executable
  if test -r ${TCLCONFIG} ; then
    AC_MSG_RESULT(ok)
  else
    AC_MSG_ERROR([tclConfig.sh does not refer to an existing file])
  fi

  dnl execute that beast
  . ${TCLCONFIG}

  dnl is TCL_EXEC_PREFIX found?
  if test -z ${TCL_EXEC_PREFIX} ; then
    AC_MSG_ERROR([tclConfig.sh did not set TCL_EXEC_PREFIX which is required to find tclsh])
  fi

  if test -z ${TCL_VERSION} ; then
    AC_MSG_ERROR([tclConfig.sh did not set TCL_VERSION which is required to find tclsh])
  fi

  dnl figure out where tclsh is based on what tclConfig.sh told us... form a 
  TCL_BIN=${TCL_EXEC_PREFIX}/bin
  TCLSH_TRIAL=tclsh${TCL_VERSION}

  AC_MSG_CHECKING([tcl version])
  AC_MSG_RESULT([${TCL_VERSION}])

  dnl check for tclsh in TCL_BIN
  AC_PATH_PROG(TCLSH, ${TCLSH_TRIAL}, [unknown], ${TCL_BIN})
  if test "${TCLSH}" = "unknown" ; then
    AC_MSG_ERROR("Unable to find ${TCLSH_TRIAL} in ${TCL_BIN}")
  else 
    AC_MSG_RESULT([${TCLSH}])
  fi

  dnl Now, hunt for the Tcl include files, since we don't strictly
  dnl know where they are; some folks put them (properly) in the 
  dnl default include path, or maybe in /usr/local; the *BSD folks
  dnl put them in other places.
  AC_MSG_CHECKING([where Tcl includes are])
  for tclinclude in "${TCL_PREFIX}/include/tcl${TCL_VERSION}" \
                    "${TCL_PREFIX}/include/tcl" \
                    "${TCL_PREFIX}/include" ; do
    if test -r "${tclinclude}/tcl.h" ; then
      TCL_CPPFLAGS="-I${tclinclude}"
      break
    fi
  done
  if test -z "${TCL_CPPFLAGS}" ; then
    AC_MSG_WARN(can't find Tcl includes; use of Tcl disabled.)
    with_tcl=no
  fi
  AC_MSG_RESULT(${TCL_CPPFLAGS})
  
  dnl Finally, pick up the Tcl configuration if we haven't found an
  dnl excuse not to.
  TCL_LIBS="${TCL_LD_SEARCH_FLAGS} ${TCL_LIB_SPEC} ${TCL_LIBS}"

  dnl Newer tclConfig.sh may depend on LIB_RUNTIME_DIR. Set it equal
  dnl to the directory holding tclConfig.sh
  LIB_RUNTIME_DIR=$( echo ${TCLCONFIG} | sed -e 's:\(/.*\)/.*:\1:' )
fi


AC_SUBST(TCL_DEFS)
AC_SUBST(TCL_LIBS)
AC_SUBST(TCL_CPPFLAGS)
AC_SUBST(TCLSH)
AC_SUBST(LIB_RUNTIME_DIR)

dnl
dnl   TK/Wish  If WISH is not define find it:
dnl

version=`echo puts \\\$tcl_version | ${TCLSH}`

dnl We can't use wish to find tkConfig.sh since it requires an X11 server to run.
dnl we're going to assume that it's either ../tk or ../tk${version} relative to
dnl TclLibBase...unless of course we've been handed it by the --with switches.

TkLibBase=$( echo $TKCONFIG | sed -e 's:/\(.*\)/.*:\1:' )

if test -z ${TKCONFIG}; then   
  dnl no tkConfig.sh specified... see if we can find it.
  for guess in "${TclLibBase}/../tk${version}" \
               "${TclLibBase}/../tk" \
               "${TclLibBase}/../../tk${version}/lib" \
               "${TclLibBase}/../tk${version}/lib" ; do
    AC_MSG_CHECKING([for tkConfig.sh in ${guess}])
    TRIAL=${guess}/tkConfig.sh
    if test -f ${TRIAL} ; then
      if test -r ${TRIAL} ; then
        TkLibBase=${guess}
        TKCONFIG=${TRIAL}
 	      AC_MSG_RESULT([found])
        break
      else 
        AC_MSG_RESULT([not found])
      fi
    else
      AC_MSG_RESULT([not found])
    fi
  done
fi

dnl Still not defined is an error:

AC_MSG_CHECKING([for tkConfig.sh])
if test \! -z ${TKCONFIG} -a -r ${TKCONFIG} ; then
  dnl Source the config file
  . ${TKCONFIG}
  AC_MSG_RESULT([${TKCONFIG}])
else
  AC_MSG_RESULT([not found])
  AC_MSG_ERROR([Can't find tkConfig.sh, use one of the --with-tkconfig flags to help me out])
fi

dnl Figure out and set the syms:

TK_LIBS="${TK_LD_SEARCH_FLAGS} ${TK_LIB_SPEC} ${TK_LIBS}"
TK_CPPFLAGS="${TK_INCLUDE_SPEC}"

AC_SUBST(WISH)
AC_SUBST(TK_DEFS)
AC_SUBST(TK_LIBS)
AC_SUBST(TK_CPPFLAGS)

dnl --- END CMU_TCL ---
])dnl CMU_TCL
