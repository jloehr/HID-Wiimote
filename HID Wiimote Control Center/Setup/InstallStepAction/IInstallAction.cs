namespace HID_Wiimote_Control_Center.Setup.InstallStepAction
{
    interface IInstallStepAction
    {
        bool Do();
        void Undo();
        string GetExceptionMessage();
    }
}
