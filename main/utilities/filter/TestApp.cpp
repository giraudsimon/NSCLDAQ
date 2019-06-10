/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#include <CFilterMain.h>
#include <CTransparentFilter.h>



int main(int argc, char** argv)
{

  // Create a transparent filter
  CFilter* filter = new CTransparentFilter;

  CFilterMain theApp(argc, argv);
  theApp.registerFilter(filter);

  theApp();

  return 0;

}
