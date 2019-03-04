PACKAGE_FILES=cpanelWidget.py

MAINS = cpanel.py

FIGURES = cpanel-active.jpg cpanel-notready.jpg cpanel-ready.jpg

INSTDIR=@prefix@/pythonLibs/nscldaq/cpanel

##
# installation hook: create the installation directory tree.
#                    install the packges in  INSTDIR
#                    ensure there's an __init__.py which is required for
#                    package directories.
#

install-exec-local:
	$(mkinstalldirs)  $(INSTDIR)
	$(mkinstalldirs)  @datarootdir@/html
	touch $(INSTDIR)/__init__.py 
	for f in $(MAINS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@/`basename $$f .py`; done
	for f in  $(PACKAGE_FILES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(INSTDIR); done
	for f in $(FIGURES); do $(INSTALL_DATA) @srcdir@/$$f @datarootdir@/html; done


EXTRA_DIST=$(PACKAGE_FILES) $(MAINS) $(FIGURES)