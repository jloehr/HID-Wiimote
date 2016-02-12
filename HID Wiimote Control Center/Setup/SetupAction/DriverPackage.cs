namespace HID_Wiimote_Control_Center.Setup.SetupAction
{

    class DriverPackage : ISetupAction
    {
        public bool IsSetUp()
        {
            return Setup.DriverPackage.IsInstalled();
        }

        public void TrySetUp()
        {
            Setup.DriverPackage.Install();
        }

        public void TryRevert()
        {
            Setup.DriverPackage.Uninstall();
        }
    }
}
