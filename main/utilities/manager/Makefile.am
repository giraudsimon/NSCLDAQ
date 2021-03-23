program_scripts=startServer.tcl mg_mkconfig.tcl mg_cfgcontainers.tcl \
	mg_cfgprogram.tcl mg_stateedit.tcl mg_seqedit.tcl mg_loggeredit.tcl \
	mg_kvedit.tcl

library_scripts=mg_containerpackage.tcl mg_containeredit.tcl \
	mg_programpackage.tcl mg_programedit.tcl mg_sequencepackage.tcl  \
	mg_loggerpackage.tcl mg_kvstorepackage.tcl  mg_auth.tcl

test_scripts=container.test program.test sequence.test transitions.test \
	loggerdb.test kv.test

EXTRADIST=$(program_scripts) $(library_scripts) $(test_scripts) minimal.img singexists.sh

install-exec-local:
	for f in $(program_scripts); do $(INSTALL_PROGRAM) @srcdir@/$$f @bindir@/`basename $$f .tcl`; done
	$(mkinstalldirs) @prefix@/TclLibs @prefix@/TclLibs/manager
	for f in $(library_scripts); do $(INSTALL_DATA) @srcdir@/$$f @prefix@/TclLibs/manager; done
	echo pkg_mkIndex -verbose @prefix@/TclLibs/manager $(library_scripts) \
	| @TCLSH_CMD@ 


check:
	$(INSTALL_PROGRAM)  @srcdir@/singexists.sh ~
	if test "`ssh  localhost  ~/singexists.sh`" = "yes" ; \
	then \
		$(INSTALL_DATA) @srcdir@/minimal.img ~ ;\
		for script in $(test_scripts); do \
		DAQBIN=@bindir@ SRCDIR=@srcdir@ TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/$$script; \
		done; \
	else \
		echo No singularity manager tests will not be done; \
	fi

