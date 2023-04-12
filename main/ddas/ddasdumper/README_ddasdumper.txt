/*!

\page The ddasdumper : Converting data for ROOT analyses
\author Jeromy Tompkins
\date April 21, 2016

\tableofcontents

\section ddasdumper Introduction

It is quite common for experimenters to repackage their data from raw event
file file formats to ROOT. The DDAS provides a tool to accomplish this called
ddasdumper. The ddasdumper processes an event file and generates a ROOT TTree.
The tree that is generated will have only a single branch whose name depends on
the command line options. We will discuss in this documentation how to use the
ddasdumper and its output format. We will also discuss basically how to process
the generated TTree.

\section ddasdumper_running_sec Running the program

The ddasdumper program is run in much the same way as any other program created
within the NSCLDAQ Filter Framework or the NSCLDAQ dumper. You can see the
various options for running it by passing the -h or \--help option. 

\verbatim
ddasdumper 1.1

Formatted dump of selected data items and conversion of ddas PHYSICS_EVENTS to
root trees.

Usage: ddasdumper [OPTIONS]...

  -h, --help            Print help and exit
  -V, --version         Print version and exit
  -s, --source=STRING   URL of source, ring buffer or file
  -m, --skip=INT        number of items to skip before dumping
  -c, --count=INT       Number of items to dump before exiting
  -S, --sample=STRING   List of item types to sample (deprecated)
  -e, --exclude=STRING  List of item types to exclude from the dump
  -f, --fileout=STRING  Path of output file
  -l, --legacy-mode     Legacy data format enabled. This is not a default
                          setting  (default=off) (deprecated use --format)
  -F, --format   Data format can be v10, v11 or v12.
\endverbatim

The key options here are \--source, \--fileout, and \--legacy-mode. The
\--fileout option will specify the path to the ROOT file that is outputted. The
--source option specifies where to read data from and its value should be in
the form of a URL ( protocol://host:port/path ).  The supported protocols are
"file" and "tcp" for reading from files or ringbuffers, respectively. For file
protocols, the host and port are ignored and left off the URL.  If you want to
read from a file at /path/to/my/evtfile.evt then you would run:

\verbatim
ddasdumper --source=file:///path/to/my/evtfile.evt --fileout=output.root
\endverbatim

If you would like to attach to a ringbuffer to process online data you would
need to identify the name of the ring buffer and the hostname of the computer.
Let's say you want to read data from a ring buffer named "myring" on host
"somecomputer", you would do that as follows:

\verbatim
ddasdumper --source=tcp://somecomputer/myring --fileout=output.root
\endverbatim

The \--legacy-mode option specifies whether the data is being from a point that
is or was downstream of an event builder.  If \--legacy-mode is set to on
(\--legacy-mode=on), then that implies the data is formatted as though it were
directly outputted from the Readout program.  Otherwise, the data is treated as
though it were built with the event builder.

The other options are described in the table below:

| Option          |  Description                                                     |
|-----------------|------------------------------------------------------------------|
| \--skip, -m      | The program will skip this number of ring items at the beginning |
| \--count, -c     | The program will process on this number of items before exiting  |
| \--sample, -S    | This is not particularly useful and should be ignored            |
| \--exclude, -e   | This is not particularly useful and should be ignored            |

\section ddasdumper_output_sec Understanding the Output File

The first thing to understand is that ddasdumper does not convert any ring
items besides physics events. The ddasdumper processes all of the ring items in
the file, but it does not output anything besides the physics event data to the
ROOT file. THe name of that ROOT file will be the same as was provided to the \--fileout
option.

The ROOT file will contain a tree named "dchan". The structure of the tree will be different
depending on whether the \--legacy-mode option was passed. If it was passed, then the tree
will have a single branch called "dchan" that consists of ddaschannel objects. If the 
\--legacy-mode option was not passed, then the tree will have a single branch consisting
of DDASEvent objects. Both the ddaschannel and DDASEvent classes documented and can be
used in user code.


\section ddasdumper_usingoutput_sec Attaching to and Processing the TTree

For completeness, we will discuss how to process the data in the TTree. A
complete discussion of processing TTrees is beyond the scope of this document
and NSCLDAQ in general. This will simply get you started down the path of
processing your data. For more information about processing TTrees, refer to
the [ROOT website](root.cern.ch). Because the tree structure differs depending
on \--legacy-mode, we will show how to do this in two parts.

\subsection ddasdumper_usingoutput_legacy_sec Processing the "Legacy-mode" Data

The first thing that must happen is to load the libddaschannel.so shared
library into the ROOT interpreter.

\verbatim
tompkins@daqdev-wheezy:ddasdumper$ root -l
root[0] .L /usr/opt/ddas/VERSION/lib/libddaschannel.so
\endverbatim

The next step is to open the ROOT file containing our data and to get the TTree
from the file. Remember the TTree is named "dchan"

\verbatim
root [1] TFile* pFile = new TFile("legacy.root", "READ")
root [2] TTree* pTree;
root [3] pFile->GetObject("dchan", pTree)
\endverbatim

At this point, you should be able to call methods of the TTree, like
`pTree->Print()` or `pTree->Scan()`. We can also associate a ddaschannel object
with the "dchan" branch so that when we get an entry from the tree, the object
will be filled with the data. Here is how you do that:

\verbatim
root [4] ddaschannel* pChan = ddaschannel
root [5] pTree->SetBranchAddress("dchan", &pChan);
\endverbatim

At this point, you need only get entries from the TTree and do something with
the data. For example:

\verbatim
root [6] pTree->GetEntry(1)
root [7] pChan->GetEnergy()
(unsigned int)8838
root [8] pTree->GetEntry(2)
root [9] pChan->GetEnergy()
(unsigned int)8849
\endverbatim

From there, you can do whatever you need. For example, to process the entire
tree you could do so by:

\code

TH1* pHist = new TH1F("pHist", "Energy; Energy (a.u.); Counts", 16000, 0, 16000);
for (int i=0; i<pTree->GetEntries(); ++i) {
  pTree->GetEntry(i);
  pHist->Fill(pChan->GetEnergy());
}

pHist->Draw();

\endcode


\subsection ddasdumper_usingoutput_current_sec Processing the Standard Data

By "standard data" I intend to mean the ROOT file generated by ddasdumper
without the \--legacy-mode flag. The steps are similar in this to the previous
section on the legacy-mode data. The difference is that the name of the branch
is not "dchan". It is "ddasevent" instead. It also will contain DDASEvent
objects instead of ddaschannel objects. 

As before, we first need to load the libddaschanel.so and then we need to get
the TTree from the file. Here is how that looks once again:

\verbatim
tompkins@daqdev-wheezy:ddasdumper$ root -l
root [0] .L /usr/opt/ddas/VERSION/lib/libddaschannel.so
root [1] TFile* pFile = new TFile("normal.root", "READ")
root [2] TTree* pTree;
root [3] pFile->GetObject("dchan", pTree)
\endverbatim

Now we need to associate an object with the branch we care about. Because the
"ddasevent" branch is filled with DDASEvent object, we need to create a
DDASEvent object and associate it with the branch. In code, that is done by:

\verbatim
root [4] DDASEvent* pEvent = new DDASEvent;
root [5] pTree->SetBranchAddress("ddasevent", &pEvent);
\endverbatim

Now you can once again gain access to the data by calling `TTree::GetEntry(long
int)`. Here is now you might analyze the entire tree to plot the multipicity.

\code

TH1* pHist = new TH1F("pHist", "Multiplicity; Multiplicity; Counts", 10, 0, 10);
for (int i=0; i<pTree->GetEntries(); ++i) {
  pTree->GetEntry(i);

  pHist->Fill(pEvent->GetNEvents());
}

pHist->Draw();
\endcode



*/
