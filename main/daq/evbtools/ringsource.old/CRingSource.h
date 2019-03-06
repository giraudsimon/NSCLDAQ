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
/**
 * @file CRingSource.h
 * @brief defines ring specific event builder data source class.
 */
#ifndef CRINGSOURCE_H
#define CRINGSOURCE_H

#include <config.h>

#include <CEVBClientApp.h>
#include <EVBFramework.h>
#include <DataFormat.h>
#include <CRingSource.h>
#include <CRingItemToFragmentTransform.h>

#include <string>
#include <vector>
#include <stdint.h>

// Forward definitions:

class CRingBuffer;
class CRingItem;
struct _PhysicsEventItem;
typedef _PhysicsEventItem *pPhysicsEventItem;

struct gengetopt_args_info;
struct timespec;

/**
 * Provides experiment specific code for the Ring Buffer experiment specific
 * data source.   This takes data from the ring named --ring (TCP/IP if needed)
 * and invokes a user written timestamp extractor in the library defined by
 * --timestampextractor.
 *
 *  The timestamp extractor is event format specific and must be supplied by 
 *  the user.  It must have "C" linkage and have a single of the signature:
 * \verbatim
 *    uint64_t timestamp(pPhysicsEventItem item);
 * \endverbatim
 * 
 * The assumption is that only responses to physics triggers actually have timestamps.
 * all other ring item types either have no timestamp (scaler items e.g.) or are barrier
 * fragments (e.g. BEGIN_RUN.
 *
 */
class CRingSource : public CEVBClientApp 
{
  // Prototype for the timestamp getter:

  typedef uint64_t (*tsExtractor)(pPhysicsEventItem item);

  // attributes:

private:
  struct gengetopt_args_info* m_pArgs;
  CRingBuffer*          m_pBuffer;
  std::vector<uint32_t> m_allowedSourceIds;
  uint32_t              m_defaultSourceId;
  tsExtractor           m_timestamp;
  bool                  m_stall;
  uint32_t              m_stallCount;
  bool                  m_expectBodyHeaders;
  bool             m_fOneshot;
  unsigned         m_nEndRuns;
  unsigned         m_nEndsSeen;
  unsigned         m_nTimeout;
  unsigned         m_nTimeWaited;
  int              m_nTimeOffset;
  CEVBFragmentList  m_frags;

  CRingItemToFragmentTransform  m_wrapper;
  bool             m_myRing;
 
  
  // Canonicals:

public:
  CRingSource(CRingBuffer* pBuffer, 
              const std::vector<uint32_t>& allowedIds, 
              uint32_t defaultId, 
              tsExtractor extractor);
  CRingSource(int argc, char** argv);
  virtual ~CRingSource();

private:
  CRingSource(const CRingSource&);
  CRingSource& operator=(const CRingSource&);
  int operator==(const CRingSource&) const;
  int operator!=(const CRingSource&) const;

public:
  virtual void initialize();
  virtual bool dataReady(int ms);
  virtual void getEvents();
  virtual void shutdown();

  void transformAvailableData(uint8_t*& pBuffer);
  const CEVBFragmentList& getFragmentList() const { return m_frags; }

  void setOneshot(bool val) { m_fOneshot = val; }
  void setNumberOfSources(unsigned nsources) { m_nEndRuns = nsources; }
  bool oneshotComplete();

public:
  std::string copyLib(std::string original);
  uint64_t timedifMs(struct timespec& later, struct timespec& earlier);
};

#endif
