# HID Wiimote - A Windows Device Driver for the Nintendo Wii Remote

⚠ This project and repository has been archived, as it hasn't been under any active development for a very long time ⚠

## Builds

You can get test signed and zipped Driver Packages from the project site:
http://julianloehr.de/educational-work/hid-wiimote/

## Build yourself

To build the driver yourself you need the following software.
All paths in the project are either relative or are using Visual Studio Macros.

The One-Click Build is open `Build` > `Batch Build...`, check only all `BuildAndZip` Configurations and hit `Build` or `Rebuild`. This will build all drivers, User Mode app and library, pack everything into a Driver Package and Zip it. The final Zips are then found in the `Zip` folder.

### Requirements

* Visual Studio 2022
* Windows SDK for Windows 11
* Windows Driver Kit (WDK 11)

All can be downloaded on this page: https://developer.microsoft.com/en-us/windows/hardware/windows-driver-kit

## Projects Overview

### Build And Zip

Helper project to zip the Driver Packages after building. Has only Release Configurations, so should be only used on Release builds. Has a dependency on `HID Wiimote Package` so the package itself is build/rebuild as well.

### Release Versioning

Utility project that generates different files containing the current version string. Version is parsed from package readme.

### HID Miniport

My own implementation of the MsHidKmdf driver, so I can pass some function addresses to the HID Wiimote driver. It basically just passes everything down to the HID Wiimote filter driver, additionally a custom IOCTL to request function addresses.

### HID Wiimote

Main driver - does all the important Wiimote stuff.

### HID Wiimote Control Center

User Mode Application to set specific device settings. Also includes an installer and updater.

### HID Wiimote User Mode

User Mode Managed Library to interact with Wiimote Devices.

### HID Wiimote Package

Driver Package/Utility project, to pack everything into one package and sign it. No code, just settings.

## Contribution

Feel free to create issues, report bugs or give feedback/features wishes.
