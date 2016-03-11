using System;
using System.Diagnostics;

namespace HIDWiimote.ControlCenter.Setup.SetupAction
{
    class TestMode : ISetupAction
    {
        private const string BCDEdit = "bcdedit.exe";
        private const string Id = "{current}";
        private const string TestSigningEntryName = "testsigning";
        private const string TestSigningSetCommand = "-set TESTSIGNING ";
        private const string TestSigningSetCommandOn = TestSigningSetCommand + "ON";
        private const string TestSigningSetCommandOff = TestSigningSetCommand + "OFF";

        public bool IsSetUp()
        {
            string Result = RunBCDEdit("/enum " + Id);

            string[] Lines = Result.Split('\n');
            string TestsigningValue = Array.Find<string>(Lines, (string Line) => { return Line.StartsWith(TestSigningEntryName); });

            return ((TestsigningValue != null) && TestsigningValue.Contains("Yes"));
        }

        public void TrySetUp()
        {
            RunBCDEdit(TestSigningSetCommandOn);
        }

        public void TryRevert()
        {
            RunBCDEdit(TestSigningSetCommandOff);
        }

        private string RunBCDEdit(string Argument)
        {
            ProcessStartInfo StartInfo = new ProcessStartInfo(BCDEdit, Argument);
            StartInfo.RedirectStandardOutput = true;
            StartInfo.UseShellExecute = false;
            StartInfo.CreateNoWindow = true;
            StartInfo.WindowStyle = ProcessWindowStyle.Hidden;

            Process New = Process.Start(StartInfo);
            New.WaitForExit();

            return New.StandardOutput.ReadToEnd();
        }
    }
}
