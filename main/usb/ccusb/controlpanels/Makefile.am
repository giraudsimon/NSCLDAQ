PACKAGES=ledcontrol.tcl ULMTriggerGUIPanel.tcl

APPS=ledph7106.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs @prefix@/TclLibs/ccusbpanels @bindir@
	for f in $(PACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/ccusbpanels; done
	echo pkg_mkIndex @prefix@/TclLibs/ccusbpanels "*.tcl" | @TCLSH_CMD@
	for f in $(APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f     @bindir@; done
	$(INSTALL_SCRIPT) @srcdir@/ULMTriggerGUI.tcl @bindir@/ULMTriggerGUI 
	$(mkinstalldirs) @prefix@/share/html/figures
	$(INSTALL_DATA) @srcdir@/ulmtrigger_startup.png @prefix@/share/html/figures/.
	$(INSTALL_SCRIPT) @srcdir@/ULMTriggerGUI.tcl @bindir@/ULMTriggerGUI 

EXTRA_DIST=$(APPS) $(PACKAGES) ULMTriggerGUI.tcl ulmtrigger.xml @srcdir@/ulmtrigger_startup.png


