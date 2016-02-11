using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace HID_Wiimote_Control_Center.Setup
{
    class DriverPackage : IInstallerTask
    {
        const string InstallDir = @"%ProgramFiles%\DIFX\HID Wiimote";
        const string UninstallFileName = "Uninstall.bat";
        const string DPInstFileName = "dpinst.exe";
        const string InfFileName = "HIDWiimote.inf";
        const string UninstallerInfFileToken = "{{InfFileName}}";
        const string InstallDirToken = "{{InstallDir}}";

        public bool IsGood()
        {
            return IsInstalled();
        }

        public void TryMakeBad()
        {
            Uninstall();
        }

        public void TryMakeGood()
        {
            Install();
        }

        public static void Uninstall()
        {
            string UninstallerString = DriverPackageUninstallerRegistry.GetUninstallString();

            Process Uninstall = Process.Start("cmd.exe", "/C \"" + UninstallerString + "\"");
            Uninstall.WaitForExit();
        }

        public static void Install()
        {
            if (IsInstalled())
            {
                return;
            }

            string ActuralInstallDir = Environment.ExpandEnvironmentVariables(InstallDir);
            string DPInstPath = System.IO.Path.Combine(ActuralInstallDir, DPInstFileName);
            string InfPath = System.IO.Path.Combine(ActuralInstallDir, InfFileName);
            string UninstallerPath = System.IO.Path.Combine(ActuralInstallDir, UninstallFileName);

            // Create Dir
            System.IO.Directory.CreateDirectory(ActuralInstallDir);

            // Copy DPinst, Inf & Create Uninstaller
            System.IO.File.Copy(DPInstFileName, DPInstPath);
            System.IO.File.Copy(InfFileName, InfPath);
            using (System.IO.StreamWriter UninstallerStreamWriter = new System.IO.StreamWriter(UninstallerPath))
            {
                string UninstallerContent = HID_Wiimote_Control_Center.Properties.Installer.UninstallerContent;
                UninstallerContent = UninstallerContent.Replace(UninstallerInfFileToken, InfFileName);
                UninstallerContent = UninstallerContent.Replace(InstallDirToken, ActuralInstallDir);
                UninstallerStreamWriter.Write(UninstallerContent);
            }

            // Run DPinst
            Process DPInst = Process.Start(DPInstFileName);
            DPInst.WaitForExit();
            System.Console.WriteLine(DPInst.ExitCode);   

            // Set up uninstall reg key
            DriverPackageUninstallerRegistry.CreateHIDWiimoteUninstallKey(UninstallerPath, DPInstPath);
        }

        private static bool IsInstalled()
        {
            DriverPackageUninstallerRegistry.DriverPackageState DPState = DriverPackageUninstallerRegistry.GetDriverPackageState(VersionStrings.DriverPackageVersion);

            return (DPState != DriverPackageUninstallerRegistry.DriverPackageState.NoneInstalled);
        }
    }
}
