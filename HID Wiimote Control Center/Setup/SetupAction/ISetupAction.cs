namespace HIDWiimote.ControlCenter.Setup.SetupAction
{
    public interface ISetupAction
    {
        bool IsSetUp();
        void TrySetUp();
        void TryRevert();
    }
}
