/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

#ifndef DAQ_V8_CPHYSICSEVENTBUFFER_H
#define DAQ_V8_CPHYSICSEVENTBUFFER_H

#include <V8/CV8Buffer.h>
#include <V8/DataFormatV8.h>
#include <ByteBuffer.h>
#include <BufferPtr.h>

#include <cstdint>
#include <memory>

namespace DAQ {
  namespace V8 {

    class CRawBuffer;

    /*!
     * \brief Representation of a physics event
     *
     * Note that this is a building block to a V8::CPhysicsEventBuffer
     * and is really just a ByteBuffer with some extra operations that
     * enable access to the data with proper byte ordering.
     */
    class CPhysicsEvent
    {
    public:
      using iterator       = Buffer::BufferPtr<std::uint16_t>;

    private:
      bool                m_needsSwap;
      Buffer::ByteBuffer  m_buffer;

    public:
      /*!
       * \brief Construct from a ByteBuffer
       * \param data
       * \param needsSwap
       */
      CPhysicsEvent(const Buffer::ByteBuffer& data, bool needsSwap);

      /*!
       * \brief Construct from an expiring ByteBuffer
       *
       * This moves the data to avoid copying the whole
       * byte buffer during construction.
       *
       * \param data
       * \param needsSwap
       */
      CPhysicsEvent(Buffer::ByteBuffer&& data, bool needsSwap);

      /*!
       * \brief Copy constructor - copies deeply
       * \param rhs  - the event to copy from
       */
      CPhysicsEvent(const CPhysicsEvent& rhs);

      /*! Destructor       */
      ~CPhysicsEvent();

      /*!
       * \brief Assignment operator
       *
       * This does a deep copy of the target object.
       *
       * \param rhs - object to copy
       * \return reference to this
       */
      CPhysicsEvent& operator=(const CPhysicsEvent& rhs);

      /*!
       * \brief Returns the total number of shorts in body
       *
       *  Different versions of the buffers follow different conventions
       *  for specifying the number of shorts in the body.
       *
       * \return
       */
      std::size_t getNTotalShorts() const;

      /*!
       * \brief Get an iterator with proper byte swapping semantics
       * \return Buffer::BufferPtr<std::uint16_t>
       */
      iterator begin() const;

      /*!
       * Get an iterator with proper byte swapping semantics to end of buffer
       * \return Buffer::BufferPtr<std::uint16_t>
       */
      iterator end() const;

      /*!
       * \brief Access the buffer directly with ability to modify it
       * \return reference to buffer
       */
      Buffer::ByteBuffer& getBuffer() { return m_buffer;}

      /*!
       * \brief Direct, read-only access to buffer
       * \return reference to buffer
       */
      const Buffer::ByteBuffer& getBuffer() const { return m_buffer;}

      /*!
       * \brief Determines if data is in native byte order or not
       * \return bool
       * \retval true  - data not in native byte order
       * \retval false - otherwise
       */
      bool dataNeedsSwap() const { return m_needsSwap; }
    };


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    /*!
     * \brief Representation of the a DATABF type buffer
     *
     * The CPhysicsEventBuffer class allows for building up data buffers with
     * or reading them in. The main utilities
     * provided by this class is the ability to iterate over the events in
     * the body and then also to retrieve the buffer header.
     *
     *  When constructing from a raw buffer of data, it is necessary to decode
     *  size of each event when traversing the buffer. This class supports multiple
     *  means to do this. The standard means of doing this uses the buffmt data member of
     *  the corresponding bheader to determine the parse scheme. This approach falls
     *  short though because conversions can make all kinds of weird data buffers.
     *  By weird, I mean that the size value is not standard. This therefore, provides
     *  a means to override the preference of the buffer header (i.e. buffmt) with
     *  an alternative policy. The choice is set via the static member m_bodyType.
     */
    class CPhysicsEventBuffer : public CV8Buffer
    {
    public:
      using Event          = typename std::shared_ptr<CPhysicsEvent>;
      using Body           = typename std::vector<Event >;
      using iterator       = typename Body::iterator;
      using const_iterator = typename Body::const_iterator;

      enum BodyTypePolicy { BufferPreference,
                  Inclusive16BitWords,
                  Exclusive16BitWords,
                  Inclusive32BitBytes,
                  Inclusive32BitWords};
    private:
      bheader m_header;
      Body    m_body;
      bool    m_mustSwap;

    public:
      static  BodyTypePolicy m_bodyType;

    public:
      /*!
       *  \brief Default constructor
       *
       *  This is not actuaully intended to be used to construct a legitimate 
       *  object for use. Rather it should create an object that will later recieve
       *  a valid state after assignment.
       *
       */
      CPhysicsEventBuffer();

      /*  \brief Construct from raw data
       *
       *  This constructor can be used to pass in a body of raw data that is parsed
       *  into multiple physics events according to the semantics of the m_bodyType.
       *  It is important that the header contain valid information for the type, 
       *  sequence, ssignature, and lsignature elements. In the end, these types of 
       *  information are only useful for IO type operations. You only really need
       *  valid types if you intend to send this data on. In the end, the toRawBuffer()
       *  method will compute the proper values for nwds and nevt. 
       *
       *  \param header     a header specifying a DATABF type
       *  \param rawBody    body of buffer to parse into events
       */
      CPhysicsEventBuffer(const bheader& header,
                          const Buffer::ByteBuffer& rawBody);

      /*! \brief Convenience constructor for data consisting of shorts
       *
       *  This constructor is essentially the same thing as 
       *  CPhysicsEventBuffer(const bheader&, const Buffer::ByteBuffer&)
       *
       *  It just marshalls the data of the vector into a ByteBuffer and then
       *  calls the other constructor.
       *
       *  \param header   a header specifying a DATABF type
       *  \param body     the data
       *  \param mustSwap whether the data in the body needs swapping
       */
      CPhysicsEventBuffer(const bheader& header,
                          const std::vector<std::uint16_t>& body,
                          bool mustSwap=false);

      /*! \brief Construct from a CRawBuffer
       *
       * Because IO operations are intended to be accomplished through a
       * a CRawBuffer, this accepts one and transforms the data into a 
       * much more usable entity.
       *
       * The caller is responsible for ensuring that the data in the 
       * CRawBuffer is actually for a DATABF.
       *
       * \param rawBuffer   the data
       */
      CPhysicsEventBuffer(const CRawBuffer& rawBuffer);

      /*! \brief Copy constructor
       *
       * This does a deep copy of all the physics events in the body
       *
       * \param rhs   the object to copy
       */
      CPhysicsEventBuffer(const CPhysicsEventBuffer& rhs);

      /*! \brief Assignment operator overload
       *
       * Like the copy constructor, this does a deep copy of the events
       *
       * \param rhs   the object to copy
       */
      CPhysicsEventBuffer& operator=(const CPhysicsEventBuffer& rhs);

      /*! \brief Destructor
       *
       */
      ~CPhysicsEventBuffer();

      /*! \brief Access the header */
      bheader getHeader() const;

      /*! \brief Access the type */
      BufferTypes type() const { return DATABF; }

      // Access to the events
      iterator       begin();
      const_iterator begin() const;

      iterator       end();
      const_iterator end() const;

      std::size_t size() const { return m_body.size(); }
      Event at(std::size_t index) { return m_body.at(index); }

      
      /*! \brief Serialize the data 
       *
       *  The data is essentially copied into a byte array for IO operations.
       *  It is simply appended to any data that already exists in the buffer,
       *  however, the parsed header is replaced. The unique thing about physics
       *  event buffers is that the byte order of the data when constructed is
       *  kept in tact. This may mean that the header data is swapped before being
       *  serialized.
       *
       *  \param buffer   the buffer to copy into
       */
      void toRawBuffer(CRawBuffer &buffer) const;

      /*! \brief Add an event to the buffer
       *
       * The data in the physics event must not cause the data in the buffer
       * to become larger than the gBufferSize. If that is the case, the event
       * is not appended.
       *
       * \param pEvent  the event to add
       *
       * \return boolean
       * \retval true  - event was added
       * \retval false - event was not added for size reasons
       */
      bool appendEvent(std::shared_ptr<CPhysicsEvent> pEvent);

      /*! \brief Compute how many bytes are left unoccupied
       *
       * The user is only allowed to add an event if it is less tan
       * gBufferSize. This computes the current size of all the events
       * and then uses that size to compute the difference between that
       * and gBufferSize
       *
       * \returns gBufferSize - (total byte size of events)
       */
      std::size_t getNBytesFree() const;

     private:
      /*  \brief Parse body data
       *
       *  This is a helper function for the constructors that construct
       *  from raw data
       *
       * \param beg   start of data to parse
       * \param end   end of data to parse
       */
      void parseBodyData(Buffer::ByteBuffer::const_iterator beg,
                         Buffer::ByteBuffer::const_iterator end);

      /// The next two methods are related to parseBodData()
      void parseStandardBody(Buffer::ByteBuffer::const_iterator beg,
                             Buffer::ByteBuffer::const_iterator end);
      void parseGeneralBody(Buffer::ByteBuffer::const_iterator beg,
                            Buffer::ByteBuffer::const_iterator end);

      void swapBytesOfHeaderInPlace(bheader& header) const;

      /*! \brief Compute the number of shorts in the buffer */
      std::size_t computeNWords() const;

      /*! Sets the nevt, nwds, and type=DATABF    */
      void updateHeader(bheader& header) const;

      /*! \brief Convert between local policy type and PhysicsEventSizePolicy
       * 
       *  This truly solves a type mismatch. The conversion is between 
       *  enum values that are the same.
       */
      PhysicsEventSizePolicy mapBodyType(BodyTypePolicy type) const;
    };
    


  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CPHYSICSEVENTBUFFER_H
