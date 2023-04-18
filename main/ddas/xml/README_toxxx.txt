/*!

\page toxxx
\author Ron Fox
\date 3/18/2020

\section overview Overview

This section documents three programs: toxml,  tocrate and tosetfile

-  toxml can take DDAS DSP settings from several source types and write them
   back out in xml.
-  tocrate can take DDAS DSP settings in Set files or in Crate XML files
   and send them to modules in a crate.  Optionally those modules can be booted.
   
For more on the XML file format see: \ref dsp_xml

\subsection toxml The toxml program.

toxml can take settings from XIA setfiles and from live modules and writes
them to XML files.  The advantage of XML files is that they store settings in
meaningful units of measure while Set files and the module, internally, store
settings in module type dependent units.

Here is the Help text for the toxml program:

<pre>
ToXml 1.0

Usage: ToXml [OPTIONS]...

  -h, --help          Print help and exit
  -V, --version       Print version and exit
  -s, --source=ENUM   Where the data to write the XML comes from  (possible
                        values="setfile", "modules")
  -c, --crate=STRING  XML Crate file that describes where module data are
                        written
  -f, --file=STRING   Setfile with the data (valid only if --source=setfile)
  -m, --msps=STRING   Only used with setfile: speeds of the modules form
                      slot:MHz
</pre>

-   --source determines where the settings data comes from.  This can be either
    the literal text "setfile" to take the settings from an XIA setfile or
    "modules" to get the settings from a module.
-   --crate specifies a crate XML file.  This will be used to determine
    which slots are read, the crate id, and where each module's XML file will
    be written.  The crate XML file will be rewritten, so it must be writable,
    the module XML files must also be writeable.
-   --file is only allowed if --source=setfile  the value of this option is
    the path to the setfile.  The setfile only has to be readable.
-   --mspsis only allowed if --source=sefile.  In order to convert the values in
    a setfile to the meaningful units used in module XML files, the speed of the
    digitizer in MHz must be known.  The default speed for each slot is set
    to 250.  YOu can use --msps to override this on a slot by slot basis. Each
    occurance of --msps defines the speed of one physical slot.  The value of
    this flag is specified in the form slot:speed  where slot is the physical
    slot number and speed the digitizer speed in MHz

<pre>
$DDAS_BIN/toxml --source=modules --crate=crate.xml
</pre>

This example takes settings from the slots described in crate.xml and rewrites
both crate.xml and the module settings file specified by the slot tags in it.

<pre>
$DDAS_BIN/toxml --source=setfile --crate=crate.xml --file=crate_1.set \
   --msps=2:500 --msps=5:100
</pre>

Converts a set file (crate_1.set) into XML.  The slots converted are defined
by the slot gats in crate.xml, which also says where the module parameter
XML files are written.  Slot 2 is defined to be a 500MHz module and slot 5 a 100MHz
module.  All other modules are 250MHz (the default speed).

\subsection tocrate The tocrate program

tocrate takes either XML or setfile data and loads the parameters in those files
to the digitizers in specified slots.  Optionally, slots can be fully booted.
Here's the help text for tocrate:

<pre>
tocrate 1.0

Usage: tocrate [OPTIONS]...

  -h, --help         Print help and exit
  -V, --version      Print version and exit
  -s, --source=ENUM  Where the data to load modules comes from  (possible
                       values="setfile", "xml")
  -f, --file=STRING  Set file or XML crate file depending on source
  -b, --fullboot     Perform a full boot of te modules before loading them
                       (default=on)
  -c, --crate=SHORT  ID of crate - only use this if source=setfile
  -S, --slot=INT     A slot to load - use as many times as there are slots
  -m, --msps=STRING  Speed specification of a digitizer (slot:mhz) default is
                     250MHz
</pre>

-   --source specifies where the settings come from.  --source=setfile
    indicate an XIA set file will provide the settings.  --source=xml
    indicates a Crate XML file will be provided and that, in turn, will
    point to module settings.
-   --file provides either the Crate XML file (--source=xml) or the
    set file (--settings=setfile) from which settings are taken.
-   --fullboot - if provided, prior to loading the parameters, each module
    has firmare loaded and its FPGAs and DSP started.
-   --crate only legal for --source=setfile provides the crateid which will
    be programmed into each module.
-   --slot only legal if --source=setfile.  Provides, in order the slot numbers
    that will be loaded.  The order determines the module id assignment.
    If more than one slot is used, supply this parameter more than once.
-   --msps only legal if --source=setfile.  Provides speed overrides for slots.
    Given that modules are loaded using Pixie16WriteSglModPar and Pixie16WriteSglChanPar,
    the speed of each digitizer must be known to convert setfile values to
    the units used by these functions.  By default, slots are assigned a speed
    of 250MHz.  Each occurance of --msps overrides the default speed.  The
    argument of --msps is of the form slot:speed where slot is the slot number
    and the speed is the digitizer MHz.
    
Here are some examples:

<pre>
 $DDAS_BIN/tocrate --source=xml --file=crate.xml
</pre>

The slots described in crate.xml are loaded with the parameters in module XML
files pointed to by their slot tags. The modules are assumed to already have
their firmware loaded and running.  adding the--fullboot flag to this line
will boot each module prior to loading it.

<pre>
 $DDAS_BIN/tocrate --source=setfile --file=crate_1.set --crate=0 \
   --slot=2 --slot=3 --slot=4 --slot=5 --slot=6                 \
   --msps=2:500 --msps=5:100   --fullboot 
</pre>

Loads slots 2-6 from the setfile crate_1.set.  Slot 2 is specified to be 500MHz,
slot 5 is specified to be 100Mhz, and all other modules are 250MHz. 
All modules are fully booted before being
loaded.

Note that when the setfile was written, there must have been a 500Mhz module in
slot2, a 100Mhz module in slot 5 and 250MHz modules in slots 3,4,6.

The load will require about 20 seconds per module regardless of the source type.
This is due to the time required to call Pixie16WriteSglModPar
and Pixie16WriteSglChanPar for each parameter that must be loaded.

\subsection tocsetfile The tosetfile program

As we've seen tocrate, run directly from an XML file takes approximately 20
seconds per module to load the configuration.  For large systems, this
can try the user's patience.  Therefore, the program tosetfile was written to
convert an XML crate specification into a setfile that can be loaded more
rapidly.

Here's the help text for tosetfile:

<pre>
ToSetfile 1.0

Usage: ToSetfile [OPTIONS]...

  -h, --help            Print help and exit
  -V, --version         Print version and exit
  -x, --xml=STRING      Path to XML Crate definition file
  -s, --setfile=STRING  Path to output Set file
  -m, --msps=STRING     Slot speeds each in the form slot:speed defaults are
                          250MHz

</pre>

As with all of these utilities, the default module speed is 250MHz.  You can
override this on a per slot basis by specifying the--msps option one or more
times.  The value of this option is a string of the form slot:MHz where slot is
the crate slot number and MHz the speed of the module in megahertz.  Here's an
example:

<pre>
$DDAS_BIN/tosetfile --xml=crate.xml --setfile=crate.set --msps=2:500
</pre>

This translates the XML crate specification rooted in crate.xml to a setfile
named crate.set.  Slot 2 will be a digitizer that runs at 500MHz. All other
digitizers default to 250MHz..

*/
