namespace HIDWiimote.ControlCenter.Setup.InstallStepAction
{
    interface IInstallStepAction
    {
        bool Do();
        void Undo();
        string GetExceptionMessage();
    }
}
