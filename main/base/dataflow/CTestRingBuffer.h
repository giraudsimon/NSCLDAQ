


#ifndef CTESTRINGBUFFER_H
#define CTESTRINGBUFFER_H

#include <CRingBuffer.h>
#include <vector>


class CTestRingBuffer : public CRingBuffer 
{
  private:
    std::vector<char> m_buffer;

  public:

    CTestRingBuffer(std::string name);

    /*! \brief Insert data into the sink
     *
     *  The data are pushed onto the back of the m_buffer.
     *
     *  \param pData  address of the block of data to write
     *  \param nBytes number of bytes to write
     */
    size_t put(const void* pBuffer, size_t nBytes, unsigned long timeout=ULONG_MAX);
    size_t get(void* pBuffer, size_t maxBytes, size_t minBytes = 1, 
               unsigned long timeout=ULONG_MAX);
    size_t peek(void* pBuffer, size_t maxbytes);
    void   skip(size_t nBytes);

    size_t availableData();

    void pollblock() {}

    /*!
     * \brief Access the underlying buffer
     * \return read-only reference to the data buffer
     */
    const std::vector<char>& getBuffer() const { return m_buffer; };

  private:

    /*!
     * \brief Read dead from the source
     *
     *  It is the caller's responsibility to ensure that the number
     *  destination of the data being read is large enough to hold the
     *  requested data.
     *
     * \param pBuffer   location to place the data
     * \param nBytes    number of bytes to read
     *
     *  \throws std::runtime_error if insufficient data exists in buffer to satisfy request
     */
    virtual void read(char* pBuffer, size_t nBytes);



};

#endif
