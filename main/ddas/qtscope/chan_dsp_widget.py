import numpy as np

from PyQt5.QtGui import QDoubleValidator
from PyQt5.QtWidgets import QWidget, QGridLayout, QLabel, QLineEdit, QVBoxLayout, QSizePolicy

import colors

# @todo 0-index the DSP grid?
# @todo Some visual indication settings are changed but not applied.

class ChanDSPWidget(QWidget):
    """
    Channel DSP tab widget (QWidget).
    
    Generic channel DSP tab widget intended to be subclassed for particular 
    families of DSP parameters. This class interacts only with the internal 
    DSP settings and its own display. Note that the actual DSP parameters are 
    1-indexed while the DSP grid is 0-indexed. Provides template methods for 
    subclasses which are called in the appropriate class methods.

    Attributes:
        param_names (list): List of DSP parameter names.
        param_labels (list): List of DSP parameter GUI column titles.
        nchannels (int): Number of channels per module.
        has_extra_params (bool): Extra parameter flag.
        param_grid (QGridLayout): Grid of QWidgets to display DSP parameters.

    Methods:
        configure(): Initialize GUI.
        update_dsp(): Update DSP from GUI.
        display_dsp(): Display current DSP in GUI.
        copy_chan_dsp(): Copy DSP from channel idx in GUI.
        copy_mod_dsp(): Display copy_dsp in GUI.
    """
    
    def __init__(
            self,
            param_names=None, param_labels=None, nchannels=16,
            *args, **kwargs
    ):
        """
        ChanDSPWidget class constructor.

        Initialize generic channel DSP widget, set parameter validators and 
        labels. Channel DSP is displayed on an nchannel x nDSP grid of 
        QLineEdit widgets. Note that the actual DSP parameters are 0-indexed
        while the grid is 1-indexed. 
        
        Arguments:
            param_names (list): DSP parameter names for API read/write calls.
            param_labels (list): Column labels for the GUI.
            nchannels (int): Number of channels per module (optional, 
                             defualt=16). 
        """
        
        super().__init__(*args, **kwargs)

        self.param_names = param_names
        self.param_labels = param_labels
        self.nchannels = nchannels
        self.has_extra_params = False
        
        # Subwidget configuration:
        
        dsp_grid = QWidget()
        self.param_grid = QGridLayout(dsp_grid)

        self.param_grid.addWidget(QLabel("Ch."), 0, 0)
        for col, label in enumerate(self.param_labels, 1):
            self.param_grid.addWidget(QLabel(label), 0, col)

        # @todo (ASC 3/20/23): There is some Pythonic equivalent to this:
        #   for i, row in zip(range(nchannels), range(1, nchannels+1))
        # but it looks and feels horrible. Perhaps equally as irritating
        # to have to set row = i + 1 every time though, or having i+1s
        # floating around everywhere.
        
        for i in range(self.nchannels):
            self.param_grid.addWidget(QLabel("%i" %i), i+1, 0)
            for col, _ in enumerate(self.param_labels, 1): 
                w = QLineEdit()
                
                # Generally this is a double, overwrite in subclasses
                # if needed:
                
                w.setValidator(
                    QDoubleValidator(
                        0, 999999, 3,
                        notation=QDoubleValidator.StandardNotation
                    )
                )
                
                self.param_grid.addWidget(w, i+1, col)
        
        # Define layout and add widgets:
        
        layout = QVBoxLayout()
        layout.addWidget(dsp_grid)
        layout.addStretch()
        self.setLayout(layout)
        
    def configure(self, mgr, mod):
        """        
        Initialize and display widget settings from the DSP dataframe. 
        
        We just display the contents of the internal DSP, though some 
        widgets inheriting from this class may perform additional actions 
        by overriding this function if necessary.
        
        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
            mod (int): Module number.
        """
        
        self.display_dsp(mgr, mod)

    def update_dsp(self, mgr, mod):
        """
        Update dataframe from GUI values.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
            mod (int): Module number.
        """
        
        for i in range(self.nchannels):
            for col, name in enumerate(self.param_names, 1):
                val = float(
                    self.param_grid.itemAtPosition(i+1, col).widget().text()
                )
                mgr.set_chan_par(mod, i, name, val)
                
                
    def display_dsp(self, mgr, mod):
        """
        Update GUI with dataframe values.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
            mod (int): Module number.
        """
        
        for i in range(self.nchannels):
            for col, name in enumerate(self.param_names, 1):
                val = np.format_float_positional(
                    mgr.get_chan_par(mod, i, name),
                    precision=3,
                    unique=False
                )
                self.param_grid.itemAtPosition(i+1, col).widget().setText(val)
            
    def copy_chan_dsp(self, idx):
        """
        Copy channel parameters from a single channel to all other 
        channels on the module. Do not modify the underlying dataframe.

        Arguments:
            idx (int): Channel index to copy parameters from.
        """

        for i in range(self.nchannels):
            for col, p in enumerate(self.param_names, 1):
                val = self.param_grid.itemAtPosition(idx+1, col).widget().text()
                self.param_grid.itemAtPosition(i+1, col).widget().setText(val)
