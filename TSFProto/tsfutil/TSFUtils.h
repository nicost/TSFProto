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

};

#endif
