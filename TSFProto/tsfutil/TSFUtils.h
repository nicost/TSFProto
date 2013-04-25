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

#include "../buildcpp/TSFProto.pb.h"
#include <iostream>
#include "TSFException.h"


class TSFUtils
{
   public:

      static TSF::SpotList GetHeaderBinary(std::ifstream* ifs) throw (TSFException);
      static TSF::SpotList GetHeaderText(std::ifstream* ifs);
      static void WriteHeaderBinary(std::ofstream* ofs, TSF::SpotList);
      static void WriteHeaderText(std::ofstream* ofs, TSF::SpotList);

      static TSF::Spot GetSpotBinary(std::ifstream* ifs);
      static void WriteSpotBinary(std::ofstream* of);

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
