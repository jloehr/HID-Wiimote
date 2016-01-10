# HID Wiimote - A Windows Device Driver for the Nintendo Wii Remote

## Builds

You can get test signed and zipped Driver Packages from the project site:
http://julianloehr.de/educational-work/hid-wiimote/

## Build yourself

To build the driver yourself you need the following software.
All paths in the project are either relative or are using Visual Studio Macros.

The One-Click Build is open `Build` > `Batch Build...`, check only all `BuildAndZip` Configurations and hit `Build` or `Rebuild`. This will build all drivers, the installer, pack everything into a Driver Package and Zip it. The final Zips are then found in the `Zip` folder.

### Requirements

* Visual Studio 2015
* Windows Driver Kit (WDK 10)

Both can be downloaded on this page: https://msdn.microsoft.com/en-us/windows/hardware/dn913721.aspx

### Paths to adjust

The Dirver Package is going to copy the Driver Package Installer (DPInst) into the Build directoy. (Only on Release Configurations)

Normally the `WindowsSdkDir` Macro in Visual Studio should point to the Windows SDK 10 installation directory (which in turn contains the WDK). If not either fix it or change the path in the `Driver Package Properties` > `Driver Install` > `Package Files`.

## Projects Overview

### Build And Zip

Helper project to zip the Driver Packages after building. Has only Release Configurations, so should be only used on Release builds. Has a dependency on `HID Wiimote Package` so the package itself is build/rebuild as well.

### HID Miniport

My own implementation of the MsHidKmdf driver, so i can pass some function adresses to the HID Wiimote driver. It basicly just passes everything down to the HID Wiimote filter driver, additionaly a custom IOCTL to request function adresses.

### HID Wiimote

Main driver - does all the important Wiimote stuff.

### HID Wiimote Package

Driver Package/Utility project, to pack everything into one package and sign it. No code, just settings.

### TinyInstaller

My main installer written in C#. Just a helper to install my certificate to boost the driver ranking (so it will be picked instead of the default driver). Then invokes the Driver Package Installer (DPInst.exe).

## Contribution

Feel free to create issues, reporting bugs or giving feedback/features wishes.


