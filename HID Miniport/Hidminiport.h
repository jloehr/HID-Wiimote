/*

Copyright (C) 2014 Julian Löhr
All rights reserved.

Filename:
	hidminiport.c

Abstract:
	Header file for hidminiport.c

*/

#ifndef _HIDMINIPORT_H_
#define _HIDMINIPORT_H_

#include <wdm.h>
#include <hidport.h>

#include "Trace.h"

DRIVER_INITIALIZE DriverEntry;
DRIVER_ADD_DEVICE AddDevice;
_Dispatch_type_(IRP_MJ_OTHER)
DRIVER_DISPATCH PassThrough;
//IO_COMPLETION_ROUTINE SendAddressesCompeted;
_Dispatch_type_(IRP_MJ_POWER)
DRIVER_DISPATCH PowerPassThrough;
_Dispatch_type_(IRP_MJ_PNP)
DRIVER_DISPATCH PnPPassThrough;
DRIVER_UNLOAD Unload;


#define GET_NEXT_DEVICE_OBJECT(DO) (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->NextDeviceObject)

#define IOCTL_WIIMOTE_ADDRESSES CTL_CODE(FILE_DEVICE_KEYBOARD, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_POOL_TAG '_WOI' 

typedef NTSTATUS (*PNOTIFY_PRESENCE)(PDEVICE_OBJECT, BOOLEAN);

typedef struct _HID_MINIPORT_ADDRESSES 
{
	PDEVICE_OBJECT FDO;
	PNOTIFY_PRESENCE HidNotifyPresence;

} HID_MINIPORT_ADDRESSES, * PHID_MINIPORT_ADDRESSES;

#endif