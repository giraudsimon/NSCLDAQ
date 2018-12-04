/////////////////////////////////////////////////
//                 readscaler
// Authors: K. Brown
//          G. Cerizza
// 
// Program to read scaler channels and dump
// the rates into a txt file
//
/////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <unistd.h>

bool debug=false;

int main(int argc, char*argv[])
{
  int runbegin;
  int runend;
  int numchannels;
  std::string stagearea;
  
  unsigned short *point;

  int physicsEvent = 0;
  int physicsEventCounter = 0;
  int scalerBuffer = 0;
  int Npauses = 0;
  int Nresumes = 0;
  int runNo = 0;

  std::ostringstream outstring;
  std::ifstream evtfile;

  // Calling readscaler from terminal 
  if (argc != 5)
    {
      std::cout << "> readscalers needs more arguments!" << std::endl;
      std::cout << "> readscalers path_to_run_dirs run_begin run_end number_of_channels" << std::endl;
      exit(0);
    }
  else
    {
      stagearea = (std::string)argv[1];
      runbegin = atoi(argv[2]);
      runend = atoi(argv[3]);
      if (runbegin > runend) {
	int tmp;
	tmp = runend;
	runend = runbegin;
	runbegin = tmp;
      }
      numchannels = atoi(argv[4]);
      if (debug){
      std::cout << "Starting run: " << runbegin << std::endl;
      std::cout << "Ending run: " << runend << std::endl;
      std::cout << "Number of channels: " << numchannels << std::endl;
      }
    }

  // Loop over run numbers
  for (int run = runbegin; run < (runend+1); run++) 
    {
      int scalernumbers[numchannels] = {0};
      scalerBuffer = 0;
      // Output file
      std::string ofile = "scaler_run";
      ofile += std::to_string(run);
      ofile += ".csv";
      std::ofstream outscalers(ofile);

      // Accessing the files
      std::ostringstream strdir;

      int exchar = 0;
      for (exchar = 0; exchar < 5; exchar++)
      	{
      	  strdir.str("");
	  strdir << stagearea << "/run" << run << "/run-" << std::setfill('0') << std::setw(4) << run;
      	  if (exchar == 0){
      	    strdir << "-00.evt";	    
      	  }
      	  else {
      	    strdir << "-" << std::setfill('0') << std::setw(2) << exchar << ".evt";
      	  }

	  std::string filename = strdir.str();
	  if (debug)
	    std::cout << filename << std::endl;
	  
	  // Open evt file
	  evtfile.clear();
	  evtfile.open(filename.c_str(),std::ios::binary);
	  
	  if (evtfile.bad() || evtfile.fail() || !evtfile)
	    { 
	      break;
	    }
	  
	  outscalers << "SourceID,Time";
	  for (int i = 0; i < numchannels; i++)
	    outscalers << ",Channel" << i;
	  outscalers << std::endl;
	  
	  // Loop over evt files
	  for (;;)
	    {
	      int const hBufferWords = 4;
	      int const hBufferBytes = hBufferWords*2;
	      unsigned short hBuffer[hBufferWords];
	      evtfile.read((char*)hBuffer,hBufferBytes);
	      point = hBuffer;

	      if(evtfile.eof() || evtfile.bad())
		{
		  std::cout << "----> The run below has a problem: file didn't close correctly!!" << std::endl;
		  break;
		}
	      
	      int nbytes = *point++;
	      int nbytes2 = *point++;
	      int type = *point++;
	      int type2 = *point;
	      
	      int dBufferBytes = nbytes - 8; // Skipping the inclusive size and data type
	      int dBufferWords = dBufferBytes/2; // Calculating 16 bit words from bytes
	      
	      unsigned short dBuffer[dBufferWords];
	      evtfile.read((char*)dBuffer,dBufferBytes);
	      point = dBuffer;
	      
	      // This is to read the Body Header
	      int BHsize = *point++;
	      int BHsize2 = *point++;
	      int64_t eventTstamp = *point;
	      int sourceID  =-1;
	      int BarrierType;
	      
	      if(BHsize==20)
		{
		  point +=4; // Skip timestamp
		  sourceID = *point++;
		  point++;
		  BarrierType = *point++;
		  point++;
		}
	      else
		{
		  // Buffers with no body header
		}
	      
	      // Selecting the scaler type
	      if (type == 1)
		{
		  runNo = *point;
		}
	      else if (type == 30)
		{
		  physicsEvent++;
		}
	      else if (type == 31)
		{
		  physicsEventCounter++;
		}
	      else if (type == 2)
		{
		  break;
		}
	      else if (type == 20)
		{
		  if (debug){
		    std::cout << "Found a scalar buffer" << std::endl;
		    std::cout << "Run number = " << std::dec << runNo << std::endl;
		    std::cout << sourceID << std::endl;
		  }
		  
		  scalerBuffer++;
		  if (scalerBuffer%30000 == 0)
		    std::cout << '\xd'<< scalerBuffer << std::flush;
		  
		  unsigned short * scaler = point;
		  unsigned short size1;
		  unsigned short size2;
		  unsigned long size;
		  scaler +=4; // Skipping to the date
		  
		  unsigned short Time1 = *scaler++;
		  unsigned short Time2 = *scaler++;
		  
		  time_t rTime = (Time2 << 16) | Time1;
		  
		  scaler +=6;
		  
		  std::string strTime = ctime(&rTime);
		  strTime.erase(strTime.length() - 1, 1);
		  outscalers << sourceID << "," << strTime;
		  if (debug)
		    std::cout << sourceID << " " << strTime << std::endl;
		  
		  for(int i = 0; i < numchannels; i++)
		    {
		      size1 = *scaler++;
		      size2 = *scaler++;
		      size = (size2 << 16) | size1;
		      scalernumbers[i] += size;
		      if (debug)
			std::cout << i << " " << scalernumbers[i] << std::endl;
		      outscalers << "," << size;
		    }
		  outscalers << std::endl;

		}
	      else if (type == 3) Npauses++;
	      else if (type == 4) Nresumes++;
	      
	    }	  
	  
	  std::cout << "#########################################" << std::endl;
	  if (exchar == 0)
	    std::cout << "# Summary for Run = " << run << std::endl;
	  else
	    std::cout << "# Cumulative summary for Run = " << run << "-0" << exchar << std::endl;	    
	  std::cout << "#########################################" << std::endl;
	  std::cout << "Number of scaler buffers read = " << scalerBuffer << std::endl;
	  
	  for (int i = 0; i < numchannels; i++){
	    std::cout << "Channel " << i << " " << scalernumbers[i] << "  Ave Rate = " << (float)scalernumbers[i]/(float)scalerBuffer << std::endl;
	  }
	  
	  evtfile.close();
	  evtfile.clear(); 
	}
    }
}
