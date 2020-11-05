

PACKAGE_FILES = project.py state.py filemenu.py projectdisplay.py \
	ringspec.py programspec.py formdialog.py RecorderDialog.py

MAINS = experimentConfiguration.py

TESTS = projectHostsTest.py \
projectOpenTest.py \
projectProgramTest.py \
project.py \
projectRingTest.py \
run_all_tests.py \
schemaTest.py	\
eventLogTest.py	


FIGURES=initial.jpg hosts.jpg filledinhosts.jpg emptyprogramtable.jpg emptyringtable.jpg filledinprogramdef.jpg \
	filledinrings.jpg programdefdialog.jpg programlisting.jpg




INSTDIR = @prefix@/pythonLibs/nscldaq/expconfiguration

# Install hook.. create the directory  in which we'll install the modules/packages.
#                ensure there's an __init__.py there so that it's treated as a package.
#

install-exec-local:
	$(mkinstalldirs)  $(INSTDIR)
	touch $(INSTDIR)/__init__.py 
	for f in  $(PACKAGE_FILES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(INSTDIR); done
	for f in $(MAINS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@/`basename $$f .py`; done
	for f in $(FIGURES); do $(INSTALL_DATA) @srcdir@/$$f @datarootdir@/html; done

check-TESTS:
	(cd @srcdir@; PYTHONPATH=$(INSTDIR) @PYTHON@ run_all_tests.py)

EXTRA_DIST = $(PACKAGE_FILES) $(MAINS) $(TESTS) $(FIGURES) expConfig.xml




check: check-TESTS

