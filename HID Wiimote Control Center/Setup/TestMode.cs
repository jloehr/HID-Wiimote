using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;

namespace HID_Wiimote_Control_Center.Setup
{
    class TestMode : IInstallerTask
    {
        private const string BCDEdit = "bcdedit.exe";
        private const string Id = "{current}";
        private const string TestSigningEntryName = "testsigning";
        private const string TestSigningSetCommand = "-set TESTSIGNING ";
        private const string TestSigningSetCommandOn = TestSigningSetCommand + "ON";
        private const string TestSigningSetCommandOff = TestSigningSetCommand + "OFF";
        
        public bool IsGood()
        {
            string Result = RunBCDEdit("/enum " + Id);

            string[] Lines = Result.Split('\n');
            string TestsigningValue = Array.Find<string>(Lines, (string Line) => { return Line.StartsWith(TestSigningEntryName); });
            
            return ((TestsigningValue != null) && TestsigningValue.Contains("Yes"));
        }

        public void TryMakeBad()
        {
            RunBCDEdit(TestSigningSetCommandOff);
        }

        public void TryMakeGood()
        {
            RunBCDEdit(TestSigningSetCommandOn);
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
