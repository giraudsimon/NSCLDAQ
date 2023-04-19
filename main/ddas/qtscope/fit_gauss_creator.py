import pandas as pd
import numpy as np
from scipy.optimize import curve_fit

class GaussFit:
    """
    Gaussian fitting function: f(x) = A*exp(-(x-mu)^2 / (2*sd^2))

    Attributes:
        A (float): Amplitude.
        mu (float): Mean.
        sd (float): Standard deviation.
        form (str): Function formula.

    Methods:
        gauss(): Returns the array of function values evaluated over the 
                 fitting range.
        set_initial_parameters(): Set initial parameter values. Guess from 
                                  the data if none are provided.
        start(): Implementation of the fitting algorithm.
    """
    
    def __init__(self, A, mu, sd, form):
        """
        Gaussian fit function class constructor. Sets initial fit 
        parameters.
        
        Arguments:
            A (float): Amplitude.
            mu (float): Mean.
            sd (float): Standard deviation.
            form (str): Function formula.
        """
        
        self.A = A
        self.mu = mu
        self.sd = sd
        self.form = form

    def feval(self, x, A, mu, sd):
        """
        Evaluate the fit function over x.

        Arguments:
            x (ndarray): Array of x values in the fitting range.
            A (float): Amplitude.
            mu (float): Mean.
            sd (float): Standard deviation.
        
        Returns:
            ndarray: Array containing the fit values over the range.
        """
        
        return A*np.exp(-(x-mu)**2 / (2*sd**2))

    def set_initial_parameters(self, x, y, params):
        """
        Set initial parameter values. Guess at the amplitude, mean, and stddev 
        using the defined fit range if no parameters are provided on the fit 
        panel.

        Arguments:
            x (list): x data values.
            y (list): y data values.
            params (list): Array of fit parameters.
        """

        if (params[0] != 0.0):
            self.A = params[0]
        else:
            self.A = max(y)
            
        if (params[1] != 0.0):
            self.mu = params[1]
        else:
            self.mu = np.mean(x)
            
        if (params[2] != 0.0):
            self.sd = params[2]
        else:
            self.sd = np.std(x)

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

        self.set_initial_parameters(x, y, params)

        p_init = [self.A, self.mu, self.sd]
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

class GaussFitBuilder:
    """Builder method for factory creation."""
    
    def __init__(self):
        """GaussFitBuilder class constructor."""
        
        self._instance = None

    def __call__(self, A=0, mu=0, sd=0, form=""):
        """
        Create an instance of the fit function if it does not exist and 
        return it to the caller. Arguments passed as **kwargs from factory.

        Arguments:
            A (float): Amplitude.
            mu (float): Mean.
            sd (float): Standard deviation.
            form (str): Function formula.

        Returns:
            GaussFit: Instance of the fit class.
        """
       
        if not self._instance:
            self._instance = GaussFit(A, mu, sd, form)
            
        return self._instance
