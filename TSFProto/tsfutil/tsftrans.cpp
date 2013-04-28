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


   TSF::SpotList* sl = new TSF::SpotList();
   TSF::Spot* spot = new TSF::Spot;

   // Silence Protocol Buffer warnings
   google::protobuf::LogSilencer* ls = new google::protobuf::LogSilencer();
         
   try {
      if (inputBinary)
      {
         std::fstream ifs;
         ifs.open(inputFile, std::ios_base::in | std::ios_base::out | std::ios_base::binary);

         TSFUtils* tsfIn = new TSFUtils(&ifs, TSFUtils::READ);
         
         tsfIn->GetHeaderBinary(sl);

         if (outputText)
         {
            // truncate and open outputfile in text mode
            // TODO: test for existence of file and warn user
            std::ofstream ofs;
            ofs.open(outputFile, std::ios_base::out | std::ios_base::trunc);
            TSFUtils::WriteHeaderText(&ofs, sl);

            int ret = tsfIn->GetSpotBinary(spot);
            std::vector<std::string> fields;

            TSFUtils::ExtractSpotFields(spot, fields);
            TSFUtils::WriteSpotFields(&ofs, fields);
            TSFUtils::WriteSpotText(&ofs, spot, fields);

            unsigned long counter = 0;
            while (ret == TSFUtils::GOOD)
            {
               ret = tsfIn->GetSpotBinary(spot);
               if (ret == TSFUtils::GOOD)
                  TSFUtils::WriteSpotText(&ofs, spot, fields);
               counter++;
               if (counter % 100000 == 0)
               {
                  std::cout << ".";
                  std::cout.flush();
               }
            }
            std::cout << "Wrote " << counter << " spots\n";
            ofs.close();
         } else if (outputBinary)
         {
            std::fstream fs; 
            fs.open(outputFile, std::ios_base::out | std::ios_base::trunc | 
                  std::ios_base::binary);
            TSFUtils* tsfOut = new TSFUtils(&fs, TSFUtils::WRITE);

            unsigned long counter = 0;
            while (tsfIn->GetSpotBinary(spot) == TSFUtils::GOOD)
            {
               tsfOut->WriteSpotBinary(spot);
               counter++;
               if (counter % 100000 == 0)
               {
                  std::cout << ".";
                  std::cout.flush();
               }
            }

            std::cout << "Wrote " << counter << " spots\n";
            tsfOut->WriteHeaderBinary(sl);
            fs.close();
         }
         ifs.close();

      } else if (inputText)
      { 
         std::ifstream ifs;
         ifs.open(inputFile, std::ios_base::in);
         
         TSFUtils::GetHeaderText(&ifs, sl);
         std::vector<std::string> fields;
         TSFUtils::GetSpotFields(&ifs, fields);

         if (outputText)
         {
            // truncate and open outputfile in text mode
            // TODO: test for existence of file and warn user
            std::ofstream ofs;
            ofs.open(outputFile, std::ios_base::out | std::ios_base::trunc);
            TSFUtils::WriteHeaderText(&ofs, sl);
            TSFUtils::WriteSpotFields(&ofs, fields);

            int counter = 0;
            while (TSFUtils::GetSpotText(&ifs, spot, fields) == TSFUtils::GOOD)
            {
               // write the spots out
               TSFUtils::WriteSpotText(&ofs, spot, fields);
               counter++;
               if (counter % 100000 == 0)
               {
                  std::cout << ".";
                  std::cout.flush();
               }
            }
            std::cout << "Found " << counter << " spots\n";

            ifs.close();
            ofs.close();
         } else if (outputBinary)
         {
            std::fstream fs; 
            fs.open(outputFile, std::ios_base::out | std::ios_base::trunc | 
                  std::ios_base::binary);

            TSFUtils* tsfOut = new TSFUtils(&fs, TSFUtils::WRITE);

            unsigned long counter = 0;
            while (TSFUtils::GetSpotText(&ifs, spot, fields) == TSFUtils::GOOD)
            {
               tsfOut->WriteSpotBinary(spot);
               counter++;
               if (counter % 100000 == 0)
               {
                  std::cout << ".";
                  std::cout.flush();
               }
            }

            std::cout << "Wrote " << counter << " spots\n";
            tsfOut->WriteHeaderBinary(sl);

            delete tsfOut;
            ifs.close();
            fs.close();
         }
      }
   } catch (TSFException ex) 
   {
      std::cout << ex.getMessage().c_str() << std::endl;
      return 1;
   } catch (...)
   {
      std::cout << "Some exceptions occurred.  Exciting now...\n";
   }

   delete sl;
   delete spot;

   delete ls;

   return 0;
}

