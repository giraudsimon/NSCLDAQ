import matplotlib
matplotlib.use("Qt5Agg")
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg
import numpy as np
import inspect
import copy
import sys

from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QMessageBox
from PyQt5.QtGui import QPaintEvent

from fit_panel import FitPanel

import xia_constants as xia
from run_type import RunType

DEBUG = False

class Plot(QWidget):
    """
    Plotting widget for the GUI utilizing the matplotlib Qt5 backend.

    Attributes:
        figure(pyplot figure): matplotlib pyplot figure instance.
        canvas(FigureCanvasQTAgg): Canvas the figure renders onto imported 
                                       from Qt5 backend.
        fit_panel(QWidget): Trace and histogram fitting GUI widget.
        fit_factory (FitFactory): Factory method for fitting plot data.
        toolbar (NavigationToolbar2QT): Figure navigation toolbar imported 
                                            from Qt5 backend.

    Methods:
        draw_trace_data(): Draw single- or multuple-channel ADC trace data.
        draw_analyzed_trace(): Draw a single-channel ADC trace and its filter 
                               outputs. 
        draw_run_data(): Draw single- or multuple-channel energy histogram 
                         or baseline data.
        on_begin_run(): Draw a blank histogram-style canvas when starting
                        a histogram run.
        draw_test_data(): Draw a test figure with a random number of subplots.
        test_draw(): Draw random numbers on subplot idx of a test plot.
    """
    
    def __init__(self, toolbar_factory, fit_factory, *args, **kwargs):
        """
        Plot class constructor.

        Arguments:
            toolbar_factory (ToolBarFactory): Factory for implemented toolbars.
            fit_factory (FitFactory): Factory method for fitting plot data.
        """
        
        super().__init__(*args, **kwargs)
        
        #
        # Main layout
        #
        
        self.figure = plt.figure(tight_layout=True)
        self.canvas = FigureCanvasQTAgg(self.figure)
        self.fit_panel = FitPanel()
        self.toolbar = toolbar_factory.create("plot", self.canvas, self)

        # Initialize the factory with the known methods:
        
        self.fit_factory = fit_factory
        self.fit_factory.initialize(self.fit_panel.function_list)

        # Once initialized, we assume the config information contains some
        # valid text field giving the fit function formula:

        self._update_formula()

        # Disable toolbar on creation (enabled on boot):
        
        self.toolbar.disable()
        
        # Blank canvas with axes to show that this is the plot area:
        
        ax = self.figure.add_subplot(1, 1, 1)
        ax.set_xlim(0, 1)
        ax.set_ylim(0, 1)

        layout = QVBoxLayout()
        layout.addWidget(self.toolbar)
        layout.addWidget(self.canvas)
        self.setLayout(layout)

        #
        # Signal connections
        #
        
        self.toolbar.logscale.clicked.connect(self._set_yscale)
        self.toolbar.b_fit_panel.clicked.connect(self._show_fit_panel)

        self.fit_panel.b_fit.clicked.connect(self._fit)
        self.fit_panel.b_clear.clicked.connect(self._clear_fit)
        self.fit_panel.b_cancel.clicked.connect(self._close_fit_panel)
        self.fit_panel.function_list.currentTextChanged.connect(
            self._update_formula
        )
        
    def draw_trace_data(self, data, nrows=1, ncols=1, idx=1):
        """
        Draws an ADC trace on the plot canvas.

        Arguments:
            data (array): Array of trace data values.
            nrows (int):  Number of subplot rows (optional, default=1).
            ncols (int):  Number of subplot columns (optional, default=1).
            idx (int):  Subplot index in [1, nrows*ncols] (optional).
        """
        
        ax = self.figure.add_subplot(nrows, ncols, idx)            
        ax.plot(data, "-")            
        ax.set_xlabel("Sample number (60 ns/sample)")
        ax.set_ylabel("Voltage (ADC units)")
        ax.set_xlim(0, xia.MAX_ADC_TRACE_LEN)
        self._set_yscale(ax)

        if (nrows*ncols) > 1:
            ax.set_title("Channel {}".format(idx-1))#  Channels are 0-indexed.
            ax.tick_params(axis='x', labelsize=6)
            ax.tick_params(axis='y', labelsize=6)
            ax.xaxis.label.set_size(6)
            ax.yaxis.label.set_size(6)
        
        self.canvas.draw_idle()
        
    def draw_analyzed_trace(self, trace, fast_filter, cfd, slow_filter):
        """
        Draw a trace and its filter outputs.

        Arugments:
            trace (array): Single-channel ADC trace data.
            fast_filter (list): Computed fast filter output.
            cfd (list): computed CFD output.
            slow_filter (list): Computed slow filter output.
        """
        
        ax1 = self.figure.add_subplot(3, 1, 1)
        ax1.set_title("Trace")
        ax1.plot(trace, "-")
        
        ax2 = self.figure.add_subplot(3, 1, 2)
        ax2.set_title("Timing filters")
        ax2.plot(fast_filter, "-")
        ax2.plot(cfd, "-")
        ax2.legend(["Trigger filter", "CFD"], loc="upper right")
        
        ax3 = self.figure.add_subplot(3, 1, 3)
        ax3.set_title("Energy filter")
        ax3.plot(slow_filter, "-")

        # Get y range from data:
        
        margin = 0.1
        for ax in self.figure.get_axes():
            ymin = sys.float_info.max
            ymax = sys.float_info.min
            
            for line in ax.get_lines():
                lmin = np.amin(line.get_ydata())
                lmax = np.amax(line.get_ydata())
                if lmin < ymin:
                    ymin = lmin
                if lmax > ymax:
                    ymax = lmax
                    
            yrange = ymax - ymin
            ax.set_xlim(0, xia.MAX_ADC_TRACE_LEN)
            ax.set_ylim(ymin - margin*yrange, ymax + margin*yrange)
            
        self.canvas.draw_idle()
        
    def draw_run_data(self, data, run_type, nrows=1, ncols=1, idx=1):
        """
        Draws an energy histogram on the plot canvas.

        Arguments:
            data (array): Single-channel run data.
            run_type (Enum member): Type of run data to draw.
            nrows (int): Number of subplot rows (optional, default=1).
            ncols (int): Number of subplot columns (optional, default=1).
            idx (int): Subplot index in [1, nrows*ncols] (optional).
        """
        
        ax = self.figure.add_subplot(nrows, ncols, idx)
        ax.plot(data, "-")
        
        if run_type == RunType.HISTOGRAM:
            ax.set_xlabel("Energy (ADC units)")
        elif run_type == RunType.BASELINE:
            ax.set_xlabel("Baseline value (ADC units)")
            
        ax.set_ylabel("Counts/1 ADC unit")
        ax.set_xlim(0, xia.MAX_HISTOGRAM_LENGTH)
        self._set_yscale(ax)
        
        if (nrows*ncols) > 1:
            chan = idx - 1 # Channels are zero-indexed.
            ax.set_title("Channel {}".format(chan))
            ax.tick_params(axis='x', labelsize=6)
            ax.tick_params(axis='y', labelsize=6)
            ax.xaxis.label.set_size(6)
            ax.yaxis.label.set_size(6)
            
        self.canvas.draw_idle()
                        
    def on_begin_run(self, run_type):
        """
        Draws a blank histogram canvas when beginning a new run.
        
        Arguments:
            run_type (Enum member): Type of run data to draw.
        """
        self.figure.clear()

        try:
            if run_type != RunType.HISTOGRAM and run_type != RunType.BASELINE:
                raise ValueError("Encountered unexpected run type {}, select a valid run type and begin a new run".format(run_type))            
        except ValueError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))            
        else:
            ax = self.figure.add_subplot(1, 1, 1)
            if run_type == RunType.HISTOGRAM:
                ax.set_xlabel("ADC energy")
            elif run_type == RunType.BASELINE:
                ax.set_xlabel("Baseline value")
            ax.set_ylabel("Counts/1 ADC unit")
            ax.set_xlim(0, xia.MAX_HISTOGRAM_LENGTH)
            ax.set_ylim(0, 1)
            self._set_yscale(ax)        
            self.canvas.draw_idle()

    def get_subplot_data(self, idx):
        """
        Get y data from a single subplot by axis index. Returns an empty list 
        if the subplot does not have any associated data.

        Arguments:
            idx (int): Axes index number (note this is 0-indexed while the 
                       subplots are 1-indexed!)

        Returns:
            list: Data from the selected subplot.
        """
        
        axes = self.figure.get_axes()
        if axes[idx].get_lines():
            return np.ndarray.tolist(axes[idx].lines[0].get_ydata())
        else:
            return []        

    def draw_test_data(self):
        """
        Draw data points in (-1, 1) on a test canvas with a random number of 
        rows and columns with rows, columns in [1, 4]
        """
        
        self.figure.clear()
        
        rng = np.random.default_rng()
        nrows = rng.integers(1, 5)
        ncols = rng.integers(1, 5)
        
        for i in range(nrows):
            for j in range(ncols):
                idx = ncols*i + j + 1
                ax = self.figure.add_subplot(nrows, ncols, idx)
                data = 2*rng.random(size=5) - 1
                ax.plot(data, "-")
                
        self.canvas.draw_idle()
        
    #
    # Private methods
    #
    
    def _set_yscale(self, ax=None):
        """
        Configurs the axes display on the canvas.

        If the log scale checkbox is selected, immidiately redraw all subplots.
        """
        
        # Button clicked, redraw all otherwise just the current one:
        
        if self.sender():
            for ax in self.figure.get_axes():
                self._set_subplot_yscale(ax)
            self.canvas.draw_idle()
        else:
            self._set_subplot_yscale(ax)
                    
    def _set_subplot_yscale(self, ax):
        """
        Sets the y-axis display for a single figure (linear or log).
    
        The y-axis is autoscaled in log for trace data because all values are 
        greater than zero. For histogram data the axis scaling in log is 
        handled automatically, but in linear the y-axis is set to start at zero.

        Arguments:
            ax (matplotlib Axes): matplotlib class containing the figure 
                                  elements.
        """
        
        # Check number of data points to determine if this is a histogram:
        
        is_hist = False
        ymax = sys.float_info.min
        for line in ax.get_lines():
            ydata = line.get_ydata()
            lmax = np.amax(ydata)
            if lmax > ymax:
                ymax = lmax
                
            # This is a kludge, since we assume all histograms are max length.
            # Stop looking if we've found a histogram:
            
            if len(ydata) == xia.MAX_HISTOGRAM_LENGTH:
                is_hist = True
                break

        ax.autoscale(axis="y")
        if self.toolbar.logscale.isChecked():
            
            # Also a potential kludge: symmetric log handles possible zeroes
            # nicely in the case of empty histograms or inverted traces:
            
            if ymax > 0:
                ax.set_yscale("log")
            else:
                ax.set_yscale("symlog")
                ax.set_ylim(0.1, 1)
        else:
            ax.set_yscale("linear")
            if ymax > 0:
                if is_hist:
                    ax.set_ylim(0,None)
            else:
                ax.set_ylim(0, 1)

    def _show_fit_panel(self):
        """Show the fit panel GUI in a popup window."""

        self.fit_panel.show()

    def _fit(self):
        """Perform the fit based on the current fit panel settings."""
        
        if len(self.figure.get_axes()) == 1:
            ydata = self.get_subplot_data(0)
            if len(ydata) > 0:
                ax = plt.gca()
                fcn = self.fit_panel.function_list.currentText()
                config = self.fit_factory.configs.get(fcn)
                fit = self.fit_factory.create(fcn, **config)
                xmin, xmax = self._axis_limits(ax)
                params = [
                    float(self.fit_panel.p0.text()),
                    float(self.fit_panel.p1.text()),
                    float(self.fit_panel.p2.text()),
                    float(self.fit_panel.p3.text()),
                    float(self.fit_panel.p4.text()),
                    float(self.fit_panel.p5.text())
                ]

                # Get data in fitting range [xmin, xmax):

                x = [] # Just the plain x-axis value.
                y = []
                for i in range(len(ydata)):
                    if i >= xmin and i < xmax:
                        x.append(i)
                        y.append(ydata[i])
                        
                if DEBUG:
                    print("Function config params:", config)
                    print("Fit limits: {}, {}".format(xmin, xmax))
                    print("Initial guess params:", params)
                
                fitln = fit.start(x, y, params, ax, self.fit_panel.results)

                self.canvas.draw_idle()
            else:
                QMessageBox.about(self, "Warning", "Cannot perform the fit! There is no data on the displayed plot. Please acquire single-channel data and attempt the fit again.")
        else:
            QMessageBox.about(self, "Warning", "Cannot perform the fit! Currently displaying data from multiple channels or an analyzed trace. Please acquire single-channel data and attempt the fit again.")
            
    def _axis_limits(self, ax):
        """
        Get the fit limits based on the selected range. If no range is
        provided the currently displayed axes limits are assumed to be the 
        fit range.

        Arguments:
            ax (PyPlot axes): Currently displayed single channel data plot axis.
        
        Returns:
            int, int: Left and right axis limits.
        """
        
        left, right = ax.get_xlim()
        if self.fit_panel.range_min.text():
            left = int(self.fit_panel.range_min.text())
        else:
            left = int(ax.get_xlim()[0]) 
        if self.fit_panel.range_max.text():
            right = int(self.fit_panel.range_max.text())
        else:
            right = int(ax.get_xlim()[1])
            
        return left, right

    def _clear_fit(self):
        """Clear previous fits and reset the fit panel GUI."""

        if len(self.figure.get_axes()) == 1:

            # Delete all plots except for the data:
            
            ax = plt.gca()
            if len(ax.lines) > 1:
                for i in range(len(ax.lines) - 1):
                    ax.lines.pop(1)
                self.canvas.draw_idle()

            # Reset fit panel display:
                    
            self.fit_panel.reset()

    def _update_formula(self):
        """
        Update the fit function formula when a new fitting function is 
        selected from the drop down menu.
        """

        fcn = self.fit_panel.function_list.currentText()
        config = self.fit_factory.configs.get(fcn)
        self.fit_panel.function_formula.setText(config["form"])

    def _close_fit_panel(self):
        """Close the fit panel window."""

        self.fit_panel.close()
