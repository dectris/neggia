#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

int main(int, char **) {
  int fd = open(TEST_HDF5, O_RDONLY);
  off_t fsize = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  const char magicNumber[] = "\211HDF\r\n\032\n";
  char * filePointer = (char*) mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
  if(filePointer == MAP_FAILED) {
    std::cout << "ERRNO: " << errno << std::endl;
    return 1;
  }
  close(fd);
  assert(std::string(filePointer,0,8) == std::string(magicNumber));
  int version = (int)filePointer[8];
  std::cout << "VERSION: " << version << std::endl;
  assert(filePointer[11] == 0);
  int offsetSize = (int)filePointer[13];
  std::cout << "OFFSET SIZE: " << offsetSize << std::endl;
  int offsetLength = (int)filePointer[14];
  std::cout << "OFFSET LENGTH: " << offsetLength << std::endl;
  assert(filePointer[15] == 0);

  uint16_t groupLeafNodeK = *(uint16_t*)(filePointer+16);
  std::cout <<"GROUP LEAF NODE K: " << groupLeafNodeK << std::endl;
  uint16_t groupInternalNodeK = *(uint16_t*)(filePointer+18);
  std::cout <<"INTERNAL LEAF NODE K: " << groupInternalNodeK << std::endl;
  
  uint64_t baseAddress = *(uint64_t*)(filePointer+24);
  assert(baseAddress == 0);
  uint64_t AddressOfFileFreespaceInfo = *(uint64_t*)(filePointer+24+offsetSize);
  assert(AddressOfFileFreespaceInfo == 0xffffffffffffffff);
  uint64_t EndOfFileAddress = *(uint64_t*)(filePointer+24+2*offsetSize);
  assert (EndOfFileAddress == fsize);
  uint64_t DriverInformationBlockAddress = *(uint64_t*)(filePointer+24+3*offsetSize);
  std::cout << "Driver Information Block Address: " <<  std::hex << DriverInformationBlockAddress <<std::endl; 


  char * rootTable = (filePointer + 24 + 4*offsetSize);
  uint64_t linkNameOffset = *(uint64_t*)(rootTable);
  std::cout << "LINK NAME OFFSET: "  << linkNameOffset << std::endl;
  uint64_t objectHeaderAddress = *(uint64_t*)(rootTable + offsetSize);
  std::cout << "OBJECT HEADER ADDRESS: "  << objectHeaderAddress << std::endl;
  uint32_t cacheType = *(uint32_t*)(rootTable + 2*offsetSize);
  std::cout << "CACHE TYPE: " << cacheType << std::endl;
  assert (*(uint32_t*)(rootTable + 2*offsetSize+4) == 0);

  uint64_t addressOfBTree = *(uint64_t*)(rootTable + 2*offsetSize+8);
  std::cout << "OFFSET OF B TREE: " << std::hex << addressOfBTree << std::endl;
  uint64_t addressOfLocalHeap = *(uint64_t*)(rootTable + 3*offsetSize+8);
  std::cout << "OFFSET OF B TREE LOCAL HEAP: " << std::hex << addressOfLocalHeap << std::endl;
  std::cout << "LOCAL HEAP SIGNATURE: " << std::string((char*)(filePointer+addressOfLocalHeap),4) << std::endl;
  char nodeSignature[5];
  memset(nodeSignature,0,5);
  memcpy(nodeSignature,filePointer+addressOfBTree,4);
  std::cout << "NODE SIGNATURE: " << nodeSignature << std::endl;
  int nodeType  = filePointer[addressOfBTree+4];
  std::cout << "NODE TYPE: " << nodeType << std::endl;
  int nodeLevel = filePointer[addressOfBTree+5];
  std::cout << "NODE LEVEL: " << nodeLevel << std::endl;
  int nodeEntries = *(uint16_t*)(filePointer + addressOfBTree+6);
  std::cout << "NODE ENTRIES: " << std::dec << nodeEntries << std::endl;
  std::cout << "ADDRESS OF LEFT SIBLING: " << std::hex <<  *(uint64_t*)(filePointer + addressOfBTree+8) << std::endl;
  std::cout << "ADDRESS OF RIGHT SIBLING: " << std::hex <<  *(uint64_t*)(filePointer + addressOfBTree+16) << std::endl;
  for(int i=0; i<groupLeafNodeK; ++i) {
     std::cout << "CHILD NODE: " << std::dec << i << std::endl;
     uint64_t offsetOfKeyLocalHeap = *(uint64_t*)(filePointer + addressOfBTree + 24+i*16);
     std::cout << "ADDRESS OF KEY LOCAL HEAP: " << std::hex << offsetOfKeyLocalHeap << std::endl;
     std::cout << "KEY NAME: \"" << std::string((char*)(filePointer+addressOfLocalHeap+32+offsetOfKeyLocalHeap)) << "\"" << std::endl;
  }



  
  munmap(filePointer, fsize);
  return 0;
}
