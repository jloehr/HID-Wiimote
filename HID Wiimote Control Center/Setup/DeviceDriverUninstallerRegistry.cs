/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	DeviceDriverUninstallerRegistry.cs

Abstract:
	Let's one create and retrieve the uninstaller registry entry and quiery its state 

*/
using Microsoft.Win32;

namespace HIDWiimote.ControlCenter
{
    static class DeviceDriverUninstallerRegistry
    {
        private static string UninstallKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\HID Wiimote\\";

        private struct Version
        {
            public ushort Major;
            public ushort Minor;
            public ushort Build;
            public ushort Revision;

            public Version(string VersionString)
            {
                string[] StringParts = VersionString.Split('.');

                Major = ParseStringPart(StringParts, 0);
                Minor = ParseStringPart(StringParts, 1);
                Build = ParseStringPart(StringParts, 2);
                Revision = ParseStringPart(StringParts, 3);
            }

            public bool IsNewer(Version Other)
            {
                if (this.Major != Other.Major)
                {
                    return this.Major > Other.Major;
                }

                if (this.Minor != Other.Minor)
                {
                    return this.Minor > Other.Minor;
                }

                if (this.Build != Other.Build)
                {
                    return this.Build > Other.Build;
                }

                if (this.Revision != Other.Revision)
                {
                    return this.Revision > Other.Revision;
                }

                return false;
            }

            private static ushort ParseStringPart(string[] StringParts, int index)
            {
                if (StringParts.Length <= index)
                {
                    return 0;
                }

                ushort.TryParse(StringParts[index], out ushort Result);

                return Result;
            }
        }

        public enum DeviceDriverState { NoneInstalled, OlderInstalled, CurrentInstalled, NewerInstalled };

        public static DeviceDriverState GetDeviceDriverState(string AppVersionString)
        {
            return GetDeviceDriverState(AppVersionString, out string InstalledVersionString);
        }

        public static DeviceDriverState GetDeviceDriverState(string AppVersionString, out string InstalledVersionString)
        {
            InstalledVersionString = "";
#if !DEBUG
            RegistryKey UninstallerKey = GetHIDWiimoteUninstallKey();

            if (UninstallerKey == null)
            {
                return DeviceDriverState.NoneInstalled;
            }

            InstalledVersionString = UninstallerKey.GetValue("DisplayVersion") as string;

            if (AppVersionString == InstalledVersionString)
            {
                return DeviceDriverState.CurrentInstalled;
            }

            Version AppVersion = new Version(AppVersionString);
            Version InstalledVersion = new Version(InstalledVersionString);

            if (AppVersion.IsNewer(InstalledVersion))
            {
                return DeviceDriverState.OlderInstalled;
            }

            return DeviceDriverState.NewerInstalled;
#else
            return DeviceDriverState.CurrentInstalled;
#endif
        }

        public static string GetInstalledVersionString()
        {
            RegistryKey UninstallerKey = GetHIDWiimoteUninstallKey();
            return UninstallerKey.GetValue("DisplayVersion") as string;
        }

        public static string GetUninstallString()
        {
            RegistryKey UninstallerKey = GetHIDWiimoteUninstallKey();

            if (UninstallerKey == null)
            {
                return string.Empty;
            }

            return UninstallerKey.GetValue("UninstallString") as string;
        }

        public static bool CreateHIDWiimoteUninstallKey(string UninstallCommnad, string DPInstPath)
        {
            RegistryKey UninstallerKey = Registry.LocalMachine.CreateSubKey(UninstallKey);
            if (UninstallerKey == null)
            {
                return false;
            }

            UninstallerKey.SetValue("DisplayName", "HID Wiimote " + VersionStrings.DeviceDriverVersion, RegistryValueKind.String);
            UninstallerKey.SetValue("DisplayIcon", DPInstPath + ",0", RegistryValueKind.String);
            UninstallerKey.SetValue("DisplayVersion", VersionStrings.DeviceDriverVersion, RegistryValueKind.String);
            UninstallerKey.SetValue("Publisher", "Julian Löhr", RegistryValueKind.String);
            UninstallerKey.SetValue("UninstallString", string.Format("\"{0}\"", UninstallCommnad), RegistryValueKind.String);
            UninstallerKey.SetValue("HelpLink", "https://www.julianloehr.de/educational-work/hid-wiimote/", RegistryValueKind.String);

            return true;
        }

        private static RegistryKey GetHIDWiimoteUninstallKey()
        {
            return Registry.LocalMachine.OpenSubKey(UninstallKey, false);
        }
    }
}
