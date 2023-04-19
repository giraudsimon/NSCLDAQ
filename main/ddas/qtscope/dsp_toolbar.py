from PyQt5.QtWidgets import QToolBar, QPushButton, QSpinBox, QWidget, QSizePolicy

import colors

class DSPToolBar(QToolBar):
    """
    Toolbar for configuring module DSP (QToolBar).

    Attributes:
        b_apply (QPushButton): Button to apply parameters.
        b_load (QPushButton): Button to load parameters.
        b_copy_mod (QPushButton): Button to copy module DSP.
        copy_mod(QSpinBox): Module selection spinbox.
        b_copy_chan (QPushButton): Button to copy channel DSP.
        copy_chan(QSpinBox): Channel selection spinbox.
        b_cancel (QPushButton): Button to close the window.
        copy_mod_action (QAction): Command interface for copy module button.
        copy_mod_sb_action (QAction): Command interface for module spinbox.
        copy_chan_action (QAction): Command interface for copy channel button.
        copy_chan_sb_action (QAction): Command interface for channel spinbox.

    Methods:
        disable(): Disable all toolbar widgets.
        enable(): Enable all toolbar widgets.
        enable_mod_dsp(): Enable widgets for module DSP.
    """
    
    def __init__(self, *args, **kwargs):
        """DSPToolBar class constructor."""
        
        super().__init__(*args, **kwargs)

        self.setMovable(False)
        
        # Widget definitions:
        
        self.b_apply = QPushButton("Apply")
        self.b_load = QPushButton("Load")
        self.b_copy_mod = QPushButton("Copy mod.")
        self.copy_mod = QSpinBox() # Range set on boot.
        self.b_copy_chan = QPushButton("Copy chan.")
        self.copy_chan = QSpinBox() # Range set on boot.
        self.b_cancel = QPushButton("Cancel")

        self.b_apply .setStyleSheet(colors.CYAN)
        self.b_load.setStyleSheet(colors.CYAN)
        self.b_copy_mod.setStyleSheet(colors.BLUE)
        self.b_copy_chan.setStyleSheet(colors.BLUE)
        self.b_cancel.setStyleSheet(colors.RED)

        # Expanding blank space:
        
        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        # Add widgets to the toolbar. Actions for the copy module and copy
        # channel widgets are used to change their visibility in the toolbar.
        # See https://doc.qt.io/qt-5/qtoolbar.html#addWidget:
        
        self.addWidget(self.b_apply)
        self.addWidget(self.b_load)
        self.copy_mod_action = self.addWidget(self.b_copy_mod)
        self.copy_mod_sb_action = self.addWidget(self.copy_mod)
        self.copy_chan_action = self.addWidget(self.b_copy_chan)
        self.copy_chan_sb_action = self.addWidget(self.copy_chan)
        self.addWidget(spacer)
        self.addWidget(self.b_cancel)

    def disable(self):
        """Disable every child widget in the toolbar."""
       
        for c in self.children():
            if(c.isWidgetType()):
                c.setEnabled(False)
                c.repaint()            
                
    def enable(self):
        """Enable every child widget in the toolbar."""
        
        for c in self.children():
            if(c.isWidgetType()):
                c.setEnabled(True)
                c.repaint()

    def enable_mod_dsp(self):
        """Enable module-DSP-specific actions."""
        
        self.disable()
        
        self.copy_mod_action.setVisible(False)
        self.copy_mod_sb_action.setVisible(False)
        self.copy_chan_action.setVisible(False)
        self.copy_chan_sb_action.setVisible(False)
        self.b_apply.setEnabled(True)
        self.b_load.setEnabled(True)
        self.b_cancel.setEnabled(True) 

class DSPToolBarBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """DSPToolbarBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """
        Create an instance of the toolbar and return it to the caller.

        Returns:
            DSPToolBar: Instance of the toolbar class.
        """

        return DSPToolBar(*args, **kwargs)
