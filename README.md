##  Description of  tagged spot file format (.tsf) 

Nico Stuurman, April 3, 2013, modified Jan. 2017

The goal of the tagged post file format is to provide an efficient data format for superresolution microscopy data that generate images by locating the position of single fluorescent emitters.  Two formats exists, a binary format (using Google protocol buffers) and a text format.  An application is free to  upport only one or the other, but is encouraged to support both.

### Binary format.   

The binary format relies on [Google Protocol buffers](http://code.google.com/apis/protocolbuffers/) to encode data. Two message types are defined, “SpotList”, which contains metadata concerning the dataset, and “Spot”, which contains the per-spot data.  For convenience, a copy of the proto file describing these messages is included at the bottom of the document, however, the authorative version resides in this github repository.

Note that you can extend the TSF.proto file yourself and add fields that you need for your specific application.  The file created with your proto file can still be read by all other software that reads the tagged spot file format (although it will not be able to interpret the fields that you added).  

File offsets are of type signed int or signed long (to ensure compatibility with Java code) and are encoded as big endian.

### File layout on disk: 

1. `int magic` (4 bytes) - must be 0 (this is for compatibility with older versions of the tsf format)

2. `long SpotList offset` (8 bytes) - offset in bytes to SpotList message (note, this is the offset from the beginning of the file minus 12 bytes).

3. As many copies of the message “Spot” as desired, where

	* `varint length` - Size of the Spot message in a Google Protocol buffer defined varint
	
	* `Spot message`

4. The Spot messages are followed by the SpotList message:

	* `varint length` - Size of SpotList message in a Google Protocol buffer defined varint
	
	* `SpotList message`

The SpotList message is at the end so that it ispossible to write all the Spot data first and then list the number of Spots that have been written in the SpotList message.  Code that reads the file can first read the SpotList (using the SpotList offset) and will then know how many Spot messages to expect.  Note that it is not necessary to know this number in advance, one can simply parse Spot messages until no more new ones are found.

###  Building and/or using the code


To use the tsf format in your own code, you need Java or C++ source code to work with the tsf format.  This source code is created by the Google Protocol Buffers binary based on the TSF.proto file.  You can create this source code yourself by downloading and installing Google Protocol Buffers and compiling the the TSF.proto file.  To do so, I use the following script (Mac OS X shell script):


~~~~
protoc -I=src/ --java_out=build src/TSFProto.proto
javac -sourcepath build -classpath dist/gproto.jar build/edu/ucsf/tsf/TaggedSpotsProtos.java
jar cf dist/TSFProto.jar -C build .
protoc -I=src/ --cpp_out=buildcpp src/TSFProto.proto
g++ -c buildcpp/TSFProto.pb.cc -o buildcpp/libTSFProto.o
ar cru buildcpp/libTSFPProto.a buildcpp/TSFProto.o /usr/local/lib/libprotobuf.a
~~~~

Alternatively, you can download and install the source code that I build.  It resides in this github repository.  The Java source code is in directory “build”, the C++ source code in directory “buildcpp”.

If you code in C++, you can save a lot of time by making use of the class TSFUtils, which is part of the source code of the example application in the tsfutil directory.  The example command line application tsftrans can read and write tsf files in both binary and text formats and is an easy tool to convert between the two.

The following (pseudo Java) code shows how you can read in a file in TSF format:

~~~~
        FileInputStream fi = new FileInputStream(selectedFile);
        DataInputStream di = new DataInputStream(fi);

        // the file has an initial 0, then the offset (as long)
        // to the position of spotList
        int magic = di.readInt();
        long offset = di.readLong();
        fi.skip(offset);
        edu.ucsf.tsf.TaggedSpotsProtos.SpotList psl = SpotList.parseDelimitedFrom(fi);
        fi.close();


        // psl contains the header information
        // the following code shows examples how to use it   

        String name = psl.getName();
        int width = psl.getNrPixelsX();
        int height = psl.getNrPixelsY();
        float pixelSizeUm = psl.getPixelSize();
        long totalSpots = psl.getNrSpots();
        long maxNrSpots = 0;

        // now read the messages that contain the spot data
        // Re-open the stream

        fi = new FileInputStream(selectedFile);
        fi.skip(12); // size of int + size of long

        // simple example of reading Spot messages.
        // One could also store the Spots as an Array or List
        // and access the Spot data directly in the code (i.e., use
        // the Spot message as a native data structure

        edu.ucsf.tsf.TaggedSpotsProtos.SpotSpot pSpot;

        while (fi.available() > 0 && (expectedSpots == 0 || totalSpots < expectedSpots)) {

             pSpot = Spot.parseDelimitedFrom(fi);
             double x = pSpot.getXPosition();
             double y = pSpot.getYposition();
             double in = pSpot.getIntensity();
             if (pSpot.hasZ()) {
                 double zc = getZ();            
             }

             totalSpots++;
        }
        fi.close();
~~~~

### Reading and writing files from Matlab 

To be written...

### Text format


The text version of the Tagged Spot format is modeled closely after the binary format.  The first line of the file (terminated with a newline “\n” symbol) contains the Summary metadata (the SpotList message in the binary format).  It contains Key-Value pairs (separated by “: “) where the keys are defined in the TSF.proto file SpotList description, and the values are given in asci representation.  The Key-Value pairs are separated by the tab (“\t”) sign.

The second line of the file contains the Keys of the Spot message for the Spot data that follow.  The allowed names for the Keys are defined in the TSF.proto file Spot message definition.  Keys are separated by tabs (“\t”), and the line ends with a newline (“\n”) symbol.

All following lines are Spot data in asci format separated by tabs (“\t”) and each Spot is on its own line (that ends with “\n”).

### TSF.proto file: 

~~~~
package TSF;

option java_package = "edu.ucsf.tsf";
option java_outer_classname = "TaggedSpotsProtos";

enum FitMode {
  ONEAXIS = 0;
  TWOAXIS = 1;
  TWOAXISANDTHETA = 2;
}

enum IntensityUnits {
  COUNTS = 0;
  PHOTONS = 1;
}
enum LocationUnits {
  NM = 0;
  UM = 1;
  PIXELS = 2;
}

// If units will always be the same for all spots, then use these units tags,
message SpotList {
  // UID for the application that generated these data
  // Request a UID from nico at cmp.ucsf.edu or use 1
  required int32 application_id = 1 [default = 1];

  optional string name = 2; // name identifying the original dataset
  optional string filepath = 3; // path to the image data used to generate these spot data
  optional int64 uid = 4; // Unique ID, can be used by application to link to original data
  optional int32 nr_pixels_x = 5; // nr pixels in x of original data
  optional int32 nr_pixels_y = 6; // nr pixels in y of original data
  optional float pixel_size = 7; // pixel size in nanometer
  optional int64 nr_spots = 8; // number of spots in this data set
  optional int32 box_size = 17; // size (in pixels) of rectangular box used in Gaussian fitting
  optional int32 nr_channels = 18; // Nr of channels in the original data set
  optional int32 nr_frames = 19; // Nr of frames in the original data set
  optional int32 nr_slices = 20; // Nr of slices in the original data set
  optional int32 nr_pos = 21; // Nr of positions in the original data set


  // otherwise use the unit tags with each spot
  optional LocationUnits location_units = 22;
  optional IntensityUnits intensity_units = 23; // use only if different from SpotList

  // If fitmode  will always be the same for all spots, then use this fitmode
  // otherwise use the fitmode with each spot
  optional FitMode fit_mode = 24;
  optional bool is_track = 25 [default = false]; // flag indicating whether
  // this is a sequence of spot data in consecutive time frames thought to 
  // originate from the same entity

  //repeated Spot spot = 8;
 
     
}

message Spot {
  required int32 molecule = 1; // ID for this spot
  required int32 channel = 2; // channels are 1-based
  required int32 frame = 3; // frames are 1-based
  optional int32 slice = 4; // slices are 1-based
  optional int32 pos = 5; // positions are 1-based

  // xyz coordinates of the spot in location_units  after fitting and optional correction
  optional LocationUnits location_units = 17;
  required float x = 7;
  required float y = 8;
  optional float z = 9;

  // Intensity values
  optional IntensityUnits intensity_units = 18; // use only if different from SpotList
  required float intensity = 10; // integrated spot density
  optional float background = 11; // background determined in fit
  optional float width = 12; // peak width at half height in location units
     // for asymetric peaks, calculate the width as the square root of the
     // product of the widths of the long and short axes

  optional float a = 13; // shape of the peak: width of the long axis
     // divided by width of the short axis
  
  optional float theta = 14; // rotation of assymetric peak, only used
     // when fitmode == TWOAXISANDTHETA


  optional int32 flag = 6; // flag to categorize spots. Implementation specific

  // Original xyz coordinates from fitting before drift or other correction correction
  optional float x_original = 101;
  optional float y_original = 102;
  optional float z_original = 103;

  // localization precision
  optional float x_precision = 104;
  optional float y_precision = 105;
  optional float z_precision = 106;

  // position in the original image (in pixels) used for fitting
  optional int32 x_position = 107;
  optional int32 y_position = 108;

  // These ids can be used in your own .proto derived from this one
  extensions 1500 to 2047;

}


~~~~