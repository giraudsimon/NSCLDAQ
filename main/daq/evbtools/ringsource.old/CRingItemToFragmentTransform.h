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
 * @file CRingItemToFragmentTransform.h
 * @brief defines ring specific event builder data source class.
 */
#ifndef CRINGITEMTOFRAGMENTTRANSFORM_H
#define CRINGITEMTOFRAGMENTTRANSFORM_H

#include <config.h>

#include <CEVBClientApp.h>
#include <EVBFramework.h>
#include <DataFormat.h>

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <functional>

// Forward definitions:

class CRingBuffer;
class CRingItem;
struct _PhysicsEventItem;
typedef _PhysicsEventItem *pPhysicsEventItem;

class CRingItemToFragmentTransform 
{
  // Prototype for the timestamp getter:

private:
  std::vector<std::uint32_t> m_allowedSourceIds;
  std::uint32_t         m_defaultSourceId;
  std::function<uint64_t(pPhysicsEventItem)> m_timestamp;
  bool                  m_expectBodyHeaders;

  // Canonicals:

public:
  CRingItemToFragmentTransform(std::uint32_t defaultSourceId);
  virtual ~CRingItemToFragmentTransform();

  // Main entry point
  ClientEventFragment operator()(CRingItem* pItem, uint8_t* pDest);

  // Getters and setters
  void setAllowedSourceIds(const std::vector<uint32_t>& ids)
  { m_allowedSourceIds = ids; }
  std::vector<uint32_t>& getAllowedSourceIds() { return m_allowedSourceIds; }

  void     setDefaultSourceId(uint32_t id) { m_defaultSourceId = id; }
  uint32_t getDefaultSourceId() { return m_defaultSourceId; }

  void setTimestampExtractor(std::function<uint64_t(pPhysicsEventItem)> extractorFunc) 
  { m_timestamp = extractorFunc; }
  std::function<uint64_t(pPhysicsEventItem)> getTimestampExtractor() const { return m_timestamp; }

  void setExpectBodyHeaders(bool yesno) { m_expectBodyHeaders = yesno; }
  bool getExpectBodyHeaders() const { return m_expectBodyHeaders; }

private:
  bool formatPhysicsEvent(pRingItem item, CRingItem* p, ClientEventFragment& frag);
  void validateSourceId(uint32_t sourceId);
  bool isValidSourceId(uint32_t sourceId);
};

#endif
