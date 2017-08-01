/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	ISetupAction.cs

Abstract:
	Setup Action Interface

*/
namespace HIDWiimote.ControlCenter.Setup.SetupAction
{
    public interface ISetupAction
    {
        bool IsSetUp();
        void TrySetUp();
        void TryRevert();
    }
}
