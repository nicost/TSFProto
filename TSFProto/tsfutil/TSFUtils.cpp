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
#include <sstream>
#include <netinet/in.h>

#include "TSFException.h"
#include "TSFUtils.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/wire_format.h>

int32_t TSFUtils::SwapInt32(int32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | ((val >> 16) & 0xFFFF);
}

int64_t TSFUtils::SwapInt64(int64_t val)
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}

// Reads a signed 32bit big endian int from a stream and converts to little endian 
// if needed
int32_t TSFUtils::ReadInt32(std::istream *ifs) throw (TSFException)
{
   int32char tmp;
   ifs->read(tmp.ch, 4);
   if (!IsBigEndian())
   {
      tmp.i = SwapInt32(tmp.i);
   }
   return tmp.i;
}

// Reads a signed 64bit big endian int from a stream and converts to little endian 
// if needed
int64_t TSFUtils::ReadInt64(std::istream *ifs) throw (TSFException)
{
   int64char tmp;
   ifs->read(tmp.ch, 8);
   if (!IsBigEndian())
   {
      tmp.i = SwapInt64(tmp.i);
   }
   return tmp.i;
}

/**
 * Reads the SpotList (Header info) from a tsf file
 * Expects a freshly opened stream, i.e. the filepointer should be at the beginning
 * Reads the magic number and offset to the SpotList
 * Reads the SpotList and return
 * For convenience, the File pointer will be set to 12, the first Spot message
 */
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

   uint32_t mSize;
   if (!codedInput->ReadVarint32(&mSize))
   {
      throw TSFException("Failed to read SpotList offset");
   }

   std::string buffer;
   TSF::SpotList* sl = new TSF::SpotList();

   // std::cout << "Magic: " << magic << ", offset: " << offset << " size: " << mSize << "\n";
   if (codedInput->ReadString(&buffer, mSize))                           
   {
      sl->ParseFromString(buffer);
   } else
   {
      throw TSFException("Failed to read SpotList data");
   }

   ifs->seekg(12);

   return *sl;
}


TSF::SpotList TSFUtils::GetHeaderText(std::ifstream* ifs)
{
}

void TSFUtils::WriteHeaderBinary(std::ofstream* ofs, TSF::SpotList spotList)
{
}

void TSFUtils::WriteHeaderText(std::ofstream* ofs, TSF::SpotList spotList)
{
   std::ostringstream headerOut;
   std::string tab = "\t";

   if (spotList.has_application_id()) 
      headerOut << "application_id: " << spotList.application_id() << tab;
   if (spotList.has_name())
      headerOut << "name: " << spotList.name() << tab;
   if (spotList.has_filepath())
      headerOut << "filepath: " << spotList.filepath() << tab;
   if (spotList.has_uid())
      headerOut << "uid: " << spotList.uid() << tab;
   if (spotList.has_nr_pixels_x())
      headerOut << "nr_pixels_x: " << spotList.nr_pixels_x() << tab;
   if (spotList.has_nr_pixels_y())
      headerOut << "nr_pixels_y: " << spotList.nr_pixels_y() << tab;
   if (spotList.has_pixel_size())
      headerOut << "pixel_size: " << spotList.pixel_size() << tab;
   if (spotList.has_nr_spots())
      headerOut << "nr_spots() " << spotList.nr_spots() << tab;
   if (spotList.has_box_size())
      headerOut << "box_size: " << spotList.box_size() << tab;
   if (spotList.has_nr_channels())
      headerOut << "nr_channels: " << spotList.nr_channels() << tab;
   if (spotList.has_nr_frames())
      headerOut << "nr_frames: " << spotList.nr_frames() << tab;
   if (spotList.has_nr_slices())
      headerOut << "nr_slices: " << spotList.nr_slices() << tab;
   if (spotList.has_nr_pos())
      headerOut << "nr_pos: " << spotList.nr_pos() << tab;
   // TODO: resolve enums to text
   if (spotList.has_location_units())
      headerOut << "location_units: " << spotList.location_units() << tab;
   if (spotList.has_intensity_units())
      headerOut << "intensity_units: " << spotList.intensity_units() << tab;
   if (spotList.has_fit_mode())
      headerOut << "fit_mode: " << spotList.fit_mode() << tab;
   if (spotList.is_track())
      headerOut << "is_track: " << spotList.is_track();

   headerOut << "\n";

   *ofs << headerOut.str().c_str();
}

TSF::Spot TSFUtils::GetSpotBinary(std::ifstream* ifs)
{
}

void TSFUtils::WriteSpotBinary(std::ofstream* of)
{
}

