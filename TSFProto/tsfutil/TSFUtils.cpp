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
 */
int TSFUtils::GetHeaderBinary(std::ifstream* ifs, TSF::SpotList* sl) throw (TSFException)
{
   if (sl == NULL)
   {
      throw TSFException("Programming error: SpotList pointer was NULL\n");
   }

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
      delete codedInput;
      delete input;
      throw TSFException("Failed to read SpotList offset");
   }

   std::string buffer;

   if (codedInput->ReadString(&buffer, mSize))                           
   {
      sl->ParseFromString(buffer);
   } else
   {
      delete codedInput;
      delete input;
      throw TSFException("Failed to read SpotList data");
   }

   delete codedInput;
   delete input;

   return GOOD;
}


int TSFUtils::GetHeaderText(std::ifstream* ifs, TSF::SpotList* sl) throw (TSFException)
{
   return GOOD;
}

void TSFUtils::WriteHeaderBinary(std::ofstream* ofs, TSF::SpotList* spotList) throw (TSFException)
{
}

/**
 * Writes the Header (SpotList) to a text file in key: value format
 * This can probably more elegantly be implemented using reflection
 *
 * param ofs - pointer to ofstream that shoudl have been opened already
 * param spotList - pointer to the Header structure
 */
void TSFUtils::WriteHeaderText(std::ofstream* ofs, TSF::SpotList* spotList) throw (TSFException)
{
   if (!ofs->is_open())
   {
      throw TSFException ("Output file is not open\n");
   }

   std::ostringstream headerOut;
   std::string tab = "\t";

   if (spotList->has_application_id()) 
      headerOut << "application_id: " << spotList->application_id() << tab;
   if (spotList->has_name())
      headerOut << "name: " << spotList->name() << tab;
   if (spotList->has_filepath())
      headerOut << "filepath: " << spotList->filepath() << tab;
   if (spotList->has_uid())
      headerOut << "uid: " << spotList->uid() << tab;
   if (spotList->has_nr_pixels_x())
      headerOut << "nr_pixels_x: " << spotList->nr_pixels_x() << tab;
   if (spotList->has_nr_pixels_y())
      headerOut << "nr_pixels_y: " << spotList->nr_pixels_y() << tab;
   if (spotList->has_pixel_size())
      headerOut << "pixel_size: " << spotList->pixel_size() << tab;
   if (spotList->has_nr_spots())
      headerOut << "nr_spots() " << spotList->nr_spots() << tab;
   if (spotList->has_box_size())
      headerOut << "box_size: " << spotList->box_size() << tab;
   if (spotList->has_nr_channels())
      headerOut << "nr_channels: " << spotList->nr_channels() << tab;
   if (spotList->has_nr_frames())
      headerOut << "nr_frames: " << spotList->nr_frames() << tab;
   if (spotList->has_nr_slices())
      headerOut << "nr_slices: " << spotList->nr_slices() << tab;
   if (spotList->has_nr_pos())
      headerOut << "nr_pos: " << spotList->nr_pos() << tab;
   // TODO: resolve enums to text
   if (spotList->has_location_units())
      headerOut << "location_units: " << spotList->location_units() << tab;
   if (spotList->has_intensity_units())
      headerOut << "intensity_units: " << spotList->intensity_units() << tab;
   if (spotList->has_fit_mode())
      headerOut << "fit_mode: " << spotList->fit_mode() << tab;
   if (spotList->has_is_track())
      headerOut << "is_track: " << spotList->is_track();

   *ofs << headerOut.str().c_str() << "\n";
}

int TSFUtils::GetSpotBinary(std::ifstream* ifs, 
      google::protobuf::io::CodedInputStream* codedInput, TSF::Spot* spot) throw (TSFException)
{

   if (spot == NULL)
   {
      throw TSFException("Programming error: spot is not pointing to an object\n");
   }
   uint32_t mSize;
   if (!codedInput->ReadVarint32(&mSize))
   {
      throw TSFException("Failed to read Spot size");
   }

   std::string buffer;

   try {
      if (codedInput->ReadString(&buffer, mSize)) 
      {
         if (!spot->ParseFromString(buffer)) 
         {
            return NOMESSAGEFOUND;
         }
      } else
      {
         throw TSFException("Failed to read Spot\n");
      }
   } catch (...)
   {
      throw TSFException("Bad exception while reading spot data \n");
   }

   return GOOD;
}


int TSFUtils::GetSpotText(std::ifstream* ifs, TSF::Spot* spot)
{

   return GOOD;
}

/**
 * Inspects Spot spot and discovers the field names contained in the spot
 * Outputs a vector with field names
 */
void TSFUtils::GetSpotFields(TSF::Spot* spot, std::vector<std::string>& fields) throw (TSFException)
{
   if (spot == NULL)
   {
      throw TSFException("Programming error: Object Spot is a null pointer\n");
   }

   const google::protobuf::Descriptor* spotDescriptor = spot->GetDescriptor();
   const google::protobuf::Reflection* spotReflection = spot->GetReflection();
   for (int i = 0; i < spotDescriptor->field_count(); i++) 
   {
      const google::protobuf::FieldDescriptor* myField = spotDescriptor->field(i);
      if (spotReflection->HasField(*spot, myField))
      {
         fields.push_back(myField->name());
      }
   }


   /*

   fields.push_back("molecule");
   fields.push_back("channel");
   fields.push_back("frame");
   if (spot->has_slice())
      fields.push_back("slice");
   if (spot->has_pos())
      fields.push_back("pos");
   if (spot->has_x())
      fields.push_back("x");
   if (spot->has_y())
      fields.push_back("y");
   if (spot->has_z())
      fields.push_back("z");
   if (spot->has_intensity())
      fields.push_back("intensity");
   if (spot->has_background())
      fields.push_back("background");
   if (spot->has_width())
      fields.push_back("width");
   if (spot->has_a())
      fields.push_back("a");
   if (spot->has_theta())
      fields.push_back("theta");
   if (spot->has_x_precision())
      fields.push_back("x_precision");
   if (spot->has_x_position())
      fields.push_back("x_position");
   if (spot->has_y_position())
      fields.push_back("y_position");
*/
}

/**
 * Writes the field names contained in vector fields to the ofstream of.
 * The stream needs to be opened and writeable
 */
void TSFUtils::WriteSpotFields(std::ofstream* of, std::vector<std::string>& fields) throw (TSFException)
{
   if (!of->is_open())
   {
      throw TSFException ("Output file is not open\n");
   }

   for (std::vector<std::string>::iterator it = fields.begin();
         it != fields.end(); it++)
   {
      *of << *it << "\t";
   }
   *of << "\n";
}

void TSFUtils::WriteSpotBinary(std::ofstream* of, TSF::Spot* spot)
{
}


void TSFUtils::WriteSpotText(std::ofstream* of, TSF::Spot* spot, std::vector<std::string>& fields)
{
   for (std::vector<std::string>::iterator it = fields.begin();
         it != fields.end(); it++)
   {
      if (*it == "molecule" && spot->has_molecule())
         *of << spot->molecule() << "\t";
      if (*it == "channel" && spot->has_channel())
         *of << spot->channel() << "\t";
      if (*it == "frame" && spot->has_frame())
         *of << spot->frame() << "\t";
      if (*it == "slice" && spot->has_slice())
         *of << spot->slice() << "\t";
      if (*it == "pos" && spot->has_pos())
         *of << spot->pos() << "\t";
      if (*it == "x" && spot->has_x())
         *of << spot->x() << "\t";
      if (*it == "y" && spot->has_y())
         *of << spot->y() << "\t";
      if (*it == "z" && spot->has_z())
         *of << spot->z() << "\t";
      if (*it == "intensity" && spot->has_intensity())
         *of << spot->intensity() << "\t";
      if (*it == "background" && spot->has_background())
         *of << spot->background() << "\t";
      if (*it == "width" && spot->has_width())
         *of << spot->width() << "\t";
      if (*it == "a" && spot->has_a())
         *of << spot->a() << "\t";
      if (*it == "theta" && spot->has_theta())
         *of << spot->theta() << "\t";
      if (*it == "x_precision" && spot->has_x_precision())
         *of << spot->x_precision() << "\t";
      if (*it == "y_precision" && spot->has_y_precision())
         *of << spot->y_precision() << "\t";
      if (*it == "x_position" && spot->has_x_position())
         *of << spot->x_position() << "\t";
      if (*it == "y_position" && spot->has_y_position())
         *of << spot->y_position() << "\t";

   }
   *of << "\n";
}


