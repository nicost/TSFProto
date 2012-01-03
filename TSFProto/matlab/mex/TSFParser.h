#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "../../buildcpp/TSFProto.pb.h"
#include <vector>

class TSFParser
{
   public:
      TSFParser(std::ifstream* ifs,
            std::vector<std::string> requestedFields);
      ~TSFParser();

      std::vector<std::string> GetFields();
      bool GetNextSpot(double* data);
      TSF::Spot GetNextSpot();
      TSF::SpotList GetSpotList() { return spotList_; };
      uint64_t GetNrSpotsFromSpotList();

   private:
      TSF::Spot NextSpot();

      bool initialized_;
      bool firstSpot_;
      TSF::SpotList spotList_;
      TSF::Spot spot_;
      std::string buffer_;
      uint32_t mSize_;
      std::vector<std::string> fields_;
      std::vector<std::string> requestedFields_;
      std::vector<int> fieldsNumeric_;
      google::protobuf::io::IstreamInputStream* input_;
      google::protobuf::io::CodedInputStream* codedInput_;

};
