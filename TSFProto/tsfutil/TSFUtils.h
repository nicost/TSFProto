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

      static int GetHeaderBinary(std::ifstream* ifs, TSF::SpotList* sl) throw (TSFException);
      static int GetHeaderText(std::ifstream* ifs, TSF::SpotList* sl) throw (TSFException);
      static void WriteHeaderBinary(std::ofstream* ofs, TSF::SpotList* sl) throw (TSFException);
      static void WriteHeaderText(std::ofstream* ofs, TSF::SpotList* sl) throw (TSFException);

      static int GetSpotBinary(std::ifstream* ifs,
          google::protobuf::io::CodedInputStream* codedInput, TSF::Spot* spot) throw (TSFException);
      static int GetSpotText(std::ifstream* ifs, TSF::Spot* spot);
      static void WriteSpotBinary(std::ofstream* of, TSF::Spot* spot);
      static void GetSpotFields(TSF::Spot* spot, std::vector<std::string>& fields) throw (TSFException);
      static void WriteSpotFields(std::ofstream* of, std::vector<std::string>& fields) throw (TSFException);
      static void WriteSpotText(std::ofstream* of, TSF::Spot* spot, std::vector<std::string>& fields);

      static const int GOOD = 0;
      static const int BAD = 1;
      static const int NOMESSAGEFOUND = 2;

      // Following are function used internally
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
      static int32_t SwapInt32(int32_t val);
      static int64_t SwapInt64(int64_t val);
      inline
      static bool IsBigEndian(void) 
      {
          union {
              uint32_t i;
              char c[4];
          } b = {0x01020304};

          return b.c[0] == 1; 
      };

};

#endif
