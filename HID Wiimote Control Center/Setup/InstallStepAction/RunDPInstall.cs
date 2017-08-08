/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	RunDPInstall.cs

Abstract:
	Install Task to run dpinst.exe

*/
using System.Diagnostics;

namespace HIDWiimote.ControlCenter.Setup.InstallStepAction
{
    class RunDPInstall : IInstallStepAction
    {
        string DPInstPath;
        string InfFilePath;

        public RunDPInstall(string DPInstPath, string InfFilePath)
        {
            this.DPInstPath = DPInstPath;
            this.InfFilePath = InfFilePath;
        }

        public bool Do()
        {
            Process DPInst = Process.Start(DPInstPath);
            DPInst.WaitForExit();

            return (((uint)DPInst.ExitCode & 0x80000000) == 0);
        }

        public void Undo()
        {
            Process DPInst = Process.Start(DPInstPath, "/q /u " + InfFilePath);
            DPInst.WaitForExit();
        }

        public string GetExceptionMessage()
        {
            return "An error occured while running DPInst.exe!";
        }
    }
}
