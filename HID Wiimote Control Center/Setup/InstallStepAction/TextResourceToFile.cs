namespace HID_Wiimote_Control_Center.Setup.InstallStepAction
{
    class TextResourceToFile : IInstallStepAction
    {
        string OutputPath;
        string FileContent;

        public TextResourceToFile(string OutputPath, string FileContent)
        {
            this.OutputPath = OutputPath;
            this.FileContent = FileContent;
        }

        public bool Do()
        {
            if (System.IO.File.Exists(OutputPath))
            {
                return false;
            }

            using (System.IO.StreamWriter UninstallerStreamWriter = new System.IO.StreamWriter(OutputPath))
            {
                UninstallerStreamWriter.Write(FileContent);
            }

            return true;
        }

        public void Undo()
        {
            System.IO.File.Delete(OutputPath);
        }

        public string GetExceptionMessage()
        {
            return "An error occured while creating the uninstaller!";
        }
    }
}
