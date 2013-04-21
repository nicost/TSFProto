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

TSF::SpotList TSFUtils::GetHeaderBinary(std::ifstream* ifs) throw (TSFException)
{
   int magic = 0;
   long offset = 0;

   *ifs >> magic;
   *ifs >> offset;

   if (offset == 0) 
   {
      throw new TSFException("Offset to header was 0");
   }

   ifs->seekg(offset, std::ios_base::cur);

   google::protobuf::io::IstreamInputStream* input = 
      new google::protobuf::io::IstreamInputStream(ifs);
   google::protobuf::io::CodedInputStream* codedInput = 
      new google::protobuf::io::CodedInputStream(input);

   uint32_t mSize;
   codedInput->ReadVarint32(&mSize);

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

