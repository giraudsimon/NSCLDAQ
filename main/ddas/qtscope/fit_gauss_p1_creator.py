import pandas as pd
import numpy as np
from scipy.optimize import curve_fit

class GaussP1Fit:
    """
    Gaussian fitting function with a linear background: 
    f(x) = A*exp(-(x-mu)^2 / (2*sd^2)) + a0 + a1*x

    Attributes:
        A (float): Amplitude.
        mu (float): Mean.
        sd (float): Standard deviation.
        a0 (float): Constant term of linear background.
        a1 (float): Slope of linear background.
        form (str): Function formula.

    Methods:
        gauss(): Returns the array of function values evaluated over the 
                 fitting range.
        set_initial_parameters(): Set initial parameter values. Guess from 
                                  the data if none are provided.
        start(): Implementation of the fitting algorithm.
    """
    
    def __init__(self, A, mu, sd, a0, a1, form):
        """
        Gaussian fit function with linear background class constructor. Sets 
        initial fit parameters.
        
        Arguments:
            A (float): Amplitude.
            mu (float): Mean.
            sd (float): Standard deviation.
            a0 (float): Constant term of linear background.
            a1 (float): Slope of linear background.
            form (str): Function formula.
        """
        
        self.A = A
        self.mu = mu
        self.sd = sd
        self.a0 = a0
        self.a1 = a1
        self.form = form

    def feval(self, x, A, mu, sd, a0, a1):
        """
        Evaluate the fit function over x.

        Arguments:
            x (ndarray): Array of x values in the fitting range.
            A (float): Amplitude.
            mu (float): Mean.
            sd (float): Standard deviation.
            a0 (float): Constant term of linear background.
            a1 (float): Slope of linear background.

        Returns:
            ndarray: Array containing the fit values over the range.
        """
        
        return A*np.exp(-(x-mu)**2 / (2*sd**2)) + a0 + a1*x

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

        if (params[3] != 0.0):
            self.a0 = params[3]
        else:
            self.a0 = min(y[0], y[-1])

        if (params[4] != 0.0):
            self.a1 = params[4]
        else:
            dy = y[-1] - y[0]
            dx = x[-1] - x[0]
            self.a1 = dy/dx 

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

        p_init = [self.A, self.mu, self.sd, self.a0, self.a1]
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

class GaussP1FitBuilder:
    """Builder method for factory creation."""
    
    def __init__(self):
        """GaussP1FitBuilder class constructor."""
        
        self._instance = None

    def __call__(self, A=0, mu=0, sd=0, a0=0, a1=0, form=""):
        """
        Create an instance of the fit function if it does not exist and 
        return it to the caller. Arguments passed as **kwargs from factory.

        Arguments:
            A (float): Amplitude.
            mu (float): Mean.
            sd (float): Standard deviation.
            a0 (float): Constant term of linear background.
            a1 (float): Slope of linear background.
            form (str): Function formula.

        Returns:
            GaussP1Fit: Instance of the fit class.
        """
       
        if not self._instance:
            self._instance = GaussP1Fit(A, mu, sd, a0, a1, form)
            
        return self._instance
