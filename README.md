# HID Wiimote - A Windows Device Driver for the Nintendo Wii Remote

## Builds

You can get test signed and zipped Driver Packages from the project site:
http://julianloehr.de/educational-work/hid-wiimote/

## Build yourself

To build the driver yourself you need the following software and make some additional configurations.

### Requirements

Visual Studio 2013
Windows Driver Kit (WDK 8.1)

Both can be downloaded on this page: https://msdn.microsoft.com/en-us/windows/hardware/gg454513#drivers

### Driver Package Configuration

The Dirver Package is going to copy the Driver Package Installer (DPInst) into the Build directoy.
When the WDK 8.1 was not installed into its default directory, change the path in the HID Wiimote Package Project.

My Post-Build Settings are going to zip up the Driver Package in Release Mode. Either adjust the path to the Zip Tools in the Post-Build Events or remove them entirely. Those are just for me that i have nice zips to upload.

At last you need to create a test signing certificate yourself, which is going to be needed to sign the Driver Package. In the HID Wiimote Package Properties open Driver Signing and set the Test Certificate. In the selection dropdown should be an entry to create a Test Certificate.

## Contribution

Feel free to create issues, reporting bugs or giving feedback/features wishes, as well as submit Pull Requests.


