#ifndef IOADAPTER_H
#define IOADAPTER_H

#include <ByteOrder.h>

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <iterator>
using namespace std;

namespace DAQ {
  namespace Buffer {

    template<class Container> class Deserializer
    {

      const Container& m_container;

      typedef typename Container::const_iterator iterator;
      iterator m_get;
      iterator m_beg;
      iterator m_end;

      bool        m_fail;
      bool        m_eof;
      bool        m_bad;

      BO::CByteSwapper m_swapper; // whether bytes need to be swapped

    public:
      Deserializer(const Container& container, bool mustSwap=false)
        : m_container(container),
        m_get(container.begin()),
        m_beg(container.begin()),
        m_end(container.end()),
        m_fail(false),
        m_eof(false),
        m_bad(false),
        m_swapper(mustSwap)
      {}

      iterator pos() { return m_get; }
      void setPosition(iterator pos) { m_get = pos; }

      template<typename T>
      void extract(T& type) {

        auto end = m_get + sizeof(type);

        // do not allow to go past the end
        if (end > m_end) {
          m_eof = true;
          m_fail = true;
        } else {

          m_swapper.interpretAs<T>(m_get, type);

          m_get = end;
        }

      }

      template<typename T>
      T extract() {
        T type;
        extract(type);
        return type;
      }

      template<typename T>
      void extract(T* begin, T* end) {
        auto itend = m_get + std::distance(begin, end)*sizeof(T);

        // do not allow to go past the end
        if (itend > m_end) {
          m_eof = true;
          m_fail = true;
        } else {

          while (begin != end) {
            m_swapper.interpretAs<T>(m_get, *begin);
            ++begin;
            ++m_get;
          }
          m_get = itend;
        }

      };

      void clear() {
        m_fail = false;
        m_eof = false;
        m_bad = false;
      }

      bool bad()  const { return m_bad; }
      bool fail() const { return m_fail; }
      bool eof()  const { return m_eof; }



    };
  } // end of Buffer
} // end of DAQ


template<class Container>
DAQ::Buffer::Deserializer<Container>&
operator>>(DAQ::Buffer::Deserializer<Container>& device, std::uint8_t& val) {

  device.extract(val);

  return device;
}

template<class Container>
DAQ::Buffer::Deserializer<Container>&
operator>>(DAQ::Buffer::Deserializer<Container>& device, std::int8_t& val) {

  device.extract(val);

  return device;
}

template<class Container>
DAQ::Buffer::Deserializer<Container>&
operator>>(DAQ::Buffer::Deserializer<Container>& device, std::uint16_t& val) {

  device.extract(val);

  return device;
}

template<class Container>
DAQ::Buffer::Deserializer<Container>&
operator>>(DAQ::Buffer::Deserializer<Container>& device, std::int16_t& val) {

  device.extract(val);

  return device;
}

template<class Container>
DAQ::Buffer::Deserializer<Container>&
operator>>(DAQ::Buffer::Deserializer<Container>& device, std::uint32_t& val) {

  device.extract(val);

  return device;
}

template<class Container>
DAQ::Buffer::Deserializer<Container>&
operator>>(DAQ::Buffer::Deserializer<Container>& device, std::int32_t& val) {

  device.extract(val);

  return device;
}

template<class Container>
DAQ::Buffer::Deserializer<Container>&
operator>>(DAQ::Buffer::Deserializer<Container>& device, std::uint64_t& val) {

  device.extract(val);

  return device;
}

template<class Container>
DAQ::Buffer::Deserializer<Container>&
operator>>(DAQ::Buffer::Deserializer<Container>& device, std::int64_t& val) {

  device.extract(val);

  return device;
}

#endif // IOADAPTER_H
