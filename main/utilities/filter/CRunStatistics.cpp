
#include "CRunStatistics.h"

void CRunStatistics::incrementItemCount()
{
  ++m_itemCount;
}

void CRunStatistics::resetItemCount()
{
  m_itemCount=0;
}

void CRunStatistics::incrementBeginCount()
{
  ++m_beginCount;
}

void CRunStatistics::resetBeginCount()
{
  m_beginCount=0;
}


void CRunStatistics::incrementEndCount()
{
  ++m_endCount;
}

void CRunStatistics::resetEndCount()
{
  m_endCount=0;
}

void CRunStatistics::reset()
{
  resetItemCount();
  resetBeginCount();
  resetEndCount();
}

