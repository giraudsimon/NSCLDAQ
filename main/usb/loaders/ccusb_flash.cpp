/* CC-USB firmware update utility
 * 
 * Copyright (C) 2005-2009 Jan Toke (toke@chem.rochester.edu)
 *
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation, version 2.
 *
 *  This Program is a utility which allows the user of a CC_USB to update the
 *    USB firmware of the CC_USB from Linux  
 * 
*/
#include <stdio.h>
#include <string.h>
#include <usb.h>
#include <time.h>
#include <libxxusb.h>
#include "skipHeader.h"



int main(int argc, char** argv)
{

 
  char* fname;
  char* serial(0);
  struct usb_device *dev;
  xxusb_device_type devices[100]; 
  time_t t1, t2;
  int i;
  long fwid;
  usb_dev_handle *udev;
  unsigned char* pconfig=new unsigned char[220000];
  unsigned char *p;
  int chint;
  long ik;
  FILE *fptr;
  int ret;


    // parameter processing:
    
    if ((argc < 2) || (argc > 3)) {
        printf("Usage\n");
        printf("   %s firmware-file [serial-string]", argv[0]);
        printf("Where:\n");
        printf("    firmware-file - Is a .bit or .bin formatted Xilinx firmware file\n");
        printf("    serial-string - If present is the specific serial number of the CC-USB to flash\n");
        return -1;
    }
    fname = argv[1];
    
    if(argc == 3) {
        serial = argv[2];
    }


	printf("\n*************************************************************************\n\n");
	printf("                   WIENER CC_USB Firmware upgrade\n");
        printf("\n*************************************************************************\n");
  

  
  //open CC_USB
  
    if (serial) {
        udev = xxusb_serial_open(serial);
      
    } else {
      if (xxusb_devices_find(devices) <= 0) {
	printf("There are no CC_CSB controllers on this system\n");
	return 0;
      }
      dev = devices[0].usbdev;
      udev = xxusb_device_open(dev); 
      
    }
    if(!udev)
    {
      printf("\nUnable to open a CC_USB device\n");
      if (serial) {
        printf("Possibly there is no CC_USB with the serial: %s\n", serial);
      }
      return 0;
    }
	  
	  
	//open Firmware File  
  ik=0;
	if ((fptr=fopen(fname,"rb"))==NULL)
	{
    printf("\n File not Found\n");
	  return 0;
	}
	
	
  // Flash VM_USB
	p=pconfig;
	while((chint=getc(fptr)) != EOF)
	{
	 *p=(unsigned char)chint;
	 p++;
	 ik=ik+1;
	}
	printf("\n");
        pconfig = (UCHAR*)skipHeader(pconfig);
	for (i=0; i < 512; i++)
	{
	 ret=xxusb_flashblock_program(udev,pconfig);
	 pconfig=pconfig+256;
	 t1=(time_t)(.03*CLOCKS_PER_SEC);
	 t1=clock()+(time_t)(.03*CLOCKS_PER_SEC);
	 while (t1>clock());
	   t2=clock();
  printf(".");
	if (i>0)
	 if ((i & 63)==63)
	   printf("\n");
  }
	while (clock() < t1)
	    ;
	  t2=clock();
	
  ret = xxusb_reset_toggle(udev);
	
  sleep(1);
  
  ret=CAMAC_register_read(udev,0,&fwid);
	printf("\n\nThe New Firmware ID is %lx\n\n\n",(fwid & 0xffff));
	if (udev)
	xxusb_device_close(udev);
	return 1;
}

  
