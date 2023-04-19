import pandas as pd
import numpy as np
from scipy.optimize import curve_fit

class ExpFit:
    """
    Exponential fitting function: f(x) = A*exp(k*x) + C.

    Attributes:
        A (float): Amplitude.
        k (float): Exponential rate constant.
        C (float): Constant baseline.
        form (str): Function formula.

    Methods:
        exp(): Returns the array of function values evaluated over the 
               fitting range.
        set_initial_parameters(): Set initial parameter values. Guess from 
                                  the data if none are provided.
        start(): Implementation of the fitting algorithm.
    """
    
    def __init__(self, A, k, C, form):
        """
        Exponential fit function class constructor. Sets initial fit 
        parameters.
        
        Arguments:
            A (float): Amplitude.
            k (float): Exponential rate constant.
            C (float): Constant baseline.
            form (str): Function formula.
        """
        
        self.A = A
        self.k = k
        self.C = C
        self.form = form
            
    def feval(self, x, A, k, C):
        """
        Evaluate the fit function over x.

        Arguments:
            x (ndarray): Array of x values in the fitting range.
            A (float): Amplitude.
            k (float): Exponential rate constant.
            C (float): Constant baseline.
        
        Returns:
            ndarray: Array containing the fit values over the range.
        """
        
        return A*np.exp(k*(x-x[0])) + C

    def set_initial_parameters(self, y, params):
        """
        Set initial parameter values. Guess at the amplitude and baseline using
        the defined fit range if no parameters are provided on the fit panel. 
        If no decay constant is provided, assume a tau of 20 microseconds from
        the config data.

        Arguments:
            y (list): y data values.
            params (list): Array of fit parameters.
        """

        if (params[0] != 0.0):
            self.A = params[0]
        else:
            self.A = max(y) - min(y)
            
        if (params[1] != 0.0):
            self.k = params[1]
            
        if (params[2] != 0.0):
            self.C = params[2]
        else:
            self.C = min(y)
            
    def start(self, x, y, params, axis, results):
        """
        Implementation of the fitting algorithm.

        Arguments:
            x (list): x data values.
            y (list): y data values.
            params (list): Array of fit parameters.
            axis (matplotlib axes): Axes for the plot.
            results (QTextEdit): Display widget for fit results.
        
        Returns:
            list of Line2D: List of lines representing the plotted fit data.
        """
        
        fitln = None

        self.set_initial_parameters(y, params)

        p_init = [self.A, self.k, self.C]
        popt, pcov = curve_fit(self.feval, x, y, p0=p_init, maxfev=5000)
        perr = np.sqrt(np.diag(pcov)) # Parameter sigma from covariance matrix.
        
        # Fit data and print the results:
        
        try:
            x_fit = np.linspace(x[0], x[-1], 10000)
            y_fit = self.feval(x_fit, *popt)
            
            fitln = axis.plot(x_fit, y_fit, 'r-')
            
            for i in range(len(popt)):
                s = "p[{}]: {:.6e} +/- {:.6e}".format(i, popt[i], perr[i])
                results.append(s)
                if i == (len(popt) - 1):
                    results.append("\n")
        except:
            pass
        
        return fitln        

class ExpFitBuilder:
    """Builder method for factory creation."""
    
    def __init__(self):
        """ExpFitBuilder class constructor."""
        
        self._instance = None

    def __call__(self, A=0, k=0, C=0, form=""):
        """
        Create an instance of the fit function if it does not exist and 
        return it to the caller. Arguments passed as **kwargs from factory.

        Arguments:
            A (float): Amplitude.
            k (float): Exponential rate constant.
            C (float): Constant baseline.
            form (str): Function formula.

        Returns:
            ExpFit: Instance of the fit class.
        """
       
        if not self._instance:
            self._instance = ExpFit(A, k, C, form)
            
        return self._instance
