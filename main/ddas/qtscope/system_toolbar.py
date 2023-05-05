from PyQt5.QtWidgets import QToolBar, QPushButton

import colors

class SystemToolBar(QToolBar):
    """
    System-level function toolbar (QToolBar).

    Attributes:
        b_boot (QPushButton): Button for system boot.
        b_chan_gui (QPushButton): Button to open channel DSP GUI.
        b_mod_gui (QPushButton): Button to open module DSP GUI.
        b_load_set (QPushButton): Button to load a settings file.
        b_save_set (QPushButton): Button to save a settings file.
        b_exit (QPushButton): Button to exit the application.

    Methods:
        disable(): Disable all toolbar widgets.
        enable(): Enable all toolbar widgets.
    """
    
    def __init__(self, *args, **kwargs):
        """SystemToolBar class constructor."""
        
        super().__init__(*args, **kwargs)

        self.setMovable(False)
        
        # Widget definitions:
        
        self.b_boot = QPushButton("Boot system")
        self.b_chan_gui = QPushButton("Channel DSP")
        self.b_mod_gui = QPushButton("Module DSP")
        self.b_load = QPushButton("Load settings")
        self.b_save = QPushButton("Save settings")
        self.b_exit = QPushButton("Exit")

        self.b_boot.setStyleSheet(colors.RED_TEXT)
        self.b_chan_gui.setStyleSheet(colors.YELLOW)
        self.b_mod_gui.setStyleSheet(colors.YELLOW)
        self.b_load.setStyleSheet(colors.ORANGE)
        self.b_save.setStyleSheet(colors.ORANGE)
        self.b_exit.setStyleSheet(colors.RED)

        # Add widgets to the toolbar:
        
        self.addWidget(self.b_boot)
        self.addWidget(self.b_chan_gui)
        self.addWidget(self.b_mod_gui)
        self.addWidget(self.b_load)
        self.addWidget(self.b_save)
        self.addWidget(self.b_exit)

        # Set initial states:
        
        self.disable()
        self.b_boot.setEnabled(True)
                
    def disable(self):
        """Disable every child widget in the toolbar."""
        
        for c in self.children():
            if(c.isWidgetType()):
                c.setEnabled(False)
                c.repaint()

        # Exit button is always enabled:
                
        self.b_exit.setEnabled(True)

    def enable(self):
        """Enable every child widget in the toolbar."""
        
        for c in self.children():
            if(c.isWidgetType()):
                c.setEnabled(True)
                c.repaint()
        
class SystemToolBarBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """SystemToolbarBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """
        Create an instance of the toolbar and return it to the caller.

        Returns:
            SystemToolBar: Instance of the toolbar class.
        """
                    
        return SystemToolBar(*args, **kwargs)
