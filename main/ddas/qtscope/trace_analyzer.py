import inspect
import math
import numpy as np

DEBUG = False

class TraceAnalyzer:
    """
    TraceAnalyzer
    
    Provides an interface for calculating filter (time, CFD, energy) output 
    for DDAS traces based on the channel DSP settings.

    Attributes:
        dsp_mgr (DSPManager): Manager for internal DSP and interface for 
                              XIA API read/write operations.
        trace (array): Single channel ADC trace.
        fast_filter (list): Fast filter output calcualted from the trace.
        cfd (list): CFD output calculated from the fast filter.
        slow_filter (list): Slow filter output calculated from the trace.
    """
    
    def __init__(self, mgr):
        """
        TraceAnalyzer constructor.
        
        Arguments:
            dsp_mgr (DSPManager): Manager for internal DSP and interface for 
                                  XIA API read/write operations.
        """

        self.dsp_mgr = mgr
        self.trace = None
        self.fast_filter = None
        self.cfd = None
        self.slow_filter = None

    def analyze(self, mod, chan, trace):
        """
        Call other analyzers to calculate filter output.

        Arguments:
            mod (int): Module number.
            chan (int): Channel number.
            trace (array): Single channel ADC trace.

        Raises:
            All encountered exceptions are raised to the caller.
        """
            
        self.trace = trace
        
        try:
            self._compute_filters(mod, chan)
        except:
            raise

    #
    # Private methods
    #
    
    def _compute_filters(self, mod, chan):
        """
        Compute fast, CFD, and slow filter output.

        Arguments:
            mod (int): Module number.
            chan (int): Channel number.

        Raises:
            ValueError: If the stored trace is empty.
        """
        
        if not self.trace:
            raise ValueError("Trace is empty, cannot compute filters")
        
        # Load DSP needed to calculate filters:

        xdt = self.dsp_mgr.get_chan_par(mod, chan, "XDT")
        fast_risetime = self.dsp_mgr.get_chan_par(
            mod, chan, "TRIGGER_RISETIME"
        )
        fast_gap = self.dsp_mgr.get_chan_par(mod, chan, "TRIGGER_FLATTOP")
        cfd_scale = self.dsp_mgr.get_chan_par(mod, chan, "CFDScale")
        cfd_delay = self.dsp_mgr.get_chan_par(mod, chan, "CFDDelay")
        slow_risetime = self.dsp_mgr.get_chan_par(mod, chan, "ENERGY_RISETIME")
        slow_gap = self.dsp_mgr.get_chan_par(mod, chan, "ENERGY_FLATTOP")
        tau = self.dsp_mgr.get_chan_par(mod, chan, "TAU")
        
        if DEBUG:
            ns = 1000
            print("{}.{}: Filter parameters\n\t XDT (ns): {:.0f}\n\t Trig. risetime (ns): {:.0f}\n\t Trig. gap (ns): {:.0f}\n\t CFD scale: {:.0f}\n\t CFD delay (ns): {:.0f}\n\t Ene. risetime (ns): {:.0f}\n\t Ene. gap (ns): {:.0f}\n\t Tau (ns): {:.0f}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, xdt*ns, fast_risetime*ns, fast_gap*ns, cfd_scale, cfd_delay*ns, slow_risetime*ns, slow_gap*ns, tau*ns))

        # Since we're stuck with XDT binning, round the filter parameters to
        # the nearest integer multiple of the XDT value to convert to length
        # in samples. Because channel DSP paramters are double we must convert
        # explicitly to integers. Minimum of 1 sample for filter risetime and
        # CFD delay. Triangular filters (gap = 0 samples) are allowed.
        
        if fast_risetime < xdt:
            fast_risetime = int(1)
        else:
            fast_risetime = int(round(fast_risetime/xdt))
        fast_gap = int(round(fast_gap/xdt))
        if cfd_delay < xdt:
            cfd_delay = int(1)
        else:
            cfd_delay = int(round(cfd_delay/xdt))
        if slow_risetime < xdt:
            slow_risetime = int(1)
        else:
            slow_risetime = int(round(slow_risetime/xdt))
        slow_gap = int(round(slow_gap/xdt))
        if tau < xdt:
            tau = int(1)
        else:
            tau = int(round(tau/xdt))

        ns = xdt*1000 # Convert from samples to time in ns.
        print("{}.{}: Filter calculation requires parameters to be an integer multiple of XDT.\nParameters haave not been changed for acquisition.\n\t XDT (ns): {:.0f}\n\t Trig. risetime (ns): {:.0f}\n\t Trig. gap (ns): {:.0f}\n\t CFD scale: {:.0f}\n\t CFD delay (ns): {:.0f}\n\t Ene. risetime (ns): {:.0f}\n\t Ene. gap (ns): {:.0f}\n\t Tau (ns): {:.0f}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, ns, fast_risetime*ns, fast_gap*ns, cfd_scale, cfd_delay*ns, slow_risetime*ns, slow_gap*ns, tau*ns))
            
        if DEBUG:
            print("{}.{}: Calculating fast filter output for trace from Mod. {} Ch. {}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, module, channel))
        self._compute_fast_filter(fast_risetime, fast_gap)
        
        if DEBUG:
            print("{}.{}: Calculating CFD output for fast filter output from Mod. {} Ch. {}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, module, channel))
        self._compute_cfd(cfd_scale, cfd_delay)

        if DEBUG:
            print("{}.{}: Calculating slow filter output for trace from Mod. {} Ch. {}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, module, channel))
        self._compute_slow_filter(slow_risetime, slow_gap, tau)
        
    def _compute_fast_filter(self, risetime, gap):
        """
        Compute the fast filter output for a single channel ADC trace.
        
        Arguments:
            risetime (int): Fast filter risetime in samples.
            gap (int): Fast filter gap in samples.
        """
        
        self.fast_filter = [0] * len(self.trace)
        
        # Calculate fast filter. See Pixie-16 User's Manual Sec. 3.3.8.1,
        # Eq. 3-1 for details.
       
        #for i in range(len(self.fast_filter)):
        for i, _ in enumerate(self.fast_filter):
            s0 = 0 # Trailing sum.
            s1 = 0 # Leading sum.
            ilow = i - 2*risetime - gap + 1
            ihigh = ilow + risetime
            if ilow >= 0:
                s0  = sum(self.trace[ilow:ihigh])
                    
                # If the trailing sum is computed, compute the leading sum if
                # it does not run off the end of the trace:
                
                ilow = i - risetime + 1
                ihigh = ilow + risetime
                if ihigh < len(self.trace):
                    s1  = sum(self.trace[ilow:ihigh])
                        
                    # Compute the filter value if we have not run off the end
                    # of the trace for the leading sum:
                    
                    self.fast_filter[i] = s1 - s0
                    
    def _compute_cfd(self, scale, delay):
        """
        Compute the CFD from the fast filter output of a single channel ADC 
        trace.

        Arguments:
            scale (int): CFD scale (fraction subtraced is 1 - scale/8).
            delay (int): CFD delay in sample number.
        """
        
        self.cfd = [0] * len(self.fast_filter)

        # Compute the CFD. See Pixie-16 User's Manual Sec. 3.3.8.1,
        # Eq. 3-2 for details:
        
        for i, _ in enumerate(self.cfd):
            if (i + delay) < len(self.fast_filter):
                self.cfd[i + delay] = self.fast_filter[i + delay]*(1 - 0.125*scale) - self.fast_filter[i]

    def _compute_slow_filter(self, risetime, gap, tau):
        """
        Compute the slow filter output of a single channel ADC trace.

        For more information on the slow filter calcualtion, see H. Tan et al.,
        "A Fast Digital Filter Algorithm for Gamma-Ray Spectroscopy With 
        Double-Exponential Decaying Scintillators," IEEE T. Nucl. Sci. 51 1541 
        (2004).

        Arguments:
            risetime (int): Dlow filter risetime in samples.
            gap (int): Slow filter gap in samples.
            tau (int): Tau in samples for pole zero correction.
        """
        
        self.slow_filter = [0] * len(self.trace)

        # Guess a baseline value by averaging 5 samples at the start and end
        # of the trace and taking the minimum value:
        
        baseline = min(np.mean(self.trace[:5]), np.mean(self.trace[-5:]))
        
        if DEBUG:
            print("{}.{}: Estimated baseline {}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name,baseline))

        # Using notation from Tan unless otherwise noted, with time in samples:
        
        b1 = math.exp(-1/tau) # Constant ratio for geometric series sum Eq. 1.
        bL = math.pow(b1, risetime)
        
        # Coefficients of the inverse matrix Eq. 2 (example matrix elements
        # given on the bottom of pg. 1542):
        
        a0 = bL/(bL - 1)
        ag = 1
        a1 = 1/(1 - bL)

        if DEBUG:
            print("{}.{}: Ratio {:.3f}, coefficients {:.3f} {:.3f} {:.3f}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, b1, a0, ag, a1))

        for i, _ in enumerate(self.trace):
            s0 = 0 # Trailing sum.
            sg = 0 # Gap sum.
            s1 = 0 # Leading sum.
            ilow = i - 2*risetime - gap + 1
            ihigh = ilow + risetime
            
            if ilow >= 0:
                s0 = sum(self.trace[ilow:ihigh]-baseline)
                    
                # If the trailing sum is computed, compute the gap and leading
                # sums if they do not run off the end of the trace:
                
                ilow = ihigh
                ihigh = ilow + gap                
                if ihigh < len(self.trace):
                    sg = sum(self.trace[ilow:ihigh] - baseline)
                    
                ilow = ihigh
                ihigh = ilow + risetime
                if ihigh < len(self.trace):
                    s1 = sum(self.trace[ilow:ihigh] - baseline)
                        
                    # Compute the filter value if we have not run off the end
                    # of the trace for the leading sum:
                    
                    self.slow_filter[i] = a0*s0 + ag*sg + a1*s1

                if DEBUG and i == len(self.trace)/2:
                    print("{}.{}: Sums {:.1f} {:.1f} {:.1f} filter {:.1f}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, s0, sg, s1, self.slow_filter[i]))
