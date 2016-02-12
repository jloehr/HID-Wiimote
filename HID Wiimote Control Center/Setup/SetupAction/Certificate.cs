using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

namespace HID_Wiimote_Control_Center.Setup.SetupAction
{
    class Certificate : ISetupAction
    {
        X509Store Store = new X509Store(StoreName.Root, StoreLocation.LocalMachine);
        X509Certificate2 CertificateInstance = new X509Certificate2(HID_Wiimote_Control_Center.Properties.Installer.JulianLoehrCA);        

        public bool IsSetUp()
        {
            bool Result;

            try
            {
                Store.Open(OpenFlags.ReadOnly);
                Result = Store.Certificates.Contains(CertificateInstance);
            }
            catch
            {
                Result = false;
            }
            finally
            {
                Store.Close();
            }

            return Result;
        }


        public void TrySetUp()
        {
            try
            {
                Store.Open(OpenFlags.ReadWrite);
                if (!Store.Certificates.Contains(CertificateInstance))
                {
                    Store.Add(CertificateInstance);
                }
            }
            finally
            {
                Store.Close();
            }
        }

        public void TryRevert()
        {
            try
            {
                Store.Open(OpenFlags.ReadWrite);
                if (Store.Certificates.Contains(CertificateInstance))
                {
                    Store.Remove(CertificateInstance);
                }
            }
            finally
            {
                Store.Close();
            }
        }
    }
}
