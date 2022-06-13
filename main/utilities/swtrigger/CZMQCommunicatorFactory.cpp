/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CZMQCommunicatorFactory.cpp
 *  @brief: Implementation of the ZMQ communicator factory.
 */
#include "CZMQCommunicatorFactory.h"
#include "CZMQServerTransport.h"
#include "CZMQClientTransport.h"
#include "CZMQRouterTransport.h"
#include "CZMQDealerTransport.h"

#include <iostream>
#include <ios>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <stdexcept>
#include <system_error>
#include <errno.h>
#include <zmq.hpp>



/**
 * constructor
 *    Read in the endpoint table.  If there are no
 *    endpoints a warning is written to stderr  There are cases this
 *    is allowed but they're rare, so it's not fatal but it does warn.
 */
CZMQCommunicatorFactory::CZMQCommunicatorFactory()
{
    readEndpointFiles();
    if (m_endpoints.empty()) {
        std::cerr << "Warning ZMC Communicator factory created with empy endpoints table\n";
    }
}

/**
 * createFanoutTransport
 *     In ZMQ this is naturally implemented with a CZMQRouterTransport.
 *  @param endpointId  selects the endpoint from the m_endpoints table.
 *  @return CTransport* - pointer to the dynamically allocated transport
 *  @throw std::invalid_argument - the endpoint has no match in the table.
 */
CTransport*
CZMQCommunicatorFactory::createFanoutTransport(int endpointId)
{
    std::string URI = getUri(endpointId);
    return new CZMQRouterTransport(URI.c_str());
}
/**
 * createFanoutClient
 *    In ZMQ, this is naturally implemented as a dealer.
 *
 *  @param endpointId - selects the end point from the m_endpoints table.
 *  @param clientId   - The client's id used to route data.
 *  @return CTransport* - pointer to the dynamically allocated transport
 *  @throw std::invalid_argument - the endpoint has no match in the table.
 * 
 */
CTransport*
CZMQCommunicatorFactory::createFanoutClient(int endpointId, int clientId)
{
    std::string URI = getUri(endpointId);
    return new CZMQDealerTransport(URI.c_str(), clientId);
}
/**
 * createFaninSource
 *     Creates a client zmq transport that pushes. pairs with a FaninSink
 *
 *  @param endpointId
 *  @return CCTransport*
 */
CTransport*
CZMQCommunicatorFactory::createFanInSource(int endpointId)
{
    std::string URI= getUri(endpointId);
    return new CZMQClientTransport(URI.c_str(), ZMQ_PUSH);
}
/**
 * createFanInSink
 *    Returns a transport that is the end in end of several PUSHers
 *    (a PUll server).
 * @param endpointId
 * @return CTransport*
 */
CTransport*
CZMQCommunicatorFactory::createFanInSink(int endpointId)
{
    std::string URI = getUri(endpointId);
    return new CZMQServerTransport(URI.c_str(), ZMQ_PULL);
}
/**
 * createOneToOneSource
 *    Creates a PUSH that's a server.   The idea is that this will be
 *    a one-to-one pair of sockets.  I'd use ZMQ_PAIR except that the
 *    following text in the docs puts me off:
 *    " ZMQ_PAIR sockets are considered experimental and may have other
 *      missing or broken aspects."
 *
 *    Using PUSH/PULL has the risk that serveral sinks could connect to us.
 *    Guess we have to trust the user :-(
 *
 *  @param endpointId  - Id of the endpoint,.
 *  @return CTransport* pointer to the created  transport.
 */
CTransport*
CZMQCommunicatorFactory::createOneToOneSource(int endpointId)
{
    std::string URI = getUri(endpointId);
    return new CZMQServerTransport(URI.c_str(), ZMQ_PUSH);
}
/**
 * createOneToOneSinkn
 *     Creates a pull that's a client.  See CreateOneToOneSource.
 *
 *  @param endpointId
 *  @return CTransport
 */
CTransport*
CZMQCommunicatorFactory::createOneToOneSink(int endpointId)
{
    std::string URI = getUri(endpointId);
    return new CZMQClientTransport(URI.c_str(), ZMQ_PULL);
}

///////////////////////////////////////////////////////////////////////
//  Private utilities.

/**
 * readEndpointFiles
 *    Determines the presence of the endpoint table files and,
 *    for each one that exists, callse readEndpointFile to add its
 *    definitions to the endpoint table.
 *
 * - ~/.zmqservices
 * - ./zmqservices
 * - ZMQ_SERVICES env variable.
 */
void
CZMQCommunicatorFactory::readEndpointFiles()
{
    // ~/.zmqservices we'll use the HOME env variable to find that:
    
    const char* pHome = getenv("HOME");
    if (pHome) {
        std::string fname(pHome);
        fname += "/.zmqservices";
        readEndpointFile(fname.c_str());
    }
    
    // We'll use get_current_dir_name (GNU specific) since it let's us
    // not worry about figuring out how big the buffer to pass to get(c)wd
    // needs to be:
    
    char* pWd = get_current_dir_name();
    if (pWd) {
        std::string fname(pWd);
        free(pWd);
        fname += "/zmqservices";
        readEndpointFile(fname.c_str());
    }
    
    // Finally see if ZMQ_SERVICES translates:
    
    const char* pEnv = getenv("ZMQ_SERVICES");
    if (pEnv) {
        readEndpointFile(pEnv);
    }
}

/**
 * readEndpointFile
 *   Reads a single endpoint file.   Each line is read into a string
 *   with whitespace trimmed from the front and back.
 *   - Empty resulting lines are ignored.
 *   - Resulting lines that begin # are ignored.
 *   - All remaining lines are decoded into an integer and a string.
 *     The integer is an endpoint id and the string the ZMQ URI that
 *     correpsponds to it.
 * @param filename  - Name of the file to process.
 * @throw std::ios_base::failure if the file can't be opened.
 * @throw std::runtime_error if there are errors processing the file.
 */
void
CZMQCommunicatorFactory::readEndpointFile(const char* filename)
{
    std::ifstream configFile;
    auto e = configFile.exceptions();        // before opening make failures
    e |= std::ios::failbit;             // throw:
    configFile.exceptions(e);
    
    try {
        configFile.open(filename);
    } catch(std::ios_base::failure& f) {
        return;                 // Open failure.
    }
    try {
        std::string line;
        while (!configFile.eof()) {
            getline(configFile, line);
            std::string originalLine = line;
            line.erase(                        // Trim w.s. from front.
                line.begin(),
                std::find_if(
                    line.begin(),
                    line.end(), std::not1(std::ptr_fun<int, int>(std::isspace))
                )
            );                  // Trim spaces off front.
            if (line.front() == '#') continue;            // Comment line.
            line.erase(
                std::find_if(
                    line.rbegin(), line.rend(),
                    std::not1(std::ptr_fun<int, int>(std::isspace))
                ).base(),
                line.end()
            );
            if (line.empty()) continue;                 // line of only whitespace.
            
            int index;
            std::string uri;
            std::stringstream l(line);
            l >> index >> uri;
            if (l.fail()) {
                std::string msg("Failed to parse line zmq service file: ");
                msg += filename;
                msg += " offending line: '";
                msg += originalLine;
                msg += "'";
                throw std::runtime_error(msg);
            }
            // Add the service:
            
            m_endpoints[index] = uri;
        }
    } catch (std::ios_base::failure& f) {
        if(configFile.eof()) return;
        throw;
    } catch (std::runtime_error& r) {
        std::cerr << r.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    catch (std::runtime_error& r) {
        std::cerr << r.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}
/**
 * getUri
 *    @param endpointId - the id of an endpoint in the m_endpoints table.
 *    @param return std::string -the endpoint uri.
 *    @throws std::invalid_argument - no such endpoint.
 */
std::string
CZMQCommunicatorFactory::getUri(int endpointId)
{
    auto p = m_endpoints.find(endpointId);
    if (p == m_endpoints.end()) {
        std::stringstream msg;
        msg << "Failed to find endpoint: " << endpointId
            << " in ZMQ endpoint lookup table";
        throw std::invalid_argument(msg.str());
    }
    return p->second;
}