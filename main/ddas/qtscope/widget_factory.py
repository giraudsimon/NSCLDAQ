DEBUG = True

class WidgetFactory:
    """
    Factory for creating GUI widgets.

    Attributes:
        builders (dict): Dictionary of builder methods for concrete classes.
    
    Methods:
        register_builder(): Register a builder by key name.
        create(): Create a widget using its builder.
    """
    
    def __init__(self):
        """WidgetFactory class constructor."""
        
        self.builders = {}

    def register_builder(self, key, builder):
        """
        Register a builder with a key.

        Arguments:
            key (str): Key name for builders dictionary.
            builder: Builder method for the concrete class.
        """        

        self.builders[key] = builder

        if DEBUG:
            print("  Registered: {}".format(key))

    def create(self, key, *args, **kwargs):
        """
        Create an instance of a widget from its key. Pass parameters to the 
        builder method using *args and **kwargs.

        Arguments:
            key (str): Get the builder method from the dictionary corresponding
                       to this key.

        Returns:
            widget: An instance of the widget class, if it exists.

        Raises:        
            ValueError: If the key doesn't correspond to a registered builder.
        """
        
        builder = self.builders.get(key)
        
        if not builder:
            raise ValueError(key)
        
        return builder(*args, **kwargs)

