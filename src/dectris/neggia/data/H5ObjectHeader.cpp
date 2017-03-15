#include "H5ObjectHeader.h"
#include <assert.h>
#include <iostream>
#include <stack>

H5ObjectHeader::H5ObjectHeader(const char *fileAddress, size_t offset): H5Object(fileAddress,offset)
{
   _init();
}

H5ObjectHeader::H5ObjectHeader(const H5Object &other): H5Object(other)
{
   _init();
}

uint16_t H5ObjectHeader::numberOfMessages() const
{
   return uint16(2);
}

uint32_t H5ObjectHeader::referenceCount() const
{
   return uint32(4);
}

uint32_t H5ObjectHeader::headerSize() const
{
   return uint32(8);
}

uint16_t H5ObjectHeader::messageType(int i) const
{
   return *(const uint16_t*)(fileAddress() + _messageOffset[i]);
}

uint16_t H5ObjectHeader::messageSize(int i) const
{
   return *(const uint16_t*)(fileAddress() + _messageOffset[i]+2);
}

uint8_t H5ObjectHeader::messageFlags(int i) const
{
   return *(const uint8_t*)(fileAddress() + _messageOffset[i]+4);
}

H5HeaderMsgPreamble H5ObjectHeader::messageData(int i) const
{
   return H5HeaderMsgPreamble(fileAddress(),_messageOffset[i]);
}

void H5ObjectHeader::_init()
{
   assert(uint8(0) == 1);
   assert(uint8(1) == 0);
   // std::cout<< std::dec << "HEADER SIZE: " << headerSize() << std::endl;

   constexpr uint64_t INVALID_SIZE = 0xffffffffffffffff;

   size_t messageOffset = offset() + 16; // 12(header data) + 4(alignment)

   struct ContBlock {
      uint64_t addr;
      uint64_t size;
   };
   std::stack<ContBlock> continuationBlocks;

   //uint64_t contBlockLocation = INVALID_SIZE;
   //uint64_t contBlockSize = INVALID_SIZE;
   uint64_t currentBlockSize = headerSize();
   uint64_t currentMessageSize = 0;
   int messageId = 0;
   while(true) {
      _messageOffset.push_back(messageOffset);
      H5Object messageObject(fileAddress(),messageOffset);
      uint16_t messageSize_ = messageObject.uint16(2);
      assert(messageSize_ == messageSize(messageId));
      // std::cout << "MESSAGE TYPE: " << std::hex << "0x" << (int) messageType(messageId) << "   ";
      // std::cout << "MESSAGE SIZE: " << std::dec << (int) messageSize(messageId) << "   ";
      assert(messageSize_%8 == 0);
      assert(messageObject.uint8(5) == 0);
      assert(messageObject.uint8(6) == 0);
      assert(messageObject.uint8(7) == 0);
      // std::cout << "MESSAGE FLAGS: " << std::hex << "0x" << (int) messageFlags(messageId) << "   ";
      if(messageType(messageId) == 0x10) {
         uint64_t cbl = messageData(messageId).uint64(8);
         if(cbl != INVALID_SIZE) {
            continuationBlocks.push({cbl, messageData(messageId).uint64(16)});
            //contBlockLocation =  cbl;
            //contBlockSize = messageData(messageId).uint64(16);
         }
         // std::cout << "CONT LOCATION: 0x" << std::hex << contBlockLocation << "   ";
      }
      // std::cout << std::endl;
      assert(messageObject.uint16(0) == messageType(messageId));
      assert(messageObject.uint16(0) <= 0x18);
      if(++messageId < numberOfMessages()) {
         uint16_t size = messageSize_ + 8;
         currentMessageSize += size;
         assert(currentMessageSize <= currentBlockSize);
         if(currentMessageSize == currentBlockSize) {
            // std::cout << "JUMPING TO 0x" << std::hex << contBlockLocation << std::endl;
            currentMessageSize = 0;

            assert(!continuationBlocks.empty());
            messageOffset = continuationBlocks.top().addr;
            currentBlockSize = continuationBlocks.top().size;
            continuationBlocks.pop();

//            assert(contBlockLocation != INVALID_SIZE);
//            messageOffset = contBlockLocation;
//            contBlockLocation = INVALID_SIZE;
//            assert(contBlockSize != INVALID_SIZE);
//            currentBlockSize = contBlockSize;
//            contBlockSize = INVALID_SIZE;


         } else {
            messageOffset += size;
         }
      } else {
         break;
      }
   }
   // std::cout << std::dec << "SIZE OF ALL MESSAGES: " << offsetX-12 << std::endl;

}

