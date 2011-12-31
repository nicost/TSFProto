#include "mex.h"
#include <vector>
#include "../../buildcpp/TSFProto.pb.h"




// Gateway function
/**
 * We expect two or three inputs:
 * filename - path to TSF proto data
 * Array with requested field names - 
 * maximum number of records to be returned (optional)
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

   plhs[0] = mxCreateCharMatrixFromStrings((mwSize) 1, (const char **) fileName);

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


   char msg[250];
   sprintf(msg, "NStructElems: %d\n", (int) NStructElems);
   mexErrMsgTxt(msg);

}
