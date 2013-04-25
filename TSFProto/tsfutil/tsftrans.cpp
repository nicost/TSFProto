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
#include <google/protobuf/io/zero_copy_stream_impl.h>


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
   bool inputText = 
      (unsigned long) ( strstr(inputFile, textExt) - inputFile) == strlen(inputFile)  - 4;
   bool inputBinary = 
      (unsigned long) ( strstr(inputFile, binaryExt) - inputFile) == strlen(inputFile)  - 4;
   bool outputText = 
      (unsigned long) ( strstr(outputFile, textExt) - outputFile) == strlen(outputFile)  - 4;
   bool outputBinary = 
      (unsigned long) ( strstr(outputFile, binaryExt) - outputFile) == strlen(outputFile)  - 4;

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
   std::ofstream ofs;
   TSF::SpotList* sl = new TSF::SpotList();
   TSF::Spot* spot = new TSF::Spot;

   google::protobuf::io::IstreamInputStream* input = NULL;
   google::protobuf::io::CodedInputStream* codedInput = NULL;

   // Silence Protocol Buffer warnings
   google::protobuf::LogSilencer* ls = new google::protobuf::LogSilencer();
         
   try {
      if (inputBinary)
      {
         ifs.open(inputFile, std::ios_base::in | std::ios_base::binary);
         
         TSFUtils::GetHeaderBinary(&ifs, sl);

         // not sure why but I need to close and reopen the file to get
         // to the first spot positions
         ifs.close();

         ifs.open(inputFile, std::ios_base::in | std::ios_base::binary);
         ifs.seekg(12);
         input = new google::protobuf::io::IstreamInputStream(&ifs);
         codedInput = new google::protobuf::io::CodedInputStream(input);

         if (outputText)
         {
            // truncate and open outputfile in text mode
            // TODO: test for existence of file and warn user
            ofs.open(outputFile, std::ios_base::out | std::ios_base::trunc);
            TSFUtils::WriteHeaderText(&ofs, sl);

            int ret = TSFUtils::GetSpotBinary(&ifs, codedInput, spot);
            std::vector<std::string> fields;

            TSFUtils::GetSpotFields(spot, fields);
            TSFUtils::WriteSpotFields(&ofs, fields);
            TSFUtils::WriteSpotText(&ofs, spot, fields);

            int counter = 0;
            while (ret == TSFUtils::GOOD)
            {
               ret = TSFUtils::GetSpotBinary(&ifs, codedInput, spot);
               if (ret == TSFUtils::GOOD)
                  TSFUtils::WriteSpotText(&ofs, spot, fields);
               counter++;
            }
            std::cout << "Found " << counter << " spots\n";
         }

         delete sl;
         delete spot;

      }
   } catch (TSFException ex) 
   {
      std::cout << ex.getMessage().c_str() << std::endl;
      if (ifs.is_open())
      {
         ifs.close();
      }
      return 1;
   } catch (...)
   {
      std::cout << "Some exceptions occurred.  Exciting now...\n";
   }

   delete ls;




   return 0;
}

