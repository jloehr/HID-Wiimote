using System.Windows;
using System.Diagnostics;
using System.IO;
using System;
using System.Security.Principal;
using System.Security.Cryptography.X509Certificates;

namespace TinyInstaller
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        const string DPInstFile = "dpinst.exe";

        public static bool IsAdministrator()
        {
            var identity = WindowsIdentity.GetCurrent();
            var principal = new WindowsPrincipal(identity);
            return principal.IsInRole(WindowsBuiltInRole.Administrator);
        }

        private void MyRun(object sender, StartupEventArgs e)
        {
            if (!IsAdministrator())
            {
                MessageBox.Show("You need to run this as Administrator.");
                this.Shutdown();
                return;
            }
            
            string DPInstPath = Path.GetFullPath(DPInstFile);

            InstallCertificate();            

            if (!File.Exists(DPInstPath))
            {
                MessageBox.Show("Error: can't find \"" + DPInstFile + "\". Please run it manually.");
                this.Shutdown();
                return;
            }

            try
            {
                Process DPInst = Process.Start(DPInstPath);
                DPInst.WaitForExit();
            }
            catch
            {
                MessageBox.Show("Error: can't start the driver package installation, please run \"" + DPInstFile + "\" manually.");
            }

            this.Shutdown();
        }

        private void InstallCertificate()
        {
            X509Store Store = new X509Store(StoreName.Root, StoreLocation.LocalMachine);
            X509Certificate2 CertificateInstance = new X509Certificate2(TinyInstaller.Resources.JulianLoehrCA);

            try
            {
                Store.Open(OpenFlags.ReadWrite);
                if (!Store.Certificates.Contains(CertificateInstance))
                {
                    Store.Add(CertificateInstance);
                }
            }
            catch (Exception e)
            {
                MessageBox.Show("Error: Can't install Certificate! Reason: " + e.Message);
            }
            finally
            {
                Store.Close();
            }
        }
    }
}
