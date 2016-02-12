using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HID_Wiimote_Control_Center.Setup.InstallAction
{
    class CopyFile : IInstallAction
    {
        string FileName;
        string InstallDir;
        string InstallFilePath;

        public CopyFile(string FileName, string InstallDir)
        {
            this.FileName = FileName;
            this.InstallDir = InstallDir;

            this.InstallFilePath = System.IO.Path.Combine(InstallDir, FileName);
        }

        public bool Do()
        {
            if (System.IO.File.Exists(InstallFilePath))
            {
                return false;
            }

            System.IO.File.Copy(FileName, InstallFilePath);
            return true;
        }

        public void Undo()
        {
            System.IO.File.Delete(InstallFilePath);
        }

        public string GetExceptionMessage()
        {
            return "An error occured while copying files!";
        }
    }
}
