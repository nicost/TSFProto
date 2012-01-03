/****************************************************
 * Parser for files storing data in Tagged Spot Format
 * Written to facilitate opening TSF files in matlab
 * but should be useful in other circumstances as well
 *
 * Nico Stuurman, January 2012.  Copyright UCSF, 2012
 *
 * License: BSD-clause3: http://www.opensource.org/licenses/BSD-3-Clause
 */

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

      /**
       * Returns data as doubles
       * It is the responsibility of the caller to allocate
       * enough memory in the pointer data such that all fields
       * in map "fields" can be returned
       */
      bool GetNextSpot(double* data);

      TSF::Spot GetNextSpot();
      TSF::SpotList GetSpotList() { return spotList_; };
      uint64_t GetNrSpotsFromSpotList();

   private:
      bool NextSpot();
      bool checkFields(std::vector<std::string> requestedFields);

      bool initialized_;
      bool firstSpot_;
      std::map<std::string, int> fieldMap_;
      typedef bool (TSF::Spot::*has_function)() const;
      std::map<int, has_function> fieldFunctionMap_;
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
