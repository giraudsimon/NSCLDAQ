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
#include "GetOpt.h"
#include "rfcmdline.h"
#include <string.h>
#include <stdlib.h>

/**
 * GetOpt.cpp - Implementation of the GetOpt class.  This class lives in the skeleton
 *              so that it has access to the final cmdline.h file genearated by the
 *              gengetopt parse of the final options.ggo.  Since options.ggo actually
 *              determines the final shape of the getopt_args_info struct, this code cannot
 *              live outside the framework but must be compiled against the final 
 *              gengetopt configuration program.
 */


/**
 * Constrution... do the parse and save the resulting struct.
 *   
 * @param argc - Number of command line words.
 * @param argv - Pointer to the command line words.
 */
GetOpt::GetOpt(int argc, char** argv) :
  m_pArgs(0)
{
  m_pArgs = new struct gengetopt_args_info;
  cmdline_parser(argc, argv, m_pArgs); // exits on error.

}
/**
 * Destructor gets rid of the m_pArgs storage.
 */
GetOpt::~GetOpt()
{
  delete m_pArgs;
}
/**
 * Get the host field from the parse of the command line.
 * The host field is always set since the host is a required parameter.
 * it's defined.
 *
 * @return std::string
 * @retval The final resulting host name.
 */
std::string
GetOpt::getHost() const
{
  return std::string(m_pArgs->evbhost_arg);
}
/**
 * Get the port from the parse of the command line.  Note that once more command line
 * defaults ensure this will always have a value.
 *
 * @return std::string
 * @retval The port chosen.  This can be one of:
 *     - managed      - the port mabnager used to look up the server.
 *     - some-integer -The integer port number to use.
 *     - some-string  - The /etc/services services of the server.
 */
std::string
GetOpt::getPort() const
{
  return std::string(m_pArgs->evbport_arg);
}
/**
 * Returns the description option value.
 *
 * @return std::string
 * @retval value of --info switch
 */
std::string
GetOpt::getDescription() const
{
  return std::string(m_pArgs->info_arg);
}
/**
 * getSourceIds
 *
 *  returns a list of the source ids that are specified in the --ids required 
 *  option.
 *
 * @return std::list<int> - the source id values.
 */
std::list<int>
GetOpt::getSourceIds() const
{
  std::list<int> result;

  for (int i = 0; i < m_pArgs->ids_given; i++) {
    result.push_back(m_pArgs->ids_arg[i]);
  }
  return result;
}

/**
 * The application maintains a pointer to the command line parsed into a struct.
 * Ths returns a pointer to a copy of that.  Returning a copy allows this to be 
 * destroyed without impact on the data returned.
 * It is the caller's responsibility to free the storage handed back.
 *
 * @return struct genopt_args_info*
 * @retval pointer to a dynamically allocated copy of the parsed parameters.
 *
 */
struct gengetopt_args_info* 
GetOpt::getArgs()
{
  struct gengetopt_args_info* pArgs = new struct gengetopt_args_info;
  memcpy(pArgs, m_pArgs, sizeof(struct gengetopt_args_info));
  return pArgs;
}

/**
 * evbName
 *   returns a pointer to the event buider name string or a null if that was not provided.
 *
 * @return const char*
 */
const char*
GetOpt::evbName() const
{
  return m_pArgs->evbname_given ? m_pArgs->evbname_arg : 0;
}

void* gpTCLApplication=0;
