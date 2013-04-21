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

#include "TSFException.h"
#include "TSFUtils.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/wire_format.h>

TSF::SpotList TSFUtils::GetHeaderBinary(std::ifstream* ifs) throw (TSFException)
{
   google::protobuf::io::IstreamInputStream* input = 
      new google::protobuf::io::IstreamInputStream(ifs);
   google::protobuf::io::CodedInputStream* codedInput = 
      new google::protobuf::io::CodedInputStream(input);


   int32_t magic;
   *ifs >> magic;

   if (magic != 0) 
   {
      throw new TSFException("Magic number is not 0, is this a tsf file?");
   }

   int64_t offset = 0;
   *ifs >> offset;

   if (offset != 0)
   {
      throw new TSFException("Offset is 0, can not find header data in this file");
   }

   ifs->seekg(offset, std::ios_base::cur);

   int32_t mSize = 0;
   *ifs >> mSize;

   std::string buffer;
   TSF::SpotList* sl = new TSF::SpotList();

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

