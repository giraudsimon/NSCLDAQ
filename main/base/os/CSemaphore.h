#ifndef OS_CSEMAPHORE_H
#define OS_CSEMAPHORE_H


#include <semaphore.h>
#include <string>

namespace Os {

class CSemaphore;

class CScopedWait
{
  private:
    CSemaphore& m_sem;

  public:
    CScopedWait(CSemaphore& sem);
    CScopedWait(const CScopedWait& rhs) = delete;
    ~CScopedWait();
};



class CSemaphore
{
public:
    virtual void wait() = 0;
    virtual void timedWait(int nMilliseconds) = 0;

    virtual bool tryWait() = 0;

    virtual void post() = 0;

    virtual int getCount() const = 0;
};




class CPosixSemaphore : public CSemaphore
{
private:
    sem_t*      m_pSem;
    std::string m_name;

private:
    CPosixSemaphore(const CPosixSemaphore& rhs) = delete;
    CPosixSemaphore& operator=(const CPosixSemaphore&);

public:
    CPosixSemaphore(const std::string& name, int count);

    virtual ~CPosixSemaphore();

    virtual void wait();
    virtual void timedWait(int nMilliseconds);

    virtual bool tryWait();
    virtual void post();

    virtual int getCount() const;

    sem_t* getNativeHandle() { return m_pSem; }
};


} // end Os namespace



#endif // CSYSTEMSEMAPHORE_H
