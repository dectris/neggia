/**
MIT License

Copyright (c) 2017 DECTRIS Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "H5Superblock.h"
#include "H5SymbolTableEntry.h"
#include "H5SymbolTableNode.h"
#include "H5DataLayoutMsg.h"
#include "H5BLinkNode.h"
#include "H5LocalHeap.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string.h>
#include "assert.h"


H5SymbolTableEntry::H5SymbolTableEntry(const char *fileAddress, size_t offset):
   H5Object(fileAddress,offset)
{}

H5SymbolTableEntry::H5SymbolTableEntry(const H5Object &other):
   H5Object(other)
{
}

size_t H5SymbolTableEntry::linkNameOffset() const
{
   return uint64(0);
}

H5ObjectHeader H5SymbolTableEntry::objectHeader() const
{
   size_t offset = uint64(8);
   return H5ObjectHeader(fileAddress(), offset);
}

H5SymbolTableEntry::CACHE_TYPE H5SymbolTableEntry::cacheType() const
{
   return (CACHE_TYPE)(uint16(16));
}


H5Object H5SymbolTableEntry::scratchSpace() const
{
    return at(24);
}

uint64_t H5SymbolTableEntry::getAddressOfBTree() const
{
    assert(cacheType() == GROUP);
    return scratchSpace().uint64(0);
}

uint64_t H5SymbolTableEntry::getAddressOfHeap() const
{
    assert(cacheType() == GROUP);
    return scratchSpace().uint64(8);
}

uint32_t H5SymbolTableEntry::getOffsetToLinkValue() const
{
    assert(cacheType() == LINK);
    return scratchSpace().uint32(0);
}

H5SymbolTableEntry H5SymbolTableEntry::find(const std::string &entry) const
{
   assert(cacheType() == 1); // makes sense only for groups


   H5BLinkNode bTree(fileAddress(),scratchSpace().int64(0));
   assert(bTree.nodeType() == 0);
   H5LocalHeap treeHeap(fileAddress(),scratchSpace().int64(8));
   while(bTree.nodeLevel() > 0) {
      bool found = false;
      for(int i=1; i<=bTree.entriesUsed(); ++i) {
         size_t off = bTree.key(i).uint64(0);
         std::string key = std::string(treeHeap.data(off));
         if(entry <= key) {
            bTree = bTree.child(i-1);
            found = true;
            break;
         }
      }
      if(!found) throw std::runtime_error("Not found");
   }

   {
      int i;
      for(i=1; i<=bTree.entriesUsed(); ++i) {
         size_t off = bTree.key(i).uint64(0);
         std::string key = std::string(treeHeap.data(off));
         if(entry <= key) {
            break;
         }
      }
      if(i > bTree.entriesUsed()) throw std::out_of_range("Not found");
      H5SymbolTableNode symbolTableNode(bTree.child(i-1));
      for(i=0; i<symbolTableNode.numberOfSymbols();++i) {
         H5SymbolTableEntry retVal(symbolTableNode.entry(i));
         size_t off = retVal.linkNameOffset();
         std::string key = std::string(treeHeap.data(off));
         if(key == entry) {
            return retVal;
         }
      }
      throw std::out_of_range("Not found");
   }

}

namespace {
bool chunkCompareGreaterEqual(const uint64_t * key0, const uint64_t * key1, size_t len) {
   for(ssize_t idx = len-1; idx>=0; --idx) {
      if(key0[idx] < key1[idx]) return false;
      if(key0[idx] > key1[idx]) return true;
   }
   return true;
}
}

H5Object H5SymbolTableEntry::dataChunk(const std::vector<size_t> & offset) const
{
   assert(cacheType() == 0); // makes sense only for datasets
   const size_t keySize = 8+offset.size()*8;
   const size_t childSize = 8;

   H5ObjectHeader objHeader(this->objectHeader());
   for(int i=0; i<objHeader.numberOfMessages(); ++i) {
      H5HeaderMsgPreamble msg(objHeader.messageData(i));
      if(msg.type() == H5DataLayoutMsg::TYPE_ID) {
         H5DataLayoutMsg dataLayoutMsg(msg.getHeaderMsg());
         H5BLinkNode bTree(dataLayoutMsg.chunkBTree());

         while(bTree.nodeLevel() > 0) {
            bool found = false;
            for(int i=bTree.entriesUsed()-1; i>=0; --i) {
               H5Object key(bTree + 24 + i*(keySize + childSize));
               if(chunkCompareGreaterEqual(offset.data(),(const uint64_t* )key.address(8),offset.size())) {
                  bTree = H5BLinkNode(key.fileAddress(),key.uint64(keySize));
                  found = true;
                  break;
               }
            }
            if(!found) throw std::runtime_error("Not found");
         }

         for(int i=0; i<bTree.entriesUsed(); ++i) {
            H5Object key(bTree + 24 + i*(keySize + childSize));
            if(memcmp(key.address(8),offset.data(),offset.size()*sizeof(uint64_t)) == 0) {
               return key;
            }
         }
         throw std::runtime_error("Not found");
      }
   }
   throw std::runtime_error("Missing H5DataCache Layout");
}

