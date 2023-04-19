import sys, os
import pandas as pd
import json
import inspect
import copy
import time

from PyQt5.QtCore import Qt, QThreadPool
from PyQt5.QtGui import QCloseEvent
from PyQt5.QtWidgets import QMainWindow, QVBoxLayout, QWidget,QApplication, QFileDialog

from pixie_utilities import SystemUtilities, RunUtilities, TraceUtilities
from dsp_manager import DSPManager

from plot import Plot 
from chan_dsp_gui import ChanDSPGUI 
from mod_dsp_gui import ModDSPGUI
from worker import Worker
from run_type import RunType
from trace_analyzer import TraceAnalyzer

import colors

# Removes the webproxy from spdaq machines:

os.environ['NO_PROXY'] = ""
os.environ['XDG_RUNTIME_DIR'] = os.environ.get("PWD")

DEBUG = False
OFFLINE_MODE = False

# @todo Would like to run the system as per custom DSP parameter formatted
# text file output -- as currently implemented the save DSP settings do not
# contain a full set of DSP parameters and are written using some standard
# python module functions. The XIA API call to boot modules requires a DSP
# settings file of a particular size and format which is inconsistent with
# our custom file format and differs by API version.

# @todo Toolbar disable shouldn't disable cancel button to close the windows.

class MainWindow(QMainWindow):
    """
    Main GUI (QMainWindow).

    Instances XIA API managers and internal DSP data storage as well as 
    toolbars for interacting with the managers and DSP. Tracks run status 
    to handle toolbar button functions. Contains plotting widget for displaying
    channel traces and histograms.

    Attributes:
        pool (QThreadPool): Global thread pool.
        dsp_mgr (DSPManager): Manager for internal DSP and interface for XIA 
                              API read/write operations.
        sys_utils (SystemUtilities): Interface to XIA API for system-level 
                                     tasks.
        trace_utils (TraceUtilities): Interface to XIA API for trace 
                                      acquisition.
        run_utils (RunUtilities): Interface to XIA API for MCA-mode run control.
        chan_gui (ChanDSPGUI): Channel DSP GUI.
        mod_gui (ModDSPGUI): Module DSP GUI.
        sys_toolbar (SystemToolBar): Toolbar for the SystemUtilities.
        acq_toolbar (AcquisitionToolBar): Toolbar for the RunUtilities.
        mplplot (Plot): matplotlib plotting widget.
        run_active (bool): True when an energy histogram or baseline run is 
                           active, False otherwise.
        active_type (Enum member): The run type set at run start, INACTIVE if
                                   when no run is active.
        trace_info (dict): Single channel ADC trace information from last 
                           single channel acquisition.

    Methods:
        closeEvent(): Overridden QWidget closeEvent to close all popups.
    """
    
    def __init__(
            self,
            chan_dsp_factory, mod_dsp_factory, toolbar_factory, fit_factory,
            *args, **kwargs
    ):
        """
        GUI MainWindow constructor.
        
        Arguments:
            chan_dsp_factroy (WidgetFactory): Factory for implemented channel
                                              DSP widgets.
            mod_dsp_factroy (WidgetFactory): Factory for implemented module DSP
                                             widgets.
            toolbar_factory (WidgetFactory): Factory for implemented toolbar 
                                             widgets.
            fit_factory (FitFactory): Factory for implemented fitting methods.
        """
        
        super().__init__(*args, **kwargs)
            
        self.setWindowTitle("QtScope (DEVEL) -- ''Just the goods, bare and plain.''")            
        self.resize(1280, 720)
        self.setWindowFlag(Qt.WindowMinimizeButtonHint, True)
        self.setWindowFlag(Qt.WindowMaximizeButtonHint, True)
        self.setMouseTracking(True)
        
        # Access to global thread pool for this applicaition:
        
        self.pool = QThreadPool.globalInstance()
            
        # XIA API managers:

        self.dsp_mgr = DSPManager()
        self.sys_utils = SystemUtilities()
        self.trace_utils = TraceUtilities()
        self.run_utils = RunUtilities()
        
        # Configure managers:
        
        if OFFLINE_MODE:
            self.sys_utils.set_boot_mode(1) # 1: offline, 0: online.
            self.trace_utils.set_use_generator(True) # Use generator data.
            self.run_utils.set_use_generator(True) # Use generator data.
            
        # DSP and trace analysis:
        
        self.trace_analyzer = TraceAnalyzer(self.dsp_mgr)
        self.trace_info = {
            "trace": None,
            "module": None,
            "channel": None
        }

        # Create managers for manipulating DSP settings:
        
        self.mod_gui = ModDSPGUI(mod_dsp_factory, toolbar_factory)
        self.chan_gui = ChanDSPGUI(chan_dsp_factory, toolbar_factory)

        #
        # Main layout GUI
        #

        self.sys_toolbar = toolbar_factory.create("sys")
        self.acq_toolbar = toolbar_factory.create("acq")
        self.mplplot = Plot(toolbar_factory, fit_factory)

        # Set initial run state information from the manager and toolbar:
        
        self.run_active = self.run_utils.get_run_active() # Better be false...
        self.active_type = RunType.INACTIVE

        # Define the main layout and add widgets:

        self.addToolBar(self.sys_toolbar)
        self.addToolBarBreak()
        self.addToolBar(self.acq_toolbar)
        
        # Central widget for the main window:
        
        self.setCentralWidget(self.mplplot)
        
        #
        # Signal connections
        #
        
        # System toolbar:
        
        self.sys_toolbar.b_boot.clicked.connect(self._boot)
        self.sys_toolbar.b_chan_gui.clicked.connect(self._show_chan_gui)
        self.sys_toolbar.b_mod_gui.clicked.connect(self._show_mod_gui)
        self.sys_toolbar.b_save.clicked.connect(self._save_settings)
        self.sys_toolbar.b_load.clicked.connect(self._load_settings)
        self.sys_toolbar.b_exit.clicked.connect(self._system_exit)
        
        # Acquisition toolbar:
        
        self.acq_toolbar.b_read_trace.clicked.connect(self._read_data)
        self.acq_toolbar.b_analyze_trace.clicked.connect(self._analyze_trace)
        self.acq_toolbar.b_read_data.clicked.connect(self._read_data)
        self.acq_toolbar.b_run_control.clicked.connect(self._run_control)

    #
    # Public methods
    #

    def closeEvent(self, event):
        """
        Overridden QWidget closeEvent function called when the main window is 
        exited via the [X] button rather than exit. Calls the same system exit 
        function as the button to close the connection to the modules and exit
        gracefully.

        Arguments:
            event (QCloseEvent): The handled signal. Always accepted.
        """
        
        event.accept()
        self._system_exit()
    
    #
    # Private methods
    #
    
    def _boot(self):
        """
        Boots the system using the SystemUtilities to call the relavent XIA API 
        functions. If the boot is successful, configure the DSP and DSP GUIs. 
        Only attempt to boot if the system has not been booted already.
        """
        
        # Access thread from global thread pool to boot:
        
        if self.sys_utils.get_boot_status() == False:
            worker = Worker(self.sys_utils.boot)
            worker.signals.running.connect(self.sys_toolbar.disable)
            worker.signals.running.connect(self.acq_toolbar.disable)
            worker.signals.finished.connect(self._on_boot)
            worker.signals.finished.connect(self.sys_toolbar.enable)
            worker.signals.finished.connect(self.acq_toolbar.enable)
            self.pool.start(worker)

    def _on_boot(self):
        """System configuration on successful system boot."""
        
        if self.sys_utils.get_boot_status() == True:

            # Populate list of module MSPS. Length of list == number of
            # installed modules in the crate:
            
            msps_list = []
            for i in range(self.sys_utils.get_num_modules()):
                msps_list.append(self.sys_utils.get_module_msps(i))

            # Configure DSP and managers. Performs first time load of DSP
            # settings from the Pixie modules.
            
            self.dsp_mgr.initialize_dsp(len(msps_list))
            self.chan_gui.configure(self.dsp_mgr, msps_list)
            self.mod_gui.configure(self.dsp_mgr, len(msps_list))
            
            # Repaint boot button, configure spinboxes, enable widgets:
            
            self.sys_toolbar.b_boot.setText("Booted")
            self.sys_toolbar.b_boot.setStyleSheet(colors.GREEN)
            self.acq_toolbar.current_mod.setRange(0, len(msps_list)-1)
            self.acq_toolbar.current_chan.setRange(0, 15)
            self.mplplot.toolbar.enable()
            
            print("QtScope system configuration complete!")

    # @todo (ASC 3/21/23): Assuming everything for now is binary .set file
    # a la XIA API 2. Couple of issues: one, XIA API 3 saves files in JSON
    # format which, while possibly called e.g. crate_1.set are _not_ backwards
    # compatable with XIA API 2. Two, support for our own file format must be
    # added. Problem one somewhat solves itself once we migrate to XIA API 3
    # because binary .set files _are_ compatabile with XIA API 3+, and
    # re-saving them will simply reformat as JSON. Which won't be an issue as
    # long as there aren't any XIA API 2 legacy systems kicking around...
    
    def _save_settings(self):
        """
        Save DSP parameters to an XIA settings file.
            
        Raises:        
            RuntimeError: If file format is unrecognized.
        """
        
        saved = False
        fname, ext = self._save_dialog()
        if fname and ext:
            try:
                if ext == "XIA settings file (*.set)":
                    self.sys_utils.save_set_file(fname)
                    saved = True
                else:
                    raise RuntimeError("Unrecognized file extension format '{}'".format(ext))
            except RuntimeError as e:
                print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))
            else:
                if saved:
                    print("DSP parameter file saved to:", fname)
    
    def _load_settings(self):
        """
        Load DSP parameters from an XIA settings file.

        Raises:        
            RuntimeError: If file format is unrecognized.
        """
        
        fname, ext = self._load_dialog()
        if fname and ext:
            try:
                if ext == "XIA settings file (*.set)":
                    self.sys_utils.load_set_file(fname)
                else:
                    raise RuntimeError("Unrecognized file extension format '{}'".format(ext))
            except RuntimeError:
                print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))

        # If the system has been booted, reload the DSP into the dataframe,
        # and reload the current channel DSP tab and module DSP (other channel
        # DSP loaded when a new tab is selected). Otherwise wait for system
        # boot (message issued by SystemMananager.cpp in this case).
        
        if self.sys_utils.get_boot_status() == True:
            self.dsp_mgr.load_new_dsp()
            self.chan_gui.load_dsp()
            self.mod_gui.load_dsp()
            
    def _save_dialog(self):
        """
        Get a file name and extension from QFileDialog.

        Returns:        
            fname (str): the file name from QFileDialog.getSaveFileName.
            ext (str): the file extension from QFileDialog.getSaveFileName.
        """
        
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        fname, ext = QFileDialog.getSaveFileName(
            self, "Save file", "",
            "XIA settings file (*.set)",
            options=options
            )
        if (fname, ext):
            return fname, ext
        else:
            return None, None

    def _load_dialog(self):
        """
        Get a file name and extension from QFileDialog.

        Returns:        
            fname (str): the file name from QFileDialog.getSaveFileName.
            ext (str): the file extension from QFileDialog.getSaveFileName.
        """
        
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        fname, ext = QFileDialog.getOpenFileName(
            self,"Save file", "", "XIA settings file (*.set)", options=options
        )
        if (fname, ext):
            return fname, ext
        else:
            return None, None
         
    def _system_exit(self):
        """Closes connection to Pixie modules and exits the application."""
        
        self.sys_utils.exit_system()
        self.pool.waitForDone(10000)
        self.pool.clear()
        app = QApplication.instance()
        app.quit()

    #
    # DSP management
    #
    
    def _show_chan_gui(self):
        """Show the channel DSP manager window."""
        
        self.chan_gui.show()

    def _show_mod_gui(self):
        """Show the module DSP manager window."""
        
        self.mod_gui.show()

    def _print_dsp(self):
        """Dump contents of DSP internal storage structure to the terminal."""
        
        self.dsp_mgr.print()

    #
    # Acquisition management
    #
    
    def _run_control(self):
        """Start or stop a run depending on current run status."""
        
        # If there is a thread running, wait for it to exit:
        
        if self.pool.activeThreadCount() > 0:
            print(self.pool.activeThreadCount(), "threads are currently communicating with the module(s). Waiting...")
            self.pool.waitForDone(10000)
            
        # Access thread from global thread pool for the begin/end operation
        # If a run is active, end it, otherwise start a new one:
        
        if self.run_active:
            if DEBUG:
                print("{}.{}: Run active status is {}, ending the current run".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, self.run_active))
                
            worker = Worker(self._end_run)
            worker.signals.finished.connect(self.chan_gui.toolbar.enable)
            worker.signals.finished.connect(self.mod_gui.toolbar.enable)
            worker.signals.finished.connect(self.acq_toolbar.enable)
            self.pool.start(worker)
        else:            
            if DEBUG:
                print("{}.{}: Run active status is {}, beginning a new run".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, self.run_active))
                
            worker = Worker(self._begin_run)
            worker.signals.running.connect(self.chan_gui.toolbar.disable)
            worker.signals.running.connect(self.mod_gui.toolbar.disable)
            worker.signals.running.connect(self.acq_toolbar.enable_run_active)
            self.pool.start(worker)

    def _begin_run(self):
        """
        Start a data run in the currently selected module. Check and update the
        run status, set the current run type, and update the acquisition 
        toolbar button states.
        """
        
        module = self.acq_toolbar.current_mod.value()
        
        # Set the run type from the index of the combo box on the toolbar
        
        self.active_type = RunType(self.acq_toolbar.run_type.currentIndex())
        
        if DEBUG:
            print("{}.{}: Beginning run in Mod. {} with run type {}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, module, self.active_type))

        # XIA API call to begin run in the current module
        
        self.run_utils.begin_run(module, self.active_type)
        self.run_active = self.run_utils.get_run_active()
        
        if DEBUG:
            print("{}.{}: Run started, active status is {}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, self.run_active))
            
        # Reconfigure the run control button and communicate run status
        
        if self.run_active:
            self.acq_toolbar.b_run_control.setText("End run")
            self.mplplot.on_begin_run(self.active_type)
            self.mod_gui.setEnabled(False)
            self.chan_gui.setEnabled(False)
            
    def _end_run(self):
        """
        Stop an MCA run in the currently selected module. Check and update the
        run status and update the acquisition toolbar button states.
        """
        
        module = self.acq_toolbar.current_mod.value()
        
        if DEBUG:
            print("{}.{}: Ending run in Mod. {} with run type {}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, module, self.active_type))

        # XIA API call to end run in the current module
        
        self.run_utils.end_run(module, self.active_type)
        self.run_active = self.run_utils.get_run_active()
            
        # Print run stats for histogram run and reconfigure GUI
        
        if not self.run_active:
            if self.active_type == RunType.HISTOGRAM:
                self.run_utils.read_stats(module)
            self.acq_toolbar.b_run_control.setText("Begin run")
            self.mod_gui.setEnabled(True)
            self.chan_gui.setEnabled(True)
            self.active_type = RunType.INACTIVE
            
        if DEBUG:
            print("{}.{}: End of run finalized, active type set to {}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, self.active_type))

    def _read_data(self):
        """
        Configure worker to read data from a module.

        If a run is active, read histogram or baseline data based on the 
        active run type, otherwise read a trace.
        """
        
        # If there is a thread running, wait for it to exit:
        
        if self.pool.activeThreadCount() > 0:
            print(self.pool.activeThreadCount(), "threads are currently communicating with the module(s). Waiting...")
            self.pool.waitForDone(10000)

        # Access thread from global thread pool for the data read operation.
        # If a run is active, read either an energy histogram or baseline
        # depending on the run type, otherwise read a trace.
        
        worker = Worker()
        worker.signals.running.connect(self.acq_toolbar.disable)
            
        if self.run_active:     
            worker.set_function(self._read_run_data)
            worker.signals.finished.connect(self.acq_toolbar.enable_run_active)
            
        else:
            worker.set_function(self._read_trace_data)
            worker.signals.running.connect(self.mod_gui.toolbar.disable)
            worker.signals.running.connect(self.chan_gui.toolbar.disable)
            worker.signals.running.connect(
                lambda enb=False: self.chan_gui.setEnabled(enb)
            )
            worker.signals.finished.connect(self.acq_toolbar.enable)
            worker.signals.finished.connect(self.mod_gui.toolbar.enable)
            worker.signals.finished.connect(self.chan_gui.toolbar.enable)
            worker.signals.finished.connect(
                lambda enb=True: self.chan_gui.setEnabled(enb)
            )

        # Start the worker now that its configured:
        
        self.pool.start(worker)

    def _read_run_data(self):
        """
        Read run (energy histogram or baseline) data from the currently 
        selected module and channel(s) then display using the plotting widget.
        """
        
        self.mplplot.figure.clear()
        module = self.acq_toolbar.current_mod.value()
        channel = self.acq_toolbar.current_chan.value()
  
        # Read from module and get data, then draw:
        
        if self.acq_toolbar.read_all.isChecked():
            for i in range(16):
                self.run_utils.read_data(module, i, self.active_type)
                data = self.run_utils.get_data(self.active_type)
                self.mplplot.draw_run_data(data, self.active_type, 4, 4, i+1)
        else:           
            self.run_utils.read_data(module, channel, self.active_type)
            data = self.run_utils.get_data(self.active_type)
            self.mplplot.draw_run_data(data, self.active_type)
            
    def _read_trace_data(self):
        """
        Read trace data from the currently selected module and channel(s) then 
        display using the plotting widget. 
        """
        
        self.mplplot.figure.clear()
        module = self.acq_toolbar.current_mod.value()
        channel = self.acq_toolbar.current_chan.value()

        # Retrieve trace from this module and channel and get its data. If
        # signal validation is required (fast acquisition mode is not
        # selected), the read function will validate and reacquire trace
        # signals until it either finds a good trace or hits a retry limit.
        
        if self.acq_toolbar.read_all.isChecked():
            for i in range(16):

                # Check signal validation and read:
                
                if self.acq_toolbar.fast_acq.isChecked():
                    self.trace_utils.read_fast_trace(module, i)
                else:
                    self.trace_utils.read_trace(module, i)

                data = self.trace_utils.get_trace_data()
                self.mplplot.draw_trace_data(data, 4, 4, i+1)
                    
                # Keep the single channel trace information:
                
                if i == channel:
                    self.trace_info.update({
                        "trace": copy.copy(data),
                        "module": module,
                        "channel": channel
                    })                  
        else:
           
            # Check signal validation and read:
            
            if self.acq_toolbar.fast_acq.isChecked():
                self.trace_utils.read_fast_trace(module, channel)
            else:
                self.trace_utils.read_trace(module, channel)

            data = self.trace_utils.get_trace_data()
            self.mplplot.draw_trace_data(data)

            # Keep the single channel trace information:
            
            self.trace_info.update({
                "trace": copy.copy(data),
                "module": module,
                "channel": channel
            })
    
    def _analyze_trace(self):
        """Setup worker to analyze a single-channel ADC trace."""
        
        worker = Worker(self._analyze_and_show_trace)
        worker.signals.running.connect(self.chan_gui.toolbar.disable)
        worker.signals.running.connect(self.mod_gui.toolbar.disable)
        worker.signals.running.connect(self.acq_toolbar.disable)
        worker.signals.finished.connect(self.chan_gui.toolbar.enable)
        worker.signals.finished.connect(self.mod_gui.toolbar.enable)
        worker.signals.finished.connect(self.acq_toolbar.enable)
        self.pool.start(worker)        
        
    # @todo Should analyze trace be an available feature if read all?    
    # @todo try-except block is pretty long for a single function.
    
    def _analyze_and_show_trace(self):
        """
        Display single channel trace filter output.
        
        Raises:
            ValueError: If the module number is changed between acquisition 
                        and analyze attempt.
            ValueError: If the channel number for a single-channel read is 
                        changed between acquisition and analysis.
        """
        
        self.mplplot.figure.clear()
        module = self.acq_toolbar.current_mod.value()
        channel = self.acq_toolbar.current_chan.value()
        
        # If there is no current trace data, acquire a trace:
        
        try: 
            if not self.trace_info["trace"]:
                
                # Check signal validation and read, update trace info:
                
                if self.acq_toolbar.fast_acq.isChecked():
                    self.trace_utils.read_fast_trace(module, channel)
                else:
                    self.trace_utils.read_trace(module, channel)
                    
                self.trace_info.update({
                    "trace": copy.copy(self.trace_utils.get_trace_data()),
                    "module": module,
                    "channel": channel
                })
            elif module != self.trace_info["module"]:
                
                # Module number changed between acquisition and analyze, cannot
                # recover because traces for the new module have not been
                # acquired, and analyze is designed to analyze a trace shown on
                # the current canvas.
                raise ValueError("Stored trace data for Mod. {}, Ch. {} does not match the current selection box Mod. {}, Ch. {}".format(self.trace_info["module"], self.trace_info["channel"], module, channel))
            elif self.acq_toolbar.read_all.isChecked() and channel != self.trace_info["channel"]:
                
                # Channel changed between acquisition and analysis. All traces
                # have been read, so get the correct data from its subplot.
                
                self.trace_info.update({
                    "trace": copy.copy(self.mplplot.get_subplot_data(channel)),
                    "module": module,
                    "channel": channel
                })
            elif not self.acq_toolbar.read_all.isChecked() and channel != self.trace_info["channel"]:
                
                # Single channel acquisition mode channel has been switched
                # since acquiring a trace (trace on the canvas does not match
                # current selection box).
                
                raise ValueError("Stored trace data for Mod. {}, Ch. {} does not match the current selection box Mod. {}, Ch. {}".format(self.trace_info["module"], self.trace_info["channel"], module, channel))
        except ValueError as e:
            print("{}.{}: Caught exception -- {}. New trace data must be acquired by clicking the 'Read trace' button prior to analysis.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))
        else:
            
            # No exceptions, analyze and draw:

            try:
                self.trace_analyzer.analyze(
                    self.trace_info["module"],
                    self.trace_info["channel"],
                    self.trace_info["trace"]
                )
            except Exception as e:
                print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))
            else:                
                self.mplplot.draw_analyzed_trace(
                    self.trace_info["trace"],
                    self.trace_analyzer.fast_filter,
                    self.trace_analyzer.cfd,
                    self.trace_analyzer.slow_filter
                )
            finally:
                
                # Reset the single channel trace information:
            
                self.trace_info.update({
                    "trace": None,
                    "module": None,
                    "channel": None 
                })            
            
    def _test(self):
        """A dummy function which can be hooked up to signals for testing."""
        
        print("{}.{}: Called".format(self.__class__.__name__, inspect.currentframe().f_code.co_name))
