namespace HID_Wiimote_Control_Center.Setup.SetupAction
{
    public interface ISetupAction
    {
        bool IsSetUp();
        void TrySetUp();
        void TryRevert();
    }
}
