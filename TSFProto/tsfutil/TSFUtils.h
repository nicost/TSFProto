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
      static TSF::SpotList GetHeaderText(std::ifstream ifs);
      static void WriteHeaderBinary(std::ofstream ofs, TSF::SpotList);
      static void WriteHeaderText(std::ofstream ofs, TSF::SpotList);

      static TSF::Spot GetSpotBinary(std::ifstream ifs);

      static void WriteSpotBinary(std::ofstream of);

      union int32char {
         char ch[4];
         uint32_t i;
      };

      union int64char {
         char ch[8];
         uint64_t i;
      };

      static uint32_t ReadInt32(std::istream *ifs) throw (TSFException);
      static uint64_t ReadInt64(std::istream *ifs) throw (TSFException);
};

#endif
