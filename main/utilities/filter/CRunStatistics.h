
#ifndef CRUNSTATISTICS_H
#define CRUNSTATISTICS_H

#include <cstddef>

class CRunStatistics 
{
  private:
    size_t m_itemCount;
    size_t m_beginCount;
    size_t m_endCount;
    int    m_runNumber;
    
  public:
    size_t getItemCount() const { return m_itemCount;}
    void incrementItemCount();
    void resetItemCount();

    size_t getBeginCount() const { return m_beginCount;}
    void incrementBeginCount();
    void resetBeginCount();

    size_t getEndCount() const { return m_endCount;}
    void incrementEndCount();
    void resetEndCount();

    void reset();

    void setRunNumber(int run) { m_runNumber = run;}
    int getRunNumber() const { return m_runNumber;}

};
#endif
