using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HID_Wiimote_Control_Center
{
    static class DriverPackageUninstallerRegistry
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
                if(this.Major != Other.Major)
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
                if(StringParts.Length <= index)
                {
                    return 0;
                }

                ushort Result = 0;
                ushort.TryParse(StringParts[index], out Result);

                return Result;
            }
        }

        public enum DriverPackageState { NoneInstalled, OlderInstalled, CurrentInstalled, NewerInstalled };


        public static DriverPackageState GetDriverPackageState(string AppVersionString, out string InstalledVersionString)
        {
            InstalledVersionString = "";
#if !DEBUG
            RegistryKey UninstallerKey = GetHIDWiimoteUninstallKey(false);

            if(UninstallerKey == null)
            {
                return DriverPackageState.NoneInstalled;
            }

            InstalledVersionString = UninstallerKey.GetValue("DisplayVersion") as string;

            if(AppVersionString == InstalledVersionString)
            {
                return DriverPackageState.CurrentInstalled;
            }

            Version AppVersion = new Version(AppVersionString);
            Version InstalledVersion = new Version(InstalledVersionString);

            if(AppVersion.IsNewer(InstalledVersion))
            {
                return DriverPackageState.OlderInstalled;
            }

            return DriverPackageState.OlderInstalled;
#else
            return DriverPackageState.CurrentInstalled;
#endif
        }

        public static void SetRegistryKey()
        {

        }

        private static RegistryKey GetHIDWiimoteUninstallKey(bool Writeable)
        {
            return Registry.LocalMachine.OpenSubKey(UninstallKey, Writeable);
        }
    }
}
