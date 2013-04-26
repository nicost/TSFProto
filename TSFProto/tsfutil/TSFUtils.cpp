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


/**
 * Reads the SpotList (Header info) from a tsf file
 * Expects a freshly opened stream, i.e. the filepointer should be at the beginning
 * Reads the magic number and offset to the SpotList
 * Reads and fills the SpotList
 * SpotList should point to a valid SpotList data structure
 * returns GOOD on succes, will throw a TSFException otherwise
 * 
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


/**
 * Reads header from a text version of the tsf file
 * Header is in the form of key: value\tkey: value, etc...
 * If SpotList was not allocated, it will be created here
 * It is the responsibility of the caller to delete spotList
 */
int TSFUtils::GetHeaderText(std::ifstream* ifs, TSF::SpotList* sl) throw (TSFException)
{
   if (!ifs->is_open())
   {
      throw TSFException("Input file was not opened correctly.  Does it exist?");
   }
   if (sl == NULL)
   {
      sl = new TSF::SpotList();
   }

   const google::protobuf::Descriptor* slDescriptor = sl->GetDescriptor();
   const google::protobuf::Reflection* slReflection = sl->GetReflection();

   std::string line;
   std::getline(*ifs, line);
   std::vector<std::string> tokens = split(line, '\t');

   for (std::vector<std::string>::iterator it = tokens.begin(); 
         it != tokens.end(); ++it)
   {
      std::vector<std::string> keyValue = split(*it, ':');
      if (keyValue.size() == 2)
      {

         const google::protobuf::FieldDescriptor* fd = 
            slDescriptor->FindFieldByName(keyValue[0]);
         if (fd != NULL)
         {
            // strip whitespace from value
            std::stringstream s(keyValue[1]);
            s >> keyValue[1];
            InsertByReflection(slReflection, sl, fd, keyValue[1]);
         }
      }
   }

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

   const google::protobuf::Descriptor* spotListDescriptor = spotList->GetDescriptor();
   const google::protobuf::Reflection* spotListReflection = spotList->GetReflection();
   for (int i = 0; i < spotListDescriptor->field_count(); i++) 
   {
      const google::protobuf::FieldDescriptor* myField = spotListDescriptor->field(i);
      if (spotListReflection->HasField(*spotList, myField))
      {
         switch (myField->type() )
         {
            case google::protobuf::FieldDescriptor::TYPE_STRING:
               headerOut << myField->name() << ": " << 
                  spotListReflection->GetString(*spotList, myField) << tab;
               break;
            case google::protobuf::FieldDescriptor::TYPE_INT32:
               headerOut << myField->name() << ": " << 
                  spotListReflection->GetInt32(*spotList, myField) << tab;
               break;
            case google::protobuf::FieldDescriptor::TYPE_INT64:
               headerOut << myField->name() << ": " << 
                  spotListReflection->GetInt64(*spotList, myField) << tab;
               break;
            case google::protobuf::FieldDescriptor::TYPE_FLOAT:
               headerOut << myField->name() << ": " << 
                  spotListReflection->GetFloat(*spotList, myField) << tab;
               break;
            case google::protobuf::FieldDescriptor::TYPE_BOOL:
               headerOut << myField->name() << ": " << 
                  spotListReflection->GetBool(*spotList, myField) << tab;
               break;
            case google::protobuf::FieldDescriptor::TYPE_ENUM:
               {
                  const google::protobuf::EnumValueDescriptor* enumDescriptor = 
                     spotListReflection->GetEnum(*spotList, myField);

                  headerOut << myField->name() << ": " << 
                     enumDescriptor->name() << tab;
                  break;
               }
            default:
               throw TSFException("While parsing the header (spotList) a type was encountered that is not (yet) supported");
         }
      }
   }

   *ofs << headerOut.str().c_str() << "\n";
}

/**
 *
 */
int TSFUtils::GetSpotBinary(google::protobuf::io::CodedInputStream* codedInput, 
      TSF::Spot* spot) throw (TSFException)
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


/**
 * Reads a single line from a text file
 * Splits the line based on tabs
 * Insert data into structure spot, assuming the same order as found in 
 * fields
 * The text file needs to be opened and readable
 */
int TSFUtils::GetSpotText(std::ifstream* ifs, TSF::Spot* spot, 
      std::vector<std::string>& fields) throw (TSFException)
{
   if (!ifs->is_open())
   {
      throw TSFException("Input file is not open in function GetSpotText");
   }

   if (spot == NULL)
   {
      throw TSFException("Programming error: spot pointer is null");
   }

   const google::protobuf::Descriptor* sDescriptor = spot->GetDescriptor();
   const google::protobuf::Reflection* sReflection = spot->GetReflection();

   std::string line;
   std::getline(*ifs, line);
   if (line.size() == 0)
   {
      return EF;
   }

   std::vector<std::string> tokens = split(line, '\t');

   if (tokens.size() != fields.size())
      throw TSFException("In function GetSpotText, the number of fields in the spot read from file does not match the expected number of fields");

   for (uint32_t i = 0; i < fields.size(); i++)
   {
      const google::protobuf::FieldDescriptor* fd = 
         sDescriptor->FindFieldByName(fields[i]);
      if (fd != NULL)
      {
         std::stringstream s(tokens[i]);
         s >> tokens[i];
         InsertByReflection(sReflection, spot, fd, tokens[i]);
      }
   }

   return GOOD;
}

/**
 * Inspects Spot spot and discovers the field names contained in the spot
 * Outputs a vector with field names
 */
void TSFUtils::ExtractSpotFields(TSF::Spot* spot, std::vector<std::string>& fields) throw (TSFException)
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

}

/**
 * Reads a line from the file ifs and extracts field names
 * ifs needs to be open and readable
 */
void TSFUtils::GetSpotFields(std::ifstream* ifs,
      std::vector<std::string>& fields) throw (TSFException)
{
   if (!ifs->is_open())
   {
      throw TSFException ("Output file is not open\n");
   }

   std::string line;
   std::getline(*ifs, line);
   fields = split(line, '\t');

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

std::vector<std::string> &TSFUtils::split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> TSFUtils::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

void TSFUtils::InsertByReflection(const google::protobuf::Reflection* sr, 
      google::protobuf::Message* m, 
      const google::protobuf::FieldDescriptor* fd, std::string val)
{
   switch (fd->type() ) 
   {
      case google::protobuf::FieldDescriptor::TYPE_STRING:
            sr->SetString(m, fd, val);
         break;
      case google::protobuf::FieldDescriptor::TYPE_INT32:
         {
            std::stringstream ss(val);
            int32_t num;
            ss >> num;
            sr->SetInt32(m, fd, num);
         }
         break;
      case google::protobuf::FieldDescriptor::TYPE_INT64:
         {
            std::stringstream ss(val);
            int64_t num;
            ss >> num;
            sr->SetInt64(m, fd, num);
         }
         break;
      case google::protobuf::FieldDescriptor::TYPE_FLOAT:
         {
            std::stringstream ss(val);
            float num;
            ss >> num;
            sr->SetFloat(m, fd, num);
         }
         break;
      case google::protobuf::FieldDescriptor::TYPE_BOOL:
         {
            std::stringstream ss(val);
            bool b;
            ss >> b;
            sr->SetBool(m, fd, b);
         }
         break;
      case google::protobuf::FieldDescriptor::TYPE_ENUM:
         {
            const google::protobuf::EnumDescriptor* ed = fd->enum_type();
            const google::protobuf::EnumValueDescriptor* evd  =
               ed->FindValueByName(val);
            if (evd != NULL)
               sr->SetEnum(m, fd, evd);
         }
         break;
      default:
         throw TSFException("While parsing the message, a type was encountered that is not (yet) supported");

   }
}

