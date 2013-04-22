/**
 * Exceptoin class for use in the Tagged Post File Format Utility
 *
 * Nico Stuurman, nico.stuurman at ucsf.edu
 *
 * Copyright UCSf, 2013
 *
 * April 20, 2013
 */

#ifndef TSFEXCEPTION_H
#define TSFEXCEPTION_H

#include <exception>

class TSFException: public std::exception
{
   public:

   TSFException(std::string msg)
   {
     msg_ = msg;
   };

   ~TSFException() throw () {};

   std::string getMessage() { return msg_;};

   private:
      std::string msg_;
};

#endif
