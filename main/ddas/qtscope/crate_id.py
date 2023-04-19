import inspect

from PyQt5.QtWidgets import QWidget, QSpinBox, QLabel, QHBoxLayout

class CrateID(QWidget):
    """
    CrateID class widget (QWidget).
    
    Configures crate ID module parameter for all modules installed in the 
    system using the value of the crate ID parameter from module 0 as a 
    reference value. Inconsistent crate IDs are set to the value of module 0.

    Attributes:
        param_names (list): List of DSP parameter names.
        nmodules (int): Number of modules installed in the crate.
        crate_id (QSpinBox): Spin box to set the crate ID value.

    Methods:
        configure(): Initialize GUI.
        update_dsp(): Update DSP from GUI.
        display_dsp(): Display current DSP in GUI.
    """
    
    def __init__(self, nmodules=None, *args, **kwargs):
        """
        CrateID class constructor.

        Arguments:
            nmodules (int): number of installed modules in the system 
                            (default=None).
        """
        
        super().__init__(*args, **kwargs)

        self.param_names = ["CrateID"]
        self.nmodules = nmodules
        
        self.crate_id = QSpinBox()
        hbox = QHBoxLayout()
        hbox.addWidget(QLabel("Crate ID:"))
        hbox.addWidget(self.crate_id)

        # Define layout:
        
        self.setLayout(hbox)
        
    def configure(self, mgr):
        """  
        Initialize and display widget settings from the DSP dataframe.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.

        Raises:
            ValueError: If the crate ID values are not consistent for all 
                        modules.
        """
        
        # Restrict to some reasonable system size (5 crates):
        
        self.crate_id.setRange(0, 4)

       # Check crate ID consistency for all channels:

        id_list = []
        for i in range(self.nmodules):
            id_list.append(mgr.get_mod_par(i, self.param_names[0]))

        try:
            if not all(id == id_list[0] for id in id_list):
                raise ValueError("Inconsistent crate IDs on Mod. {}: read {}, expected {}".format(i, id, id_list[0]))
        except ValueError as e:
            print("{}:{}: Caught exception -- {}. Setting crate IDs to the crate ID value read from Mod. 0. Click 'Apply' to update settings. Check your settings file, it may be corrupt.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))
            for i in range(self.nmodules):
                mgr.set_mod_par(i, self.param_names[0], id_list[0])
        finally:
            self.display_dsp(mgr)
        
    def update_dsp(self, mgr):
        """
        Update dataframe from GUI values.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
        """

        for i in range(self.nmodules):
            mgr.set_mod_par(i, self.param_names[0], self.crate_id.value())

    def display_dsp(self, mgr, set_state=False):
        """
        Update GUI with dataframe values.

        Arguments:
            mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
            set_state (bool): Set display state (unused).
        """
          
        self.crate_id.setValue(mgr.get_mod_par(0, self.param_names[0]))        

class CrateIDBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """CrateIDBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """
        Create an instance of the widget and return it to the caller.

        Returns:
            CrateID: Instance of the DSP class widget.
        """        
            
        return CrateID(*args, **kwargs)
