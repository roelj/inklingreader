#include "online-mode.h"
#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
usb_online_mode_init ()
{
  if (!libusb_init (NULL))
    puts ("libusb_init() succeeded.");

  /* Traverse the list instead, because now only one Inkling device
   * can be detected. */
  //handle = libusb_open_device_with_vid_pid (NULL, 0x056a, 0x0221);

  // discover devices
  libusb_device **list;
  libusb_device *found = NULL;
  ssize_t cnt = libusb_get_device_list(NULL, &list);
  ssize_t i = 0;
  int err = 0;
  if (cnt < 0)
      puts ("Error!");
  for (i = 0; i < cnt; i++)
    {
      libusb_device *device = list[i];
      struct libusb_device_descriptor descriptor;
      libusb_get_device_descriptor (device, &descriptor);
      if (descriptor.idVendor == 0x056a && descriptor.idProduct == 0x0221)
	{
	  found = device;
	  break;
	}
    }

  if (found)
    {
      libusb_device_handle* handle;
      err = libusb_open (found, &handle);
      if (err)
	puts ("Error 1!");

      if (handle != NULL)
	{
	  // SET_IDLE Request
	  // This should trigger "online-mode".
	  int bytes = libusb_control_transfer (handle, 0x21, 10, 0, 0, NULL, 0, 0);
	  printf ("bytes sent = %d\n", bytes);

	  unsigned char usb_data[34];
	  memset (&usb_data, '\0', 34);

	  // SET_REPORT Request
	  // I have no idea what these values mean..
	  usb_data[0] = 0x80;
	  usb_data[1] = 0x01;
	  usb_data[2] = 0x0a;
	  usb_data[3] = 0x01;
	  usb_data[4] = 0x01;
	  usb_data[5] = 0x0e;
	  usb_data[6] = 0x01;

	  bytes = libusb_control_transfer (handle, 0x21, 9, 0x0380, 0, usb_data, 33, 0);
	  printf ("bytes sent = %d\n", bytes);	
	}
      else
	{
	  puts ("handle is NULL.");
	}
    }
  
  libusb_free_device_list (list, 1);

}


void
usb_online_mode_exit ()
{
  libusb_exit (NULL);
}
