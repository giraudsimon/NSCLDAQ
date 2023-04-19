import numpy as np
import pandas as pd

from PyQt5.QtWidgets import QWidget, QLabel, QHBoxLayout, QSpinBox

from chan_dsp_widget import ChanDSPWidget

class EnergyFilter(ChanDSPWidget):
    """
    Energy filter DSP tab (ChanDSPWidget).
    
    Attributes:
        extra_params (Python list): Extra DSP settings configured on this tab.
    """
    
    def __init__(self, *args, **kwargs):
        """
        EnergyFilter class constructor.  

        Keyword arguments:
            module (int): Module number from factory create method.
        """
        
        # XIA API parameter names:
        
        param_names = [
            "ENERGY_RISETIME",
            "ENERGY_FLATTOP"
        ]
        
        # Parameter labels on the GUI:
        
        param_labels = [
            "EnergyRise [us]",
            "EnergyGap [us]"
        ]
        
        # Create instance of the parent class with these variables:
        
        super().__init__(param_names, param_labels, *args, **kwargs)
        
        # Add filter range selection mechanism. Filter length is limited to
        # 127 decimated clock cycles, filter range averages 2^range samples
        # (range = [1,6]) prior to filtering logic, see Pixie-16 User's
        # manual Sec. 6.5.
        
        self.has_extra_params = True        
        self.extra_params = ["SLOW_FILTER_RANGE"]
        
        self.filter_range = QSpinBox()
        self.filter_range.setRange(1, 6)
        widget = QWidget()
        hbox = QHBoxLayout()
        hbox.addWidget(QLabel("Filter range:"))
        hbox.addWidget(self.filter_range)
        widget.setLayout(hbox)
        widget.setFixedWidth(150)

        layout = self.layout()
        layout.addWidget(widget)
        self.setLayout(layout)

    #
    # Overridden class methods
    #
    
    def configure(self, mgr, mod):
        """
        Overridden configuration operations.

        Initializes and displays configurable SLOW_FILTER_RANGE module 
        parameter on the tab.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
            mod (int): Module number.
        """

        self.filter_range.setValue(
            mgr.get_mod_par(mod, "SLOW_FILTER_RANGE")
        )
        super().configure(mgr, mod)
        
    def update_dsp(self, mgr, mod):
        """
        Overridden update operations.

        Update DSP storage SLOW_FILTER_RANGE from the GUI.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
            mod (int): Module number.
        """
        
        mgr.set_mod_par(mod, "SLOW_FILTER_RANGE", self.filter_range.value())
        super().update_dsp(mgr, mod)
        
    def display_dsp(self, mgr, mod):
        """
        Overridden template display operations.

        Display SLOW_FILTER_RANGE.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
            mod (int): Module number.
        """
               
        self.filter_range.setValue(mgr.get_mod_par(mod, "SLOW_FILTER_RANGE"))
        super().display_dsp(mgr, mod)
        
class EnergyFilterBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """EnergyFilterBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """
        Create an instance of the widget and return it to the caller.

        Returns:
            EnergyFilter: Instance of the DSP class widget.
        """        
            
        return EnergyFilter(*args, **kwargs)
