HID Wiimote 0.3.0.1

HID Wiimote is a Windows Device Driver for the Nintendo Wii Remote.

Project Homepage: https://julianloehr.de/educational-work/hid-wiimote/
Supported Windows Versions: 7, 8, 8.1, 10

This software is provided free of charge. If you have paid for this, please claim a refund and report this site to: info@julianloehr.de

If you like this project and really want to pay for it, see my homepage (https://julianloehr.de) for donations.

-- Changelog --

-- 0.3.0.1 --

- Fix #24 & Fix #31: Fix Guitar Hero Buttons, Whammy and Touch bar
- Fix not detecting certain GH Accessories
- Change default settings for new unknown devices to have accelerometer and trigger axis enabled

-- 0.3.0.0 --

- Implement #2: Basic Tool for device specific settings, including an installer replacing TinyInstaller
- Fix phony button presses when connecting extension

-- 0.2.7.4 --

- Fix Balance Board not beeing detected correctly and reporting any input

-- 0.2.7.3 --

- Fix #15: Yellow Guitar Hero Button not working

-- 0.2.7.2 --

- Change Balance Board Axes

-- 0.2.7.1 --

- Fix Balance Board Axes

-- 0.2.7.0 --

- Fix RawInput not reading the primary axes correctly
- Add experimental untested Balance Board support
- Add experimental untested Guitar Hero Guitar support

-- 0.2.6.2 --

- Fix non English languages for the Driver Package Installer

-- 0.2.6.1 --

- Revert back to Multilingual Driver Package Installer

-- 0.2.6 --

- Fix #11: Improved Extension Controller detection
- Package now includes EULA and Readme file
- Replaced Multilingual Driver Package Installer with only-English one
- Introduced proper versioning, starting with 0.2.6
- Build with WDK 10.0.10586.15, to hopefully fix connectivity issues with Windows 10 Version 1511 (November Update)
