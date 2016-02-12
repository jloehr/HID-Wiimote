using HID_Wiimote_Control_Center.Setup.InstallStepAction;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HID_Wiimote_Control_Center.Setup
{
    static class DriverPackage
    {
        const string InstallDir = @"%ProgramFiles%\DIFX\HID Wiimote";
        const string UninstallFileName = "Uninstall.bat";
        const string DPInstFileName = "dpinst.exe";
        const string InfFileName = "HIDWiimote.inf";
        const string UninstallerInfFileToken = "{{InfFileName}}";
        const string InstallDirToken = "{{InstallDir}}";

        public static bool IsInstalled()
        {
            DriverPackageUninstallerRegistry.DriverPackageState DPState = DriverPackageUninstallerRegistry.GetDriverPackageState(VersionStrings.DriverPackageVersion);

            return (DPState != DriverPackageUninstallerRegistry.DriverPackageState.NoneInstalled);
        }

        public static void Install()
        {
            if (IsInstalled())
            {
                return;
            }

            string ActualInstallDir = Environment.ExpandEnvironmentVariables(InstallDir);
            string DPInstInstallPath = System.IO.Path.Combine(ActualInstallDir, DPInstFileName);
            string InfInstallPath = System.IO.Path.Combine(ActualInstallDir, InfFileName);
            string UninstallerPath = System.IO.Path.Combine(ActualInstallDir, UninstallFileName);

            string UninstallerContent = HID_Wiimote_Control_Center.Properties.Installer.UninstallerContent;
            UninstallerContent = UninstallerContent.Replace(UninstallerInfFileToken, InfFileName);
            UninstallerContent = UninstallerContent.Replace(InstallDirToken, ActualInstallDir);

            // Make sure Directory exists
            System.IO.Directory.CreateDirectory(ActualInstallDir);

            List<IInstallStepAction> InstallSteps = new List<IInstallStepAction> {
                new CopyFile(DPInstFileName, ActualInstallDir),
                new CopyFile(InfFileName, ActualInstallDir),
                new TextResourceToFile(UninstallerPath, UninstallerContent),
                new RunDPInstall(DPInstFileName, InfFileName),
                new CreateRegistryUninstallKey(UninstallerPath, DPInstInstallPath)
            };

            for (int i = 0; i < InstallSteps.Count; i++)
            {
                if (!InstallSteps[i].Do())
                {
                    for (int j = i; j >= 0; j--)
                    {
                        InstallSteps[j].Undo();
                    }

                    throw new Exception(InstallSteps[i].GetExceptionMessage());
                }
            }
        }

        public static void Uninstall()
        {
            string UninstallerString = DriverPackageUninstallerRegistry.GetUninstallString();
            Process Uninstall = Process.Start("cmd.exe", "/C \"" + UninstallerString + "\"");
            Uninstall.WaitForExit();
        }
    }
}
