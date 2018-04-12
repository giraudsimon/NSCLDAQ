INITIAL STEPS TO SET UP A FUNCTIONAL READOUT PROGRAM
----------------------------------------------------

1. Configure the envset file.

    Ensure that...

    - the EVENTS variable contains the  
      correct path to store the data

    - the DAQHOST is correct for this 
      experiment

    - RDOFILE contains the absolute path
      to the Readout executable to be used in this 
      experiment.
      
      NOTE: Running make in the readout directory 
      will create a new executable in the 
      readout directory. This new executable will 
      need to be moved or copied to the location 
      referred to by RDOFILE.

2. Configure the crate_NUM directory 
   
    A crate_1 directory exists by default and
    can be used to model other crate_NUM directories. 

    Ensure that...

    - crate_1/cfgPixie16.txt contains the appropriate 
      info.
 
         * Specify the absolute path to the set file 
           will need to modified.
 
         * Specify the number and location of modules.

    - modevtlen.txt is correct. 

    - pxisys.ini exists and is correct.


3. Choose NSCLDAQ version

    By default, this software builds against whatever
    version of the NSCLDAQ exists in the /usr/opt/daq/current
    directory. Typically there are a few choices and the
    user should ensure before using this software that 
    they are utilizing the desired version of the nscldaq.

    IMPORTANT: There are multiple files to edit when doing this.
    Be sure to edit the following files:

    Makefile            : set INSTDIR variable
    ReadoutCallouts.tcl : set path on first line
    goreadout           : set path to ReadoutShell 
