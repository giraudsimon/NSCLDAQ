/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  Main.cpp
 *  @brief: Main program to read data from a ring data source,.
 */

/**
    What this template does:  The main program can analyze data from either an
    online data source or from file.
    -  It reads ring items from the data source.
    -  It figures out the ring item type and creates the appropriate ring item
       type.
    -  Non PHYSICS_EVENT items are just dumped to stdout using the stringifiying
       method in the CRingItem class.
    -  Physics items:  The FragmentIndex class that Jeromy wrote is used
                       to iterate over all of the fragments in an event.
                       The framework allows you to register handlers for
                       data sources that are encountered in the  list of
                       fragments in an event.  Unhandled fragments are reported
                       but no error is thrown in consequence.
                       
    Note that heavy use is made of NSCLDAQ Ring item formatting classes in the
    input code.   I recommend that any NSCLDAQ analysis code do that too so you're
    not continuously reinventing wheels (or rings) that already have been
    written.
    
    To make use of this in e.g. Grutinizer
    -  Use whatever input methods you have to absorb ring items from the data
       source.
    -  Feed those ring items to a CRingItemDecoder class instance (see CRingItemDecoder.{h.cpp}).
    -  Note the in file documentation of the CFragmentHandler base class.
       You will need to have created fragment handlers for each data source type you
       are going to handle these are derived from CFragmentHandler and implement
       the function call operator.
    -  Register the the fragment handlers you need with the CRingItemDecoder
       instance so that it will call them for each fragment you can handle.
    -  If you want, you can register an "end of event" handler.  The reason
       you might want to do this is to create parameters that span the fragments
       the event has (e.g. a timestamp difference from one fragment type to another).
       
    The sample code:
       - Registers a fragment handler for S800 data.
       - Registers a fragment handler for the CAESAR data.
       
       These fragment handlers will just output  the body header info from
       each fragment and will output information about each packet the
       the event fragment has.  The end handler  is registered just to show
       that it works.
       - Assumes the endianess of this system is the same as the one that took
         the data in the first place.
       
       The code is heavily documented with comments.
*/
    
// CDataSource Factory creates a data source for file or online from a URI.
// CDataSource is the base class for data sources.
//  docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/index.html has detailed
// documentation of all the classes we're going to use.


#include "CTimeChecker.h"              // Sample code.

extern int Main(int argc, char** argv, CRingItemDecoder& decoder);

/**
 * main
 *    Entry point for the program -- the usual command parameters.
 */

int
main(int argc, char**argv)
{
    CTimeCheckDecoder       decoder;
    Main(argc, argv, decoder);
    
}
    
    
