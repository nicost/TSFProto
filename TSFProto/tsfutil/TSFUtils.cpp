/**
 * Utility functions for reading and writing Tagged Spot Format files
 *
 * This class contains static functions for reading and writing to text files
 * but should be instantiated to read and write binary filed
 * For each input or output file a seperate instance should be created and 
 * the enum "mode" in the constructor indicates whether this is a read or write 
 * instance.  When reading, the GetHeaderBinary function should be called before 
 * reading the individual spots, when writing, the individual spots should be 
 * written before calling the function WriteHeaderBinary
 *
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
 * Constructor
 * Decides whether this object is going to be used for input or output
 * Instantiation of this calss is only needed for binary (tsf) files
 * Text files can be written using static methods
 */
TSFUtils::TSFUtils(std::fstream* fs, mode mode) throw (TSFException) :
   mode_ (mode),
   fs_ (fs),
   firstWrite_(true)
{
   if (fs == NULL || !fs->is_open())
      throw TSFException("File is not open");
   // even though we only need an ifstream for reading
   // we need the write flag set to be able to seek to a previous position
   // Not sure how to test for that...

   if (mode_ == READ) 
   {
   }
   if (mode_ == WRITE)
   {
      output_ = new google::protobuf::io::OstreamOutputStream(fs_);
      codedOutput_ = new google::protobuf::io::CodedOutputStream(output_);
   }
}

TSFUtils::~TSFUtils()
{
   if (mode_ == READ)
   {
      delete input_;
      delete codedInput_;
      delete output_;
      delete codedOutput_;
   }
}

/**
 * Gets the header information from a tsf file
 * After succefully obtaining the spotlist, resets the 
 * file pointer to the first spot
 */
int TSFUtils::GetHeaderBinary(TSF::SpotList* sl) throw (TSFException)
{
   if (mode_ != READ)
      throw TSFException("TSFUtils was opened in write-mode.");

   if (sl == NULL)
      throw TSFException("Programming error: SpotList pointer was NULL");

   if (!fs_->good())
      throw TSFException("Input file stream is in a bad state...");

   int32_t magic = ReadInt32(fs_); 

   if (magic != 0) 
   {
      throw TSFException("Magic number is not 0, is this a tsf file?");
   }

   if (!fs_->good())
   {
      throw TSFException("Input file stream is in a bad state...");
   }

   int64_t offset = ReadInt64(fs_);

   if (offset == 0)
   {
      throw TSFException("Offset is 0, can not find header data in this file");
   }

   fs_->seekg(offset, std::ios_base::cur);

   input_ = new google::protobuf::io::IstreamInputStream(fs_);
   codedInput_ = new google::protobuf::io::CodedInputStream(input_);

   uint32_t mSize;
   if (!codedInput_->ReadVarint32(&mSize))
   {
      throw TSFException("Failed to read SpotList offset");
   }

   std::string buffer;

   if (codedInput_->ReadString(&buffer, mSize))                           
   {
      sl->ParseFromString(buffer);
   } else
   {
      throw TSFException("Failed to read SpotList data");
   }

   // we need to set the read caret back to byte 12
   // the protobuf utilities do not seem to support this
   // Therefore, delete them, set the file pointer to 12
   // make sure this worked and recreate the codedInputStream

   delete codedInput_;
   delete input_;
   
   fs_->clear();
   fs_->seekg(12, std::ios_base::beg);
   
   if (12 != fs_->tellg())
      throw TSFException ("Failed to set filepointer.  Try setting the read flag");

   input_ = new google::protobuf::io::IstreamInputStream(fs_);
   codedInput_ = new google::protobuf::io::CodedInputStream(input_);

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


/**
 * Writes the Header (SpotList) to a tsf file
 * The fstream need to be opened in read/write mode so that the seek back to  
 * the beginning of the file will succeed
 */
void TSFUtils::WriteHeaderBinary(TSF::SpotList* spotList) throw (TSFException)
{
   // output stream must be at insertion point for the header
   // (i.e., after all spots have been inserted)
   // register the offset, write the length as a varint, then go back to the
   // beginning of the stream
   
   int64_t offset = codedOutput_->ByteCount();

   std::string data;
   spotList->SerializeToString(&data);
   codedOutput_->WriteVarint32((int) data.length());
   codedOutput_->WriteRaw(data.c_str(), data.length());

   // Need to delete these objects to flush their content to disk
   delete codedOutput_;
   delete output_;

   fs_->seekp(4, std::ios_base::beg);

   if (fs_->tellg() != 4)
   {
      throw TSFException ("Failed to reset file pointer");
   }

   if (fs_->good())
      WriteInt64(fs_, offset - 12);
   else 
      throw TSFException("Failed to write header offset.  Output file invalid");

}

/**
 * Writes the Header (SpotList) to a text file in key: value format
 * This is implemented using reflection
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
int TSFUtils::GetSpotBinary(TSF::Spot* spot) throw (TSFException)
{
   if (mode_ != READ)
      throw TSFException("TSFUtils was opened in write-mode.");

   if (spot == NULL)
      throw TSFException("Programming error: spot is not pointing to an object\n");

   if (codedInput_ == NULL)
      throw TSFException("Programming error: Always first call GetHeaderBinary before this function");

   uint32_t mSize;
   if (!codedInput_->ReadVarint32(&mSize))
   {
      throw TSFException("Failed to read Spot size");
   }

   std::string buffer;

   try {
      if (codedInput_->ReadString(&buffer, mSize)) 
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

void TSFUtils::WriteSpotBinary(TSF::Spot* spot)
{
   if (mode_ != WRITE)
      throw TSFException ("TSFUtils was not opened in write mode");

   if (firstWrite_)
   {
      codedOutput_->Skip(12);
      firstWrite_ = false;
   }

   if (codedOutput_ == NULL)
      throw TSFException ("Can not wite spot data after header was written");

   std::string data;
   spot->SerializeToString(&data);
   codedOutput_->WriteVarint32(data.length());
   codedOutput_->WriteRaw(data.c_str(), data.length());
}


/**
 * Write a single spot to a line in a text file
 * The fields being output and their order is determined
 * by the vector "fields" that should contain fieldnames
 */
void TSFUtils::WriteSpotText(std::ofstream* of, TSF::Spot* spot, std::vector<std::string>& fields)
{
   const google::protobuf::Descriptor* sd = spot->GetDescriptor();
   const google::protobuf::Reflection* sr = spot->GetReflection();

   for (std::vector<std::string>::iterator it = fields.begin();
         it != fields.end(); it++)
   {
      const google::protobuf::FieldDescriptor* fd = 
         sd->FindFieldByName(*it);
      if (fd != NULL && sr->HasField(*spot, fd))
      {
         switch (fd->type()) 
         {
            case google::protobuf::FieldDescriptor::TYPE_STRING:  
               *of << sr->GetString(*spot, fd) << "\t";
               break;
            case google::protobuf::FieldDescriptor::TYPE_INT32:
               *of << sr->GetInt32(*spot, fd) << "\t";
               break;
            case google::protobuf::FieldDescriptor::TYPE_INT64:
               *of << sr->GetInt64(*spot, fd) << "\t";
               break;
            case google::protobuf::FieldDescriptor::TYPE_UINT32:
               *of << sr->GetUInt32(*spot, fd) << "\t";
               break;
            case google::protobuf::FieldDescriptor::TYPE_UINT64:
               *of << sr->GetUInt64(*spot, fd) << "\t";
               break;
            case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
               *of << sr->GetDouble(*spot, fd) << "\t";
               break;
            case google::protobuf::FieldDescriptor::TYPE_FLOAT:
               *of << sr->GetFloat(*spot, fd) << "\t";
               break;
            default:
               throw TSFException("This Field type is not yet supported in the WriteSpotText function"); 
         }
      }
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

/**
 * Reads a 32bit int from a codedInputStream
 * Converts to little endian on a little endian platform
 * Shoulds only be used when big endian numbers are expected
 */
int32_t TSFUtils::ReadInt32(google::protobuf::io::CodedInputStream* ci)
{
   int32char tmp;
   ci->ReadRaw(tmp.ch, 4);
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
 * Reads a 64bit int from a codedInputStream
 * Converts to little endian on a little endian platform
 * Shoulds only be used when big endian numbers are expected
 */
int64_t TSFUtils::ReadInt64(google::protobuf::io::CodedInputStream* ci)
{
   int64char tmp;
   ci->ReadRaw(tmp.ch, 8);
   if (!IsBigEndian())
   {
      tmp.i = SwapInt64(tmp.i);
   }
   return tmp.i;
}

// Writes a signed 64bit big endian int to a stream afterconverts to little endian 
// if needed
void TSFUtils::WriteInt64(std::ostream *ofs, int64_t i) throw (TSFException)
{
   int64char tmp;
   if (!IsBigEndian())
   {
      tmp.i = SwapInt64(i);
   } else
   {
      tmp.i = i;
   }
   ofs->write(tmp.ch, 8);
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

