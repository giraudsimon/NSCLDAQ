#include "CSemaphore.h"

#include <ErrnoException.h>

#include <fcntl.h>
#include <string.h>

namespace Os {


CScopedWait::CScopedWait(CSemaphore &sem) : m_sem(sem) { m_sem.wait(); }
CScopedWait::~CScopedWait() { m_sem.post(); }

CPosixSemaphore::CPosixSemaphore(const std::string &name, int count)
 : m_pSem(nullptr), m_name(name)
{
    m_pSem = sem_open(m_name.c_str(), O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, count);
    if (m_pSem == SEM_FAILED) {
        throw CErrnoException(strerror(errno));
    }
}

CPosixSemaphore::~CPosixSemaphore()
{
    sem_close(m_pSem);
    sem_unlink(m_name.c_str());
}


void CPosixSemaphore::timedWait(int nMilliseconds)
{

    struct timespec timeout;
    timeout.tv_sec = nMilliseconds/1000;
    timeout.tv_nsec = (nMilliseconds%1000) * 1000000;

    int status = sem_timedwait(m_pSem, &timeout);

    if (status < 0) {
        throw CErrnoException(strerror(errno));
    }
}

void CPosixSemaphore::wait()
{
    int status = sem_wait(m_pSem);
    if (status < 0) {
        throw CErrnoException(strerror(errno));
    }
}

bool CPosixSemaphore::tryWait()
{
    bool success = false;

    int status = sem_trywait(m_pSem);
    if (status == 0) {
        success = true;
    } else if (status < 0 && errno != EAGAIN) {
        throw CErrnoException(strerror(errno));
    } // else the process returned EAGAIN b/c it would have blocked

    return success;
}

/*!
 * \brief CPosixSemaphore::unlock
 *
 * Attempt to post (i.e. increment) the semaphore.
 *
 * \throws CErrnoException if fails b/c of EINVAL or EOVERFLOW (or anything else)
 *
 */
void CPosixSemaphore::post()
{
    int status = sem_post(m_pSem);
    if (status < 0) {
        throw CErrnoException(strerror(errno));
    }
}

int CPosixSemaphore::getCount() const
{
    int count;
    int status = sem_getvalue(m_pSem, &count);

    if (status < 0) {
        throw CErrnoException(strerror(errno));
    }

    return count;
}


} // end Os namespace
