/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	Certificate.cs

Abstract:
	Setup Action to install and uninstall a certificate

*/
using System.Security.Cryptography.X509Certificates;

namespace HIDWiimote.ControlCenter.Setup.SetupAction
{
    class Certificate : ISetupAction
    {
        X509Store Store = new X509Store(StoreName.Root, StoreLocation.LocalMachine);
        X509Certificate2 CertificateInstance = new X509Certificate2(HIDWiimote.ControlCenter.Properties.Installer.JulianLoehrCA);

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
