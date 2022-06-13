/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef GETOPT_H
#define GETOPT_H

#include <string>
#include <list>


struct gengetopt_args_info;
/**
 * GetOpt.h - header for the option management class.  This class is needed
 *            because the application library files cannot know the final shape
 *            of the getgetopt_args_info struct because it depends on whatever changes
 *            the user makes in the options.ggo file.
 *
 */


class GetOpt {
private:
  struct gengetopt_args_info* m_pArgs; // Pointer to the args.

public:
  GetOpt(int argc, char** argv);
  ~GetOpt();

public:
  std::string getHost() const;
  std::string getPort() const;
  std::string getDescription() const;
  std::list<int> getSourceIds() const;
  const char* evbName() const;
  struct gengetopt_args_info* getArgs();

  
};


#endif
