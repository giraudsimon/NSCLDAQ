#include "CTimeout.h"

CTimeout::CTimeout(double nSeconds)
    : m_nSeconds(nSeconds),
      m_start(std::chrono::high_resolution_clock::now())
{}

double CTimeout::getRemainingSeconds() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = now - m_start;

    double remainingSecs = m_nSeconds - elapsedTime.count();

    return (remainingSecs > 0) ? remainingSecs : 0;
}


bool CTimeout::expired()
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = now - m_start;

    return ((elapsedTime.count() - m_nSeconds) > m_nSeconds);
}
