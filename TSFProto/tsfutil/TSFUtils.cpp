/**
 * Utility functions for reading and writing Tagged Spot Format files
 *
 * Nico Stuurman, nico.stuurman at ucsf.edu
 *
 * April 20, 2013
 *
 * Copyright UCSF, 2013
 */


#include <iostream>
#include <fstream>
#include <netinet/in.h>

#include "TSFException.h"
#include "TSFUtils.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/wire_format.h>

uint32_t TSFUtils::ReadInt32(std::istream *ifs) throw (TSFException)
{
   // TODO: throw appropriate exceptions
   int32char tmp;
   ifs->read(tmp.ch, 4);
   tmp.i = ntohl(tmp.i);
   return tmp.i;
}

uint64_t TSFUtils::ReadInt64(std::istream *ifs) throw (TSFException)
{
   // TODO: throw appropriate exceptions
   int64char tmp;
   ifs->read(tmp.ch, 8);
//   tmp.i = be64toh(tmp.i);
   return tmp.i;
}

TSF::SpotList TSFUtils::GetHeaderBinary(std::ifstream* ifs) throw (TSFException)
{
   if (!ifs->is_open())
   {
      throw TSFException("Input file was not opened correctly.  Does it exist?");
   }
   if (!ifs->good())
   {
      throw TSFException("Input file stream is in a bad state...");
   }

   int32_t magic = ReadInt32(ifs); 

   if (magic != 0) 
   {
      throw TSFException("Magic number is not 0, is this a tsf file?");
   }

   if (!ifs->good())
   {
      throw TSFException("Input file stream is in a bad state...");
   }

   int64_t offset = ReadInt64(ifs);

   if (offset == 0)
   {
      throw TSFException("Offset is 0, can not find header data in this file");
   }

   ifs->seekg(offset, std::ios_base::cur);

   google::protobuf::io::IstreamInputStream* input = 
      new google::protobuf::io::IstreamInputStream(ifs);
   google::protobuf::io::CodedInputStream* codedInput = 
      new google::protobuf::io::CodedInputStream(input);

   int32_t mSize = ReadInt32(ifs);

   std::string buffer;
   TSF::SpotList* sl = new TSF::SpotList();

   std::cout << "Magic: " << magic << ", offset: " << offset << " size: " << mSize << "\n";
   if (codedInput->ReadString(&buffer, mSize))                           
      sl->ParseFromString(buffer);

   return *sl;
}

TSF::SpotList TSFUtils::GetHeaderText(std::ifstream ifs)
{
}

void TSFUtils::WriteHeaderBinary(std::ofstream ofs, TSF::SpotList)
{
}

void TSFUtils::WriteHeaderText(std::ofstream ofs, TSF::SpotList)
{
}

TSF::Spot TSFUtils::GetSpotBinary(std::ifstream ifs)
{
}

void TSFUtils::WriteSpotBinary(std::ofstream of)
{
}

