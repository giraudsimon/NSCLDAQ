/*!
\page libDSPParameters
\author Ron Fox
\date  3/18/2020

\section dsp_overview Overview

\note This document applies to DDAS version 3.0-000 and later.

The libDSPParameters library provides a package for managing the 
Digital Signal Processing (DSP) Parameters  of Pixie16 boards.  It also
provides utilities for managing crates of modules.  Finally the \ref toxxx
package is built on top of this library.

To understand the library you need to understand the concept of Readers
and Writers.  A Reader gets the DSP Parameters for modules or crates from some
source.  That source might be the modules themselves, XIA .set files XML files
or whatever other source you might add to the system.

There are two Reader base classes:


-   DDAS::SettingsReader is the base class of a family of classes that read DSP
    settings for a single module.
-   DDAS::CrateReader is the base class of a family of classes that read DSP
  Parameters for a collection of modules.  Normally, these modules live in a
  single PXI crate.

Conversely, writers write DSP settings to some sink.  The sink might be the
module, a .set file or a XML file.  There are two writer base classes:

-   DDAS::SettingsWriter  is the base class of a family of classes that write
    DSP parameters for a single module.
-   DDAS::CrateWriter is the base class of a family of classes that write DSP
    parameters for a crate full of modules.


\subsection dsp_readers Readers

Readers take DSP parameters from some source and store them in an
internal representation.  The internal representations used are structs that
are defined  in the
ModuleSettings.h header.  There are two types of Readers. Readers that take
settings from a single module worth of stuff and Readers that take settings
from a Crate worth of stuff.  The abstract base classes for both of these
use the strategy pattern to provde the main logic in the base class while
providing source dependent code in derived, concrete classes.

Let's look at the module readers first.  Currently there are module readers
for:

-   Pixie16 modules that are live and in crate.
-   XIA set files.
-   XML files.

Let's have a look at the base class, DDAS::SettingsReader described in the
header SettingsReader.h :

<pre>

 namespace DDAS {
     class SettingsReader
     {
     public:
         virtual ~SettingsReader() {}
         virtual DDAS::ModuleSettings get() = 0; 
     };
 }


</pre>

SettingsReader depends on the concrete class to implement an appropriate
constructor and a get method that knows how to do the actual fetch.  For more
information on the concrete classes see:

-  DDAS::PixieSettingsReader which reads DSP settings from one Pixie16 module.
-  DDAS::SetFileReader which reads DSP settings from one module in a .set File.
-  DDAS::XmlSettingsReader which reads data from XML files  see \ref dsp_xml
-  for information about the form of these XML files.

\subsection dsp_cratereaders Crate Readers

Crate readers are intended to read a collection of modules.  The
DDAS::CrateReader base class uses a strategy pattern to provide the main logic
flow for reading a crate allowing the concrete subclasses to provide readers
for individual modules.  Here's the definition of DDAS::CrateReader from the
header CrateReader.h:

<pre>
 namespace DDAS {
    class CrateReader {
    protected:
        std::vector<unsigned short> m_slots;
        unsigned                    m_crateId;
    public:
        CrateReader() {}
        CrateReader(unsigned crate, const std::vector<unsigned short>& slots);
        virtual ~CrateReader();

    public:
        virtual Crate readCrate();
        virtual SettingsReader* createReader(unsigned short slot) = 0;
    };
 }

</pre>

Only public and protected members are shown.  m_slots is a vector of slot
numbers that must be rea and m_crateId is the crate id to be given to the
collection.  This crate id overrides the crate id from the modules themselves.
The pure virtual method:  createReader must produce a DDAS::SettingsReader for
the slot number provided.  Note this is a slot number not a module id.

Concrete subclasses include:

- DDAS::PixieCrateReader (defined in PixieCrateReader.h) which reads data from
  a PXI crate.
- DDAS::SetFileCrateReader (defined in SetFileCratereader.h) which reads data
  from a Set file.
- DDAS::XMLCrateReader (defined in XMLCrateReader.h) which reads data from an
  XML file.  See \ref dsp_xml for information about the format of these XML
  files. 

\subsection dsp_writers Writers


The concept of Writers mirrors that of Readers.  There is the
DDAS::SettingsWriter class which writes a module's worth of data and
DDAS::CrateWriter which writes a PXI crate worth of data.

Here's the class definition of DDAS::SettingsWriter from SettingsWriter.h

<pre>

 namespace DDAS {
 
    class SettingsWriter
    {
    public:
        virtual ~SettingsWriter() {}
        virtual void write(const ModuleSettings& dspSettings) = 0;
    };
 
 }         
</pre>

Constructing a concrete DDAS::SettingsWriter is supposed to connect it with its
sink.  The write method then writes DDAS::ModuleSettings to that module.  This
method is pure virtual and must be implemented in concrete classes. These
include:

-   DDAS::PIxieCrateWriter (PixieCrateWriter.h) which writes to a module in a
    PXI crate.
-   DDAS::XMLSettingsWriter (XMLSettingsWriter.h) which writes to an XML file
    whose format is described in \ref dsp_xml
-   DDAS::SetFileWriter (SetFileWriter.h) which writes XIA bulk settings files.

Similarly the DDAS::CrateWriter writes a collection of modules in a PXI crate.
Modules can be selectively loaded.  Here's the definition of DDAS::CrateWriter
from CrateWriter.h:

<pre>

 namespace DDAS {
    class SettingsWriter;
    
    class CrateWriter {
    protected:
        Crate                       m_settings;
    public:
        CrateWriter(const Crate& settings);
        virtual ~CrateWriter() {}
   
    public:
        virtual void write();

        virtual void startCrate(
            int id, const std::vector<unsigned short>& slots
        )  = 0;
        virtual void endCrate(
            int id, const std::vector<unsigned short>& slots
        )    = 0;
        virtual SettingsWriter* getWriter(unsigned short slotNum) = 0;

    };
 }

</pre>

The m_settings attribute are the settings that will be loaded.  Three pure
virtual methods must be implemented by any concrete crate write:

-   startCrate - this is called prior to writing any slots and is used to
    perform any initialization.  For example, the Pixie crate writer
    initializes the XIA API library and performs a few other housekeeping
    operations.  The id argument is the crate id and slots the vector of slot
    numbers provided in the settings.
-   endCrate called after all slots have been written and provides a chance for
    the concrete class to cleanup.  As before, id is the crate id and slots are
    the vector of slot numbers.
-    getWriter provides a DDAS::SettingsWriter for the specified slot number
    (not Module).  The caller will delete this object so it must be dynamically
    instantiated.


Currently the following concrete DDAS::CrateWriter subclasses have been
supplied:

-   DDAS::PixieCrateWriter (PixieCrateWriter.h) writes to a physical crate full
    of Pixie16 modules.
-   DDAS::XMLCrateWriter writes a set of XML files whose format is described in
    \ref dsp_xml
-   DDAS::SetFileCrateWriter writes a set file that can be loaded using e.g. the
    XIA APi function Pixie16LoadDSPParametersFromFile (or a boot with bit 4 set
    in the BootPattern argument)


\subsection dsp_other Other support functions

The library contains a other useful functions and classes.

\subsubsection dsp_setfile Set file reader

The header SetFile.h provides definitions that allow you to read set files. These
are presented as a static methods for a class in the DDAS namespace.  A set file
is essentially a soup of uint32_t values (with one exception).  The header
defines this class:

<pre>
 namespace DDAS {
    class SetFile {
    public:
        // Var definition files:

        static VarOffsetArray readVarFile(const char* filename);
        static VarMapByOffset createVarOffsetMap(const VarOffsetArray& offsets);
        static VarMapByName   createVarNameMap(const VarOffsetArray& offsets);

        // set files I/O and resource management:

        static std::pair<unsigned, uint32_t*> readSetFile(const char* filename);
        static void writeSetFile(
            const char* filename, unsigned nLongs, uint32_t* pVars
        );
        static void freeSetFile(uint32_t* pVars);


        static UnpackedSetFile populateSetFileArray(
            unsigned nLongs, const uint32_t* pVars,
            const VarOffsetArray& map
        );
        static SetFileByName populateSetFileMap(
            unsigned nLong, const  uint32_t* pVars,
            const VarOffsetArray& map
        );
        static SetFileByName populateSetFileMap(
            const UnpackedSetFile& vars
        );
   };
}
</pre>

The class documentation will provide documentation for each of these methods,
however a few important points:

-  Each set file is fixed size and contains the DSP parameters for 24 modules.
   There's  no way to know which of those modules contain actual data and which
	 are just default values other than knowing what was in the PXI crate when the
	 file was written
-	 The XIA header pixie16app_defs.h provides the constant
	 definition N_DSP_PAR.  That's the number of uint32_t parameters that make
	 up each module.
-  Associated with each module type is a variable file.  This describes the offsets
   of each parameter by name in the file.  Unfortunately the set file does not
	 tell you which type of module each slot of the set file is.  You have to know
	 what was in the crate when the set file was written. This is, in fact, the
	 motivation for creating an XML representation of the DSP settings which are
	 independent of module type.
-  The DDAS::CrateManager class which we will document next provides a static
   method to return the path to the variable file given the speed of a module.	 
-  DDAS::SetFile::readSetFile returns the size of the file and a pointer to the first of the
   uint32_t's in the file.  The storage must be freed by calling
	 DDAS::SetFile::freeSetFile when you are done with it.
-  The PreampTau DSP Parameter is actually an IEEE float32 not a uint32_t  simply
   moving the bits in this value into a float will do that conversion.  See
	 the  code that extracts these values in DDAS::SetFileReader
-  Saving the best for last, you really could just use SetFileReder to populate
   a DDAS::ModuleSettings
   struct (see ModuleSettings.h) rather than call this set of functions directly.
	 
Normally this set of methods is used as follows:

-  Read the variable files for the modules.
-  Read the set file.
-  Iterate over the module slots and use the populate methods to extract the
   set file variables into one of the structures that associates the
	 DSP parameters
	 with the DSP values.

\subsubsection dsp_cratemanager Crate manager class

The DDAS::CrateManager class provides a class that can manage a PXI crate that
has Pixie16 digitizers.  Here are the important featurs of that class (defined in
CrateManager.h).  See the class documentation for more:

<pre>
namespace DDAS {
    class CrateManager {
        DAQ::DDAS::Configuration*    m_pConfiguration;
        std::vector<unsigned short>  m_slots;
    public:
        CrateManager(const std::vector<unsigned short>& slots);
        virtual ~CrateManager();

        // Slot translation

        unsigned short moduleId(unsigned short slot);
        unsigned short  slot(unsigned short id);

        // Configuration:

        void loadDSPAddressMap(unsigned short id);
        void fullBoot(unsigned short id);
        void fullBoot(unsigned short id, const char* setFile);
        static std::string getVarFile(unsigned short speed);
   };
}
</pre>

A few notes:

-   The slots vector passed into the contstructor provides the set of crate
    slot numbers that are populated with modules you care about.
-   The index into the slot vector for a module is it's module id.  This module
    id is used in most XIA API calls.  the moduleId and slot methods  translate
		slots to module ids and module ids back to slots respectively.
-   The first fullBoot loads all of the firmware but does not load the DSP
    parmaeters.  The second version loads the DSP parameters given a Set file.
-   The getVarFile method is static so given the speed of a module in Mhz
    you can retrieve the path to that module type's variable definition file

Example of getVarFile and the Setfile class:

<pre>
#include <CrateManager.h>
#include <SetFIle.h>

...
// I have a 250MHz module in slots 2 and 3 (ids 0,1)
// I want to read the set file parameters for both using the raw set file
// reader.  I'll put those into a map keyed by parameter name.

	std::string varFile       = DDAS::CrateManager::getVarFile(250);
	DDAS::VarOffsetArray vars = DDAS::SetFile::readVarFile(varFile.c_str());
  std::pair<unsigned, uint32_t*> sfDesc = DDAs::SetFile::readSetFile("mfile.set");
	uint32_t* p = sfDesc.second;
	unsigned  s = sfDesc.first;
	
	DDAS::SetFileByName slot2 =
		DDAS::SetFile::populateSetFileMap(s, p, vars);
		
	p += N_DSP_PAR;
	s -= N_DSP_PAR;
	
	DDAS::SetFileByName slot3 =
		DDAS::SetFile::populateSetFileMap(s, p, vars);
	
	DDAS::SetFile::freeSetFile(sfDesc.second);
	
	...
	
</pre>

\subsection dsp_xml XML used to describe DSP settings.

A new innovation with this library is the ability to store DSP settings in
XML files that use units that are independent of the digitizers used.
This allows you to:

-	  Create settings files that capture the characteristics of the
	  signals you are using and to use them regardless of the actual digitizers
	  you are using.
-   Create settings files with the granularity of single modules.  .set files
    capture the settings of a full crate of modules.
-   Create settings files that you can read.

What makes this work is that:

-    Module XML files describe the settings for a single module in real units
     rather than module dependent units.
-    Crate XML files stitch togetherm module XML files into the settings needed
     for a single crate of digitizers.
		 
All of this comes at a cost, however and that is load speed.  It takes about
20 seconds to load a module from its XML settings.  This is dominated by the
time to call the Pixie16WriteSglModPar and Pixie16WriteSglChanPar functions
in the XIA API.  XML file parsing is actually quite quick.

This section documents the form of the XML files that describe crates and
modules.

First a bit about XML, in case you are not familiar with. XML is an acronym
for eXtensible Markup Language.  You can think of it as a descendent of
HTML (HyperText Markup Language), the language used to specify web pages.
An XML file also called a document, consists of a header, and tags.

Tags are
of the form <tag>body</tag>  where the body can contain other tags or text
depending on what the document represents.  In XML there is normally a single
outer tag that is documents look like

<pre>
<roottag>
   other stuff
</roottag>
</pre>

Rather than

<pre>
<tag1>
   nested stuff
</tag1>
<tag2>
   nested stuff
</tag2>
</pre>

Tags can also have _attributes_  an attribute is a string of the form
key="value" or key='value' inside the tag specification. For example:

<pre>
...
   <sometag attr1="value1" attr2="value2">
	    contents.
	 </sometag>
...
</pre>

Finally, if a tag has no contents it can be expressed as:
<tag />.  Such tags can also have attributes.

With very superficial introduction to XML document structure, let's look at the
structure of the two XML document types we have defined, crates and modules

\subsection dsp_cratexml  Crate XML files

The purpose of a crate file is to specify which slots are occupied and
the properties of each module in those occupied slots.  Here's a sample
crate definition file that describes a crate with one module:

<pre>
<crate id="1">
  <slot number="2" evtlen="4" configfile="bimdev_Module_00.xml" />
</crate>
</pre>

As you can see there are two tags.  The root tag is <crate> and it can contain
as many <slot> tags are required  The <crate> tag has one mandatory attribute,
id which is the crate id that will be assigned to each module of that crate.

<slot> tags are empty tags.  Slot tags have several mandatory attributes as
well as some optional ones.  The mandatory attributes are:

-   number - the slot number being described.
-   evtlen - the length of hits expected from this module.
-   configfile  - The path to the module XML file for this slot.

The optional attributes and their default values are:

-   fifo_threshold  - the FIFO occupancy threshold that will make this slot
                      contribute to an event trigger.  When using XML
											descriptions, this is a per slot value rather than a global
											value allowing slow and fast slots each to have values that
											make sense.  This defaults to 102400
-   infinity_clock  - If true, the clock for this module is not reset after each
                      begin.  This defaults to false.  This value should be the
											same for all slots in the crate.  In a future version this
											attribute might get added to the <crate> tag.
-		external_clock  - If true, the timestamp is taken from the external clock
                      counter.  This defaults to false.  Note that this must be
											enabled in the module's CSRs and should be the same across
											the entire system (all modules in all crates).  In a future
											version, this may get added to the <crate> tag.
-		timestamp_scale - A floating point multiplier of the timestamp for this module.
                      This defaults to 1.0   It is normally used to match the timestamp
											frequency with external devices for event building.  Note
											that the hit's timestamps won't be altered, only timestamps
											passed to the event builder in the hit's body header.

\subsubsection dsp_xmlmodule Module XML files

Module XML files contain the settings for a single module.  Depending on how
they are written there may be a metadata tag in the front of the module XML
file.  Metadata tags are intended to give XML file processors clues about how
the file should be processed.  The metadata tag will look like

<pre>
<?xml version="1.0"?>
</pre>

and just indicates this file is compliant with the version 1.0 definition of XML.

A module file will look like (only one channel's settings are shown for brevity):

<pre>
<Module>
    <csra value="1"/>
    <csrb value="81"/>
    <format value="0"/>
    <maxevents value="0"/>
    <synchwait value="false"/>
    <insynch value="true"/>
    <SlowFilterRange value="3"/>
    <FastFilterRange value="0"/>
    <BackplaneTriggerEnables value="0"/>
    <crateID value="1"/>
    <slotID value="2"/>
    <moduleId value="0"/>
    <trigConfig0 value="0"/>
    <trigConfig1 value="0"/>
    <trigConfig2 value="0"/>
    <trigConfig3 value="0"/>
    <HostRTPreset value="1203982208"/>
    <channel id="0">
        <TriggerRiseTime units="microseconds" value="0.4"/>
        <TriggerFlatTop units="microseconds" value="0.08"/>
        <TriggerThreshold units="adccounts" value="65"/>
        <EnergyRiseTime units="microseconds" value="4.8"/>
        <EnergyFlatTop units="microseconds" value="0.384"/>
        <Tau units="microseconds" value="50"/>
        <TraceLength units="microseconds" value="6"/>
        <TraceDelay units="microseconds" value="2"/>
        <VOffset units="volts" value="-0.283035"/>
        <XDT units="microseconds" value="0.06"/>
        <Baseline units="percent" value="10"/>
        <EMin units="none" value="0"/>
        <BinFactor units="none" value="1"/>
        <BaselineAverage units="none" value="4"/>
        <CSRA units="bitmask" value="36"/>
        <CSRB units="bitmask" value="0"/>
        <BlCut units="none" value="4"/>
        <Integrator units="none" value="0"/>
        <FastTriggerBacklen units="microseconds" value="0.24"/>
        <CFDDelay units="microseconds" value="0.064"/>
        <CFDScale units="none" value="0"/>
        <CFDThresh units="none" value="120"/>
        <QDCLen0 units="microseconds" value="0.12"/>
        <QDCLen1 units="microseconds" value="0.12"/>
        <QDCLen2 units="microseconds" value="0.004"/>
        <QDCLen3 units="microseconds" value="0.12"/>
        <QDCLen4 units="microseconds" value="0.004"/>
        <QDCLen5 units="microseconds" value="0.12"/>
        <QDCLen6 units="microseconds" value="0.004"/>
        <QDCLen7 units="microseconds" value="0.12"/>
        <ExtTrigStretch units="microseconds" value="0.8"/>
        <VetoStretch units="microseconds" value="0.24"/>
        <MultiplicityMasks low="0" high="0"/>
        <ExternDelayLen units="microseconds" value="0.64"/>
        <FTrigoutDelay units="microseconds" value="0.08"/>
        <ChanTrigStretch units="microseconds" value="0.8"/>
    </channel>
    <channel id="1">
		....
		  </channel>
</Module>
</pre>

The root tag is <Module>  Directly beneath it are tags for module level parameters.
After the module level parameters is one <channel> tag for each channel in the
module.  The <channel> id attribute is the channel number.  Inside each
<channel> tag are tags for the DSP parameters for the channel.

Parameter tags all have the same general form. The tag name (eg. csra) identifies
the parameter and the mandatory value attribute provides the value for that parameter.
The units attribute, which is optional but provided for documentation purposes
provides the units of measure of the parameter.  All module level parameters
are unitless and therefore the units attribute is not used.

Note that the units of measure shown _must_ be used.  You cannot, for example
specify units="nanoseconds" and provide a timing value in nanoseconds.  The
XML processor does not pay attention to the units attribute.  The units attribute
is documentation to tell you, if you edit the file by hand, the units you
must specify the parameter value in.




*/