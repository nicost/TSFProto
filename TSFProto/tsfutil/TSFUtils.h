/**
 * Utility functions for reading and writing Tagged Spot Format files
 *
 * Nico Stuurman, nico.stuurman at ucsf.edu
 *
 * April 20, 2013
 *
 * Copyright UCSF, 2013
 */

#ifndef TSFUTILS_H
#define TSFUTILS_H

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <iostream>
#include <vector>
#include "../buildcpp/TSFProto.pb.h"
#include "TSFException.h"


class TSFUtils
{
   public:
      enum mode {
         READ = 0,
         WRITE = 1
      };


      TSFUtils(std::fstream* fs, mode mode) throw (TSFException);
      ~TSFUtils();

      int GetHeaderBinary(TSF::SpotList* sl) throw (TSFException);
      int GetSpotBinary(TSF::Spot* spot) throw (TSFException);

      void WriteSpotBinary(TSF::Spot* spot);
      void WriteHeaderBinary(TSF::SpotList* sl) throw (TSFException);


      static int GetHeaderText(std::ifstream* ifs, TSF::SpotList* sl) throw (TSFException);
      static void GetSpotFields(std::ifstream* ifs, std::vector<std::string>& fields) 
         throw (TSFException);
      static int GetSpotText(std::ifstream* ifs, TSF::Spot* spot, 
            std::vector<std::string>& fields) throw (TSFException);

      static void WriteHeaderText(std::ofstream* ofs, TSF::SpotList* sl) throw (TSFException);
      static void WriteSpotFields(std::ofstream* of, std::vector<std::string>& fields) 
         throw (TSFException);
      static void WriteSpotText(std::ofstream* of, TSF::Spot* spot, 
            std::vector<std::string>& fields);

      static void ExtractSpotFields(TSF::Spot* spot, std::vector<std::string>& fields) 
         throw (TSFException);


      static const int GOOD = 0;
      static const int BAD = 1;
      static const int NOMESSAGEFOUND = 2;
      static const int EF = 3;

      // Following are function used internally
      // since they are static, they may be useful to others as well..

      static void InsertByReflection(const google::protobuf::Reflection* sr,
         google::protobuf::Message* m, const google::protobuf::FieldDescriptor* fd, 
         std::string val);

      union int32char {
         char ch[4];
         int32_t i;
      };

      union int64char {
         char ch[8];
         int64_t i;
      };

      static int32_t ReadInt32(std::istream *ifs) throw (TSFException);
      static int64_t ReadInt64(std::istream *ifs) throw (TSFException);
      static int32_t ReadInt32(google::protobuf::io::CodedInputStream* ci);
      static int64_t ReadInt64(google::protobuf::io::CodedInputStream* ci);

      static int32_t SwapInt32(int32_t val);
      static int64_t SwapInt64(int64_t val);
      static void WriteInt64(std::ostream *ofs, int64_t i) throw (TSFException);
      inline static bool IsBigEndian(void) 
      {
          union {
              uint32_t i;
              char c[4];
          } b = {0x01020304};

          return b.c[0] == 1; 
      };
      static std::vector<std::string> &split(const std::string &s, char delim, 
            std::vector<std::string> &elems);
      static std::vector<std::string> split(const std::string &s, char delim);

   private:
      mode mode_;
      std::fstream* fs_;
      bool firstWrite_;
      google::protobuf::io::IstreamInputStream* input_;
      google::protobuf::io::CodedInputStream* codedInput_;
      google::protobuf::io::ZeroCopyOutputStream* output_;
      google::protobuf::io::CodedOutputStream* codedOutput_;
};

#endif
