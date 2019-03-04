PYTHONPROGRAMS = boot.py
PYTHONPACKAGES = Rings.py Programs.py ssh.py config.py

PYTHONDIR      = @prefix@/pythonLibs/nscldaq/boot

install-exec-local:
	$(mkinstalldirs) @bindir@
	$(mkinstalldirs) $(PYTHONDIR)
	for f in $(PYTHONPROGRAMS); do $(INSTALL_SCRIPT)  @srcdir@/$$f @bindir@/`basename $$f .py`; done
	for f in $(PYTHONPACKAGES); do $(INSTALL_SCRIPT)  @srcdir@/$$f $(PYTHONDIR) ; done
	touch $(PYTHONDIR)/__init__.py
	$(INSTALL_SCRIPT) @srcdir@/runner.bash @bindir@


EXTRA_DIST=$(PYTHONPROGRAMS) $(PYTHONPACKAGES) runner.bash