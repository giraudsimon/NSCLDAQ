from PyQt5.QtWidgets import QWidget, QVBoxLayout

from chan_dsp_widget import ChanDSPWidget

class QDCLen(ChanDSPWidget):
    """QDC lengths DSP tab (ChanDSPWidget)."""
    
    def __init__(self, *args, **kwargs):
        """QDCLen class constructor."""
        
        # XIA API parameter names:
        
        param_names = [
            "QDCLen0",
            "QDCLen1",
            "QDCLen2",
            "QDCLen3",
            "QDCLen4",
            "QDCLen5",
            "QDCLen6",
            "QDCLen7"
        ]
        
        # Parameter labels on the GUI:
        
        param_labels = [
            "QDCLen0 [us]",
            "QDCLen1 [us]",
            "QDCLen2 [us]",
            "QDCLen3 [us]",
            "QDCLen4 [us]",
            "QDCLen5 [us]",
            "QDCLen6 [us]",
            "QDCLen7 [us]"
        ]
        
        # Create instance of the parent class with these variables:
        
        super().__init__(param_names, param_labels, *args, **kwargs)

class QDCLenBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """QDCLenBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """
        Create an instance of the widget and return it to the caller.

        Returns:
            QDCLen: Instance of the DSP class widget.
        """        
            
        return QDCLen(*args, **kwargs)
