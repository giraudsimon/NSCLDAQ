import inspect

import bitarray as ba
ver = [int(i) for i in ba.__version__.split(".")]
if bool(ver[0] >= 1 or (ver[0] == 1 and ver[1] >= 6)):
    from bitarray.util import ba2int, int2ba
else:
    from converters import ba2int, int2ba

from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QCheckBox, QWidget, QHBoxLayout

import xia_constants as xia
from chan_dsp_widget import ChanDSPWidget

class Trace(ChanDSPWidget):
    """Trace DSP tab (ChanDSPWidget)."""
    
    def __init__(self, *args, **kwargs):
        """
        Trace class constructor.  

        Keyword arguments:
            module (int): Module number from factory create method.
        """
        
        # XIA API parameter names:
        
        param_names = [
            "TRACE_LENGTH",
            "TRACE_DELAY"
        ]

        # Parameter labels on the GUI:
        
        param_labels = [
            "TraceLength [us]",
            "TraceDelay [us]"
        ]
        
        # Create instance of the parent class with these variables. Module
        # number gets passed via kwargs.
        
        super().__init__(param_names, param_labels, *args, **kwargs)
        
        # Add trace capture CSRA checkbox:

        self.has_extra_params = True        
        self.extra_params = ["CHANNEL_CSRA"]
        
        self.cb_enabled = QCheckBox("Record traces", self)
        widget = QWidget()
        hbox = QHBoxLayout()
        hbox.addWidget(self.cb_enabled)
        widget.setLayout(hbox)
        widget.setFixedWidth(150)

        layout = self.layout()
        layout.addWidget(widget)
        self.setLayout(layout)

    #
    # Override class methods
    #

    def configure(self, mgr, mod):
        """
        Overridden configure options.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
            mod (int): Module number.

        Raises:
            ValueError: If trace enable bits are not consistent for all 
                        channels on the module.
        """

        # Check trace enable bit value consistency for all channels:

        enb_list = []
        for i in range(self.nchannels):
            csra = int2ba(
                int(mgr.get_chan_par(mod, i, "CHANNEL_CSRA")), 32, "little"
            )
            enb = csra[xia.CSRA_TRACE_ENABLE]
            enb_list.append(enb)

        try:
            if not all(enb == enb_list[0] for enb in enb_list):
                raise ValueError(
                    f"Inconsistent trace enable CSRA bits read on Mod. {mod}"
                )
        except ValueError as e:
            print("{}:{}: Caught exception -- {}. Check your settings file, it may be corrupt.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))
        finally:
            super().configure(mgr, mod)
    
    def update_dsp(self, mgr, mod):
        """
        Overridden update operations.

        Update DSP storage CSRA from based on the trace enable setting from 
        the GUI.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
            mod (int): Module number.
        """
        
        enb = self.cb_enabled.isChecked()
        
        for i in range(self.nchannels):
            
            # Read the CSRA and set the bit based on the checkbox:
            
            csra = int2ba(
                int(mgr.get_chan_par(mod, i, "CHANNEL_CSRA")), 32, "little"
            )
            csra[xia.CSRA_TRACE_ENABLE] = enb
            
            # Convert back from bit array to value and write:
            
            mgr.set_chan_par(mod, i, "CHANNEL_CSRA", float(ba2int(csra)))
            
        super().update_dsp(mgr, mod)
            
    def display_dsp(self, mgr, mod):
        """
        Overridden display operations.

        Display trace enable bit from CSRA. Consistency of trace enable bits 
        across channels is checked on configure and individual channels cannot 
        be manipulated on the GUI, so we assume here that CSRA trace enable bit
        on channel 0 is shared by all other channels.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
            mod (int): Module number.
        """
        
        csra = int2ba (
            int(mgr.get_chan_par(mod, 0, "CHANNEL_CSRA")), 32, "little"
        )
        enb = csra[xia.CSRA_TRACE_ENABLE]
        self.cb_enabled.setChecked(enb)
        super().display_dsp(mgr, mod)
        
class TraceBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """TraceBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """
        Create an instance of the widget and return it to the caller.

        Returns:
            Trace: Instance of the DSP class widget.
        """        
            
        return Trace(*args, **kwargs)
