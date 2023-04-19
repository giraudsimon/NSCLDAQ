import sys
import traceback

from PyQt5.QtCore import QObject, QRunnable, pyqtSignal, pyqtSlot

class WorkerSignals(QObject):
    """
    Defines the signals available from a running worker thread.

    Custom signals can only be defined on objects derived from QObject. Since 
    QRunnable is not derived from QObject we can't define the signals there 
    directly.

    Supported signals are:
        finished: No data.
        error: Tuple (exctype, value, traceback.format_exc()).
        result: Object data returned from processing, anything.
        progress: int indicating % progress.
    """
    
    running = pyqtSignal()
    finished = pyqtSignal()
    error = pyqtSignal(tuple)
    result = pyqtSignal(object)

class Worker(QRunnable):
    """
    Worker thread. Inherits from QRunnable to handler worker thread setup, 
    signals and wrap-up.

    Attributes:
        fn (QObject): Function attached to this worker 
                      thread.
        args (tuple): Arguments to pass to the callback function.
        kwargs (dict): Keyword arguments to pass to the 
                       callback function.
        signals (WorkerSignals): Signals emitted by the worker.

    Methods:
        run(): Executes the code we wish to run from the passed function fn.
        set_function(): Set function attribute.
    """

    def __init__(self, fn=None, *args, **kwargs):
        """
        Worker thread class constructor.

        Constructs a worker thread and callback function from a function 
        object and callback args/kwargs.

        Arguments:
            fn (QObject): Function attached to this worker 
                          thread.
            args (tuple): Arguments to pass to the callback function.
            kwargs (dict): Keyword arguments to pass to the 
                           callback function.
        """
        
        super().__init__()
        
        # Store constructor arguments (re-used for processing):
        
        self.fn = fn
        self.args = args
        self.kwargs = kwargs
        self.signals = WorkerSignals()

    @pyqtSlot()
    def run(self):
        """Initialise the runner function with passed args, kwargs."""
        
        # Retrieve args/kwargs here, process the function and emit signals:
        
        try:
            self.signals.running.emit()
            result = self.fn(*self.args, **self.kwargs)
        except:
            traceback.print_exc()
            exctype, value = sys.exc_info()[:2]
            self.signals.error.emit((exctype, value, traceback.format_exc()))
        else:
            self.signals.result.emit(result)
        finally:
            self.signals.finished.emit()

    def set_function(self, new_fn):
        """
        Set the function for this worker thread.

        Arguments:
            new_fn (QObject): Function to set.
        """
        
        self.fn = new_fn
