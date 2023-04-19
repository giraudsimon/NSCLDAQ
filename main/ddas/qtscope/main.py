#!/usr/bin/env python3
import sys, os

sys.path.append(str(os.environ.get("DAQROOT"))+"/ddas/qtscope")

from PyQt5 import QtWidgets

from widget_factory import WidgetFactory
from fit_factory import FitFactory

from analog_signal import AnalogSignalBuilder
from trigger_filter import TriggerFilterBuilder
from energy_filter import EnergyFilterBuilder
from cfd import CFDBuilder
from adc_trace import TraceBuilder
from tau import TauBuilder
from csra import CSRABuilder
from mult_coincidence import MultCoincidenceBuilder
from timing_control import TimingControlBuilder
from baseline import BaselineBuilder
from qdclen import QDCLenBuilder

from crate_id import CrateIDBuilder
from csrb import CSRBBuilder
from trigconfig0 import TrigConfig0Builder

from system_toolbar import SystemToolBarBuilder
from acquisition_toolbar import AcquisitionToolBarBuilder
from dsp_toolbar import DSPToolBarBuilder
from plot_toolbar import PlotToolBarBuilder 

from fit_exp_creator import ExpFitBuilder
from fit_gauss_creator import GaussFitBuilder
from fit_gauss_p1_creator import GaussP1FitBuilder
from fit_gauss_p2_creator import GaussP2FitBuilder

from gui import MainWindow

def main():
    """QtScope main. Create factories and start the GUI."""

    # Create the factories:

    print ("Creating factory methods and registering builders...")

    cdf = create_chan_dsp_factory()
    mdf = create_mod_dsp_factory()
    tbf = create_toolbar_factory()
    ftf = create_fit_factory()
    
    # Start application and open the main GUI window:

    print("Factory creation complete, starting GUI...")

    app = QtWidgets.QApplication(sys.argv)
    gui = MainWindow(cdf, mdf, tbf, ftf)
    gui.show()
    sys.exit(app.exec_())

def create_chan_dsp_factory():
    """
    Create a widget factory and register channel DSP builders with it. 
    Return the factory to main.
        
    Returns:
        WidgetFactory: Factory with registered channel DSP widgets.
    """

    factory = WidgetFactory()

    print("Registering channel DSP...")
    
    factory.register_builder("AnalogSignal", AnalogSignalBuilder())
    factory.register_builder("TriggerFilter", TriggerFilterBuilder())
    factory.register_builder("EnergyFilter", EnergyFilterBuilder())
    factory.register_builder("CFD", CFDBuilder())
    factory.register_builder("Trace", TraceBuilder())
    factory.register_builder("Tau", TauBuilder())
    factory.register_builder("CSRA", CSRABuilder())
    factory.register_builder("MultCoincidence", MultCoincidenceBuilder())
    factory.register_builder("TimingControl", TimingControlBuilder())
    factory.register_builder("Baseline", BaselineBuilder())
    factory.register_builder("QDCLen", QDCLenBuilder())

    return factory

def create_mod_dsp_factory():
    """
    Create a widget factory and register module DSP builders with it. 
    Return the factory to main.
        
    Returns:
        WidgetFactory: Factory with registered module DSP widgets.
    """
                       
    factory = WidgetFactory()

    print("Registering module DSP...")
    
    factory.register_builder("CrateID", CrateIDBuilder())
    factory.register_builder("CSRB", CSRBBuilder())
    factory.register_builder("TrigConfig0", TrigConfig0Builder())
    
    return factory

def create_toolbar_factory():
    """
    Create a widget factory and register module DSP builders with it. 
    Return the factory to main.
        
    Returns:
        WidgetFactory: Factory with registered toolbar widgets.
    """
               
    factory = WidgetFactory()

    print("Registering toolbars...")
    
    factory.register_builder("sys", SystemToolBarBuilder())
    factory.register_builder("acq", AcquisitionToolBarBuilder())
    factory.register_builder("dsp", DSPToolBarBuilder())
    factory.register_builder("plot", PlotToolBarBuilder())

    return factory    

def create_fit_factory():
    """
    Create a fit factory and register builders with it. Return the 
    factory to main.
        
    Returns:
        FitFactory: Instance of the fit factory.
    """

    # Configuration dictionaries for fits:
    
    config_fit_exp = {
        "A": 1,      # Unused, will guess from the data if not set.
        "k": -0.003, # ~20 microseconds in 60 ns samples.
        "C": 1,      # Unused, will guess from the data if not set.
        "form": "f(x) = p[0]*exp(p[1]*x) + p[2]" # Function formula.
    }
    
    config_fit_gauss = {
        "A": 1,  # Unused, will guess from the data if not set.
        "mu": 0, # Unused, will guess from the data if not set.
        "sd": 1, # Unused, will guess from the data if not set.
        "form": "f(x) = p[0]*exp(-(x-p[1])^2 / (2*p[2]^2))" # Function formula.
    }
    
    config_fit_gauss_p1 = {
        "A": 1,  # Unused, will guess from the data if not set.
        "mu": 0, # Unused, will guess from the data if not set.
        "sd": 1, # Unused, will guess from the data if not set.
        "a0": 0, # Unused, will guess from the data if not set.
        "a1": 0, # Unused, will guess from the data if not set.
        "form": "f(x) = p[0]*exp(-(x-p[1])^2 / (2*p[2]^2))\n\t+ p[3] + p[4]*x" # Function formula.
    }

    config_fit_gauss_p2 = {
        "A": 1,  # Unused, will guess from the data if not set.
        "mu": 0, # Unused, will guess from the data if not set.
        "sd": 1, # Unused, will guess from the data if not set.
        "a0": 0, # Unused, will guess from the data if not set.
        "a1": 0, # Unused, will guess from the data if not set.
        "a2": 0, # Unused, will guess from the data if not set.
        "form": "f(x) = p[0]*exp(-(x-p[1])^2 / (2*p[2]^2))\n\t+ p[3] + p[4]*x + p[5]*x^2" # Function formula.
    }
    
    # Register fit factory classes:
    
    factory = FitFactory()
    
    print("Registering fit functions...")
    
    factory.register_builder("Exponential", ExpFitBuilder(), config_fit_exp)
    factory.register_builder("Gaussian", GaussFitBuilder(), config_fit_gauss)
    factory.register_builder(
        "Gaussian + linear", GaussP1FitBuilder(), config_fit_gauss_p1
    )
    factory.register_builder(
        "Gaussian + quadratic", GaussP2FitBuilder(), config_fit_gauss_p2
    )

    return factory

# Run main when executed as a script, the way we intend to do this:
    
if __name__ == "__main__":    
    main()
