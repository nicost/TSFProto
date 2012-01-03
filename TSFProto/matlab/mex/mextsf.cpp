/*************************************************************
 * Matlab interface to Tagged Spot Format
 *
 * Nico Stuurman, January 2012.  Copyright UCSF, 2012
 *
 * License: BSD-clause3: http://www.opensource.org/licenses/BSD-3-Clause
 *
 */

#include "mex.h"
#include <stdint.h>
#include <vector>
#include <fstream>
#include <sstream>
#include "TSFParser.h"
#include "../../buildcpp/TSFProto.pb.h"
#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>



/**
 * Matlab Gateway function for mextsf 
 * Two inputs:
 * filename - path to TSF proto data
 * Array with requested field names - 
 *
 * Two outputs will be provided:
 * Array with field names (not all requested fields may be present)
 * Array with data
 */
void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
   const std::string ourName = "mextsf";
   char *fileName[1];
   std::vector<std::string> fieldNames; 
   mxArray *tmp;

   if (nrhs < 2) {
       mexErrMsgIdAndTxt("MATLAB:mexcpp:nargin", 
             (ourName + " requires two input arguments.").c_str());
   }
   if (nlhs != 2) {
      mexErrMsgIdAndTxt("MATLAB:mexcpp:nargout",
            (ourName + " requires two output arguments.").c_str());
   }

   // Get first input: fileName
   if (!mxIsChar(prhs[0])) {
      mexErrMsgIdAndTxt("MATLAB:mexcpp:nargin",
            (ourName + "First input should be path to input file.").c_str());
   }
   fileName[0] =  mxArrayToString(prhs[0]);

   std::ifstream ifs (fileName[0]);

   if (ifs == 0 || ifs.fail()) {
      std::string msg = "Failed to open file.";
      mexErrMsgIdAndTxt("MATLAB:mexcpp:nargin", msg.c_str());
   }

   // Get second input: cell array of fieldNames
   if(!mxIsCell(prhs[1])) {
      mexErrMsgIdAndTxt("MATLAB:mexcpp:nargin",
            (ourName + "Second input should be cell array of field names.").c_str());
   }

   mwSize NStructElems = mxGetNumberOfElements(prhs[1]);
   for (mwSize i = 0; i < NStructElems; i++) {
      tmp = mxGetFieldByNumber(prhs[1], 0, i);
      if (tmp != NULL && mxIsChar(tmp)) {
         fieldNames.push_back(mxArrayToString(tmp));
      }
   }

   // Create parser object
   TSFParser tsfP(&ifs, fieldNames);


   // Set first output argument (field names)
   std::vector<std::string> fieldsFound = tsfP.GetFields();
   plhs[0] = mxCreateCellMatrix(1, fieldsFound.size());
   for (int i=0; i < fieldsFound.size(); i++) {
      char *str[1];
      str[0] = (char *) malloc(sizeof(fieldsFound[i].c_str()));
      strcpy(str[0], fieldsFound[i].c_str());
      mxSetCell(plhs[0], i, mxCreateCharMatrixFromStrings(1, (const char **) str));
   }

   uint64_t nrSpots = tsfP.GetNrSpotsFromSpotList();;

   if (nrSpots > 0) {
      // nrSpots known in advance, this is most efficient
      plhs[1] = mxCreateNumericMatrix(fieldsFound.size(), nrSpots, mxDOUBLE_CLASS, mxREAL);
      double * pointer = mxGetPr(plhs[1]);
      for (uint64_t i=0; i < nrSpots; i++) {
         tsfP.GetNextSpot(&pointer[i * fieldsFound.size()]);
      }
   } else {
      // nrSpots is not known in advance.  This is less efficient
      uint64_t counter = 0;
      std::vector<double*> p;
      p.reserve(264000);
      bool success = true;
      while (success) {
         p.push_back((double*) mxCalloc(fieldsFound.size(), sizeof(double)));
         success = tsfP.GetNextSpot(p.back());
         counter++;
      }
      plhs[1] = mxCreateNumericMatrix(fieldsFound.size(), counter, mxDOUBLE_CLASS, mxREAL);
      double* output = mxGetPr(plhs[1]);
      for (uint64_t i=0; i < counter; i++) {
         memcpy(output + i * fieldsFound.size(), p[i], fieldsFound.size() * sizeof(double));
      }
      // TODO: mirror the array
   }

   /* useful to print state of variables
   std::ostringstream os;
   os << "Nr of Spots: " << nrSpots;
   mexErrMsgIdAndTxt("MATLAB:mexcpp:nargin", os.str().c_str());
   */

}
