/****************************************************
 * Parser for files storing data in Tagged Spot Format
 * Written to facilitate opening TSF files in matlab
 * but should be useful in other circumstances as well
 *
 * Nico Stuurman, January 2012.  Copyright UCSF, 2012
 *
 * License: BSD-clause3: http://www.opensource.org/licenses/BSD-3-Clause
 */

#include <fstream>
#include "TSFParser.h"
#include "../../buildcpp/TSFProto.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>


TSFParser::TSFParser(std::ifstream* ifs, 
            std::vector<std::string> requestedFields) :
   requestedFields_(requestedFields),
   initialized_(false)
{
   if (ifs == 0 || ifs->fail()) {
      return;
   }

   // fill fieldMap_ and fieldFunctionMap_.  
   fieldMap_["molecule"] = 1;
   fieldFunctionMap_[1] = &TSF::Spot::has_molecule;
   fieldMap_["channel"] = 2;
   fieldFunctionMap_[2] = &TSF::Spot::has_channel;
   fieldMap_["frame"] = 3;
   fieldFunctionMap_[3] = &TSF::Spot::has_frame;
   fieldMap_["slice"] = 4;
   fieldFunctionMap_[4] = &TSF::Spot::has_slice;
   fieldMap_["pos"] = 5;
   fieldFunctionMap_[5] = &TSF::Spot::has_pos;
   //fieldMap_["location_units"] = 17;
   //fieldFunctionMap_[17] = &TSF::Spot::has_location_units;
   fieldMap_["x"] = 7;
   fieldFunctionMap_[7] = &TSF::Spot::has_x;
   fieldMap_["y"] = 8;
   fieldFunctionMap_[8] = &TSF::Spot::has_y;
   fieldMap_["z"] = 9;
   fieldFunctionMap_[9] = &TSF::Spot::has_z;
   fieldMap_["intensity_units"] = 18;
   fieldFunctionMap_[18] = &TSF::Spot::has_intensity_units;
   fieldMap_["intensity"] = 10;
   fieldFunctionMap_[10] = &TSF::Spot::has_intensity;
   fieldMap_["background"] = 11;
   fieldFunctionMap_[11] = &TSF::Spot::has_background;
   fieldMap_["width"] = 12;
   fieldFunctionMap_[12] = &TSF::Spot::has_width;
   fieldMap_["a"] = 13;
   fieldFunctionMap_[13] = &TSF::Spot::has_a;
   fieldMap_["theta"] = 14;
   fieldFunctionMap_[14] = &TSF::Spot::has_theta;
   fieldMap_["flag"] = 6;
   fieldFunctionMap_[6] = &TSF::Spot::has_flag;
   fieldMap_["x_original"] = 101;
   fieldFunctionMap_[101] = &TSF::Spot::has_x_original;
   fieldMap_["y_original"] = 102;
   fieldFunctionMap_[102] = &TSF::Spot::has_y_original;
   fieldMap_["z_original"] = 103;
   fieldFunctionMap_[103] = &TSF::Spot::has_z_original;
   fieldMap_["x_precision"] = 104;
   fieldFunctionMap_[104] = &TSF::Spot::has_x_precision;
   fieldMap_["y_precision"] = 105;
   fieldFunctionMap_[105] = &TSF::Spot::has_y_precision;
   fieldMap_["z_precision"] = 106;
   fieldFunctionMap_[106] = &TSF::Spot::has_z_precision;
   fieldMap_["x_position"] = 107;
   fieldFunctionMap_[107] = &TSF::Spot::has_x_position;
   fieldMap_["y_position"] = 108;
   fieldFunctionMap_[108] = &TSF::Spot::has_y_position;

   try {
      input_ = new google::protobuf::io::IstreamInputStream(ifs);
      codedInput_ = new google::protobuf::io::CodedInputStream(input_);

      uint32_t mSize;
      codedInput_->ReadVarint32(&mSize);

      // This seems the best approach, but it crashes with protocolbuffers 2.3  
      // spotList_.ParseFromBoundedZeroCopyStream(&input, mSize); 

      // This seems dangerous, not sure what it does with 0s, however, it works
      std::string buffer; 
      codedInput_->ReadString(&buffer, mSize);
      if (!spotList_.ParseFromString(buffer))
         return;

      // Read the first spot into variable spot_
      NextSpot();

      // so that we can check which of the requested fields are present
      checkFields(requestedFields);

      initialized_ = true;
   } catch (...) {
      // We failed to initialize.  Rely on the initialized_ flag
   }
}

TSFParser::~TSFParser()
{
   if (input_ != 0)
      delete input_;

   // calling the codedInput destructor causes a crash
   //if (codedInput_ != 0)
   //   delete codedInput_;
}
      
std::vector<std::string> TSFParser::GetFields()
{
   return fields_;
}

bool TSFParser::checkFields(std::vector<std::string> requestedFields)
{
   std::vector<std::string>::iterator it;
   std::map<std::string, int>::iterator itm;
   std::map<int, has_function>::iterator itmf;

   for (it=requestedFields.begin(); it < requestedFields.end(); it++)
   {
      itm = fieldMap_.find(*it);
      if (itm != fieldMap_.end()) {
         itmf = fieldFunctionMap_.find(itm->second);
         if ( itmf != fieldFunctionMap_.end()) {
            has_function mhf = itmf->second;
            if ( ((spot_).*mhf)() ) 
            {
               fields_.push_back(itm->first);
               fieldsNumeric_.push_back(itm->second);
            }
         }
      }
   }
}

bool TSFParser::GetNextSpot(double* data)
{
   if (!initialized_)
      return false;

   if (firstSpot_)
      firstSpot_ = false;
   else
      if (!NextSpot())
         return false;

   std::vector<int>::iterator it;

   uint64_t i = 0;
   for (it=fieldsNumeric_.begin(); it < fieldsNumeric_.end(); it++)
   {
      // Unelegant to write out the functions, but since the signatures
      // of the called function are different, I do not think that 
      // a map of function pointers can be used
      switch(*it)
      {
         case 1:
            data[i] = spot_.molecule();
            break;
         case 2:
            data[i] = spot_.channel();
            break;
         case 3:
            data[i] = spot_.frame();
            break;
         case 4:
            data[i] = spot_.slice();
            break;
         case 5:
            data[i] = spot_.pos();
            break;
         case 7:
            data[i] = spot_.x();
            break;
         case 8:
            data[i] = spot_.y();
            break;
         case 9:
            data[i] = spot_.z();
            break;
         case 10:
            data[i] = spot_.intensity();
            break;
         case 11:
            data[i] = spot_.background();
            break;
         case 12:
            data[i] = spot_.width();
            break;
         case 13:
            data[i] = spot_.a();
            break;
         case 14:
            data[i] = spot_.theta();
            break;
         case 6:
            data[i] = spot_.flag();
            break;
         case 101:
            data[i] = spot_.x_original();
            break;
         case 102:
            data[i] = spot_.y_original();
            break;
         case 103:
            data[i] = spot_.z_original();
            break;
         case 104:
            data[i] = spot_.x_precision();
            break;
         case 105:
            data[i] = spot_.y_precision();
            break;
         case 106:
            data[i] = spot_.z_precision();
            break;
         case 107:
            data[i] = spot_.x_position();
            break;
         case 108:
            data[i] = spot_.y_position();
            break;
      }
      //i+=GetNrSpotsFromSpotList();
      i++;

   }

   return true;
}


TSF::Spot TSFParser::GetNextSpot()
{
   if (firstSpot_) 
      firstSpot_ = false;
   else
      NextSpot();

   return spot_;
}

bool TSFParser::NextSpot()
{
   if (!codedInput_->ReadVarint32(&mSize_))
      return false;
   if (mSize_ <= 0)
      return false;
   if (!codedInput_->ReadString(&buffer_, mSize_))
      return false; 
   if (spot_.ParseFromString(buffer_))
      return true;
   return false;
}

uint64_t TSFParser::GetNrSpotsFromSpotList()
{
   if (spotList_.has_nr_spots())                                              
      return spotList_.nr_spots(); 
   return 0;
}
