/* CC-USB Demo program
 * 
 * Copyright (C) 2005-2009 Jan Toke (toke@chem.rochester.edu)
 *
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation, version 2.
 *
 *  This Program is a utility which allows the user of a VM_USB to update the
 *    USB firmware of the VM_USB from Linux  
 * 
*/
#include <stdio.h>
#include <string.h>
#include <usb.h>
#include <time.h>
#include <libxxusb.h>
#include <skipHeader.h>



int main(int argc, char** argv)
{
  char* fname;
    char* serial(0);
    
    if ((argc < 2) || (argc > 3)) {
        printf("\nUsage:\n");
        printf("    vmusloader  firmware-file [serial-string]\n");
        printf("Where:\n");
        printf("     firmware-file - is a Xilinx .bit or .bin formatted firmware file\n");
        printf("     serial-string - if present is the serial number of the VM-USB to flash\n");
        return -1;
            
    }
    
    // By now we know the filename is present
    
    fname = argv[1];
    
    if (argc == 3) {
        serial = argv[2];
    }
    
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



	printf("\n*************************************************************************\n\n");
	printf("                   WIENER VM_USB Firmware upgrade\n");  
        printf("\n*************************************************************************\n");


  //open VM_USB
  
  if (serial) {
    udev = xxusb_serial_open(serial);
  } else {
    if(xxusb_devices_find(devices) <= 0) {
      printf("There are no VM-USB devices on this system\n");
      return 0;
    }
    dev = devices[0].usbdev;
    udev = xxusb_device_open(dev);
  }
    if(!udev)
    {
            printf("\nUnable to open a VM_USB device\n");
            if (serial) {
	      printf("VM-USB with serial %s probably is not attached.\n", serial);
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
        pconfig = (UCHAR*)skipHeader(pconfig);
	printf("\n");
	for (i=0; i < 830; i++)
	{
	 ret=xxusb_flashblock_program(udev, pconfig);
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
  
  ret=VME_register_read(udev,0,&fwid);
	printf("\n\nThe New Firmware ID is %lx\n\n\n",fwid);
	if (udev)
	xxusb_device_close(udev);
	return 1;
}

  
