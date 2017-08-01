/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	IInstallStepAction.cs

Abstract:
	Interface for Installer tasks

*/
namespace HIDWiimote.ControlCenter.Setup.InstallStepAction
{
    interface IInstallStepAction
    {
        bool Do();
        void Undo();
        string GetExceptionMessage();
    }
}
