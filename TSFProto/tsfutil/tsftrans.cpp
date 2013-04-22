/**
 * tsfutil
 *
 * Example CLI application showing how TSF format files can be read and written
 * in C++.  
 *
 * Nico Stuurman, nico.stuurman at ucsf.edu
 *
 * Copyright UCSF, 2013
 */


#include <stdio.h>


#include "TSFUtils.cpp"


void usage (int argc, const char* argv[])
{
   printf("Usage: %s inputfile outputfile\n", argv[0]);
   printf("Output and input must have .txt or .tsf extension\n");
}


int main (int argc, const char*  argv[])
{
   if (argc != 3) 
   {
      usage(argc, argv);
      return 1;
   }

   const char* inputFile = argv[1];
   const char* outputFile = argv[2];

   const char* textExt = ".txt";
   const char* binaryExt = ".tsf";

   // Check for extensions, making sure we only look the last extension
   bool inputText = long ( strstr(inputFile, textExt) - inputFile) == strlen(inputFile)  - 4;
   bool inputBinary = long ( strstr(inputFile, binaryExt) - inputFile) == strlen(inputFile)  - 4;
   bool outputText = long ( strstr(outputFile, textExt) - outputFile) == strlen(outputFile)  - 4;
   bool outputBinary = long ( strstr(outputFile, binaryExt) - outputFile) == strlen(outputFile)  - 4;

   if (inputText)
      printf("Text input file\n");
   if (inputBinary)
      printf("Binary input file\n");
   if (outputText)
      printf("Text output file\n");
   if (outputBinary)
      printf("Binary output file\n");

   if (! (inputText || inputBinary) )
   {
      printf("Input file should have either the .txt or .txf extension\n");
      return 1;
   }

   if (! (outputText || outputBinary) )
   {
      printf("Output file should have either the .txt or .txf extension\n");
      return 1;
   }


   std::ifstream ifs;

   try {
      if (inputBinary)
      {
         ifs.open(inputFile, std::ios_base::in | std::ios_base::binary);
         TSF::SpotList sl = TSFUtils::GetHeaderBinary(&ifs);
      }
   } catch (TSFException ex) 
   {
      std::cout << ex.getMessage().c_str() << std::endl;
      return 1;
   } catch (...)
   {
      std::cout << "Some exceptions occurred.  Exciting now...\n";
   }


   return 0;
}

