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

      initialized_ = true;
   } catch (...) {
      // We failed to initialize.  Rely on the initialized_ flag
   }
}

TSFParser::~TSFParser()
{
   if (input_ != 0)
      delete input_;
   if (codedInput_ != 0)
      delete codedInput_;
}
      
std::vector<std::string> TSFParser::GetFields()
{
   return fields_;
}

bool TSFParser::GetNextSpot(double* data)
{
   if (firstSpot_)
      firstSpot_ = false;
   else
      NextSpot();

   if (spot_.has_x()) {
      data[0] = spot_.x();
   }

   return false;
}


TSF::Spot TSFParser::GetNextSpot()
{
   if (firstSpot_) {
      firstSpot_ = false;
      return spot_;
   }

   return NextSpot();
}

TSF::Spot TSFParser::NextSpot()
{
   codedInput_->ReadVarint32(&mSize_);
   codedInput_->ReadString(&buffer_, mSize_); 
   spot_.ParseFromString(buffer_);
   return spot_;
}

uint64_t TSFParser::GetNrSpotsFromSpotList()
{
   if (spotList_.has_nr_spots())                                              
      return spotList_.nr_spots(); 
   return 0;
}
