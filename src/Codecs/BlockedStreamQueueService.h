// Copyright (c) 2009, Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
//
#ifndef BLOCKEDSTREAMBUFFERCONSUMER_H
#define BLOCKEDSTREAMBUFFERCONSUMER_H

#include <Common/QuickFAST_Export.h>

#include <Communication/BufferQueueService.h>
#include <Communication/LinkedBuffer.h>
#include <Codecs/DataSource.h>
#include <Codecs/SynchronousDecoder.h>
#include <Codecs/TemplateRegistry_fwd.h>
#include <Messages/ValueMessageBuilder_fwd.h>

namespace QuickFAST
{
  namespace Codecs
  {
    /// @brief Service a Receiver's Queue when expecting streaming data (TCP/IP) with (or without) block headers.
    class QuickFAST_Export BlockedStreamQueueService
      : public Communication::BufferQueueService
      , public Codecs::DataSource
    {
    public:
      /// @brief Expected type of header
      enum HeaderType {
        HEADER_FIXED,
        HEADER_FAST,
        HEADER_NONE};

    public:
      /// @brief Construct given the registry to be used by the decoder and the builder to receive data
      /// @param templateRegistry defines the messages to be decoded
      /// @param builder accepts data from the decoder for use by the application
      BlockedStreamQueueService(
          TemplateRegistryPtr templateRegistry,
          Messages::ValueMessageBuilder & builder);

      virtual ~BlockedStreamQueueService();


      /// @brief Define the header expected on each message
      /// @param type of header expected.
      /// @param prefix what comes before the message size field( (HEADER_FIXED: byte count, HEADER_FAST: field count)
      /// @param size number of bytes in message size field (HEADER_FIXED only)
      /// @param swap endian swap the size field (HEADER_FIXED only
      /// @param suffix what comes after the message size field( (HEADER_FIXED: byte count, HEADER_FAST: field count)
      void setHeader(
        HeaderType type,
        size_t prefix = 0,
        size_t size = 0,
        bool swap = false,
        size_t suffix = 0)
      {
        headerType_ = type;
        headerPrefix_ = prefix;
        blockSizeBytes_ = size;
        blockSizeSwap_ = swap;
        headerSuffix_ = suffix;
      }

      ///////////////////////////
      // Implement BufferQueueService
      virtual bool serviceQueue(Communication::Receiver & receiver);
      virtual void receiverStarted(Communication::Receiver & receiver);
      virtual void receiverStopped(Communication::Receiver & receiver);

      ///////////////////////
      // Implement DataSource
      //
      virtual bool readByte(uchar & byte);
      virtual int messageAvailable();

    private:
      bool readByteInternal(uchar & byte);
    private:
      BlockedStreamQueueService & operator = (const BlockedStreamQueueService &);
      BlockedStreamQueueService(const BlockedStreamQueueService &);
      BlockedStreamQueueService();

    private:
      Messages::ValueMessageBuilder & builder_;
      bool stopping_;

      /////////////////////////////////////
      // describe the block header (if any)

      HeaderType headerType_;
      // header prefix:
      // for fixed size headers: how many bytes preceed the message size field in the header
      // for FAST encoded headers: how many "simple" FAST fields proceed the message size (stop bit terminated)
      size_t headerPrefix_;

      // message size
      // for fixed size headers only: how many bytes in header
      size_t blockSizeBytes_;
      // for fixed size headers only: endian-swap message size?
      bool blockSizeSwap_;

      // header suffix:
      // for fixed size headers: how many bytes follow the message size field in the header
      // for FAST encoded headers: how many "simple" FAST fields follow the message size (stop bit terminated)
      size_t headerSuffix_;
      // end of block header description
      //////////////////////////////////

      Decoder decoder_;

      // Nonzero during call to consumeBuffer ->decoder
      // zero the rest of the time.
      Communication::Receiver * receiver_;

      // buffer from which data is being pulled
      Communication::LinkedBuffer * currentBuffer_;
      size_t pos_;

      // Message size extracted from header
      bool blockSizeIsValid_;
      bool parsingBlockSize_;
      size_t blockSize_;
      size_t headerPos_;
      // end of busy_ protection
      //////////////////////////
    };
  }
}
#endif BLOCKEDSTREAMBUFFERCONSUMER_H