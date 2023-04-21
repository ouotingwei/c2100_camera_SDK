//var FPS = 30;
//Frame Grabber Free Run, On Stand By To Receive Image
//Triggers Are Sent To The Camera Input I/O, Camera Will Readout To Frame Grabber
//User Has Own Triggering Mechanism


var grabber = grabbers[0];

//Device Settings (Frame Grabber)
grabber.DevicePort.set("CameraControlMethod", "NC");


//Remote Device Setting (Camera)
grabber.RemotePort.set("CxpLinkConfigurationPreferredSwitch", "CXP6_X4");
grabber.RemotePort.set("CxpLinkConfiguration", "CXP6_X4");
grabber.RemotePort.set("Width", "8576");
grabber.RemotePort.set("Height", "6860");
grabber.RemotePort.set("OffsetX", "384");
grabber.RemotePort.set("OffsetY", "70");
grabber.RemotePort.set("PixelFormat", "Mono8");
grabber.RemotePort.set("TestPattern", "GreyDiagonalRampMoving");
//grabber.RemotePort.set("TestPattern", "GreyDiagonalRamp");
//grabber.RemotePort.set("TestPattern", "Off");
grabber.RemotePort.set("ReverseX", "False");
grabber.RemotePort.set("AcquisitionFrameRate", "32");
grabber.RemotePort.set("TriggerMode", "Off");
grabber.RemotePort.set("TriggerSource", "Line0");
grabber.RemotePort.set("TriggerActivation", "FallingEdge");
grabber.RemotePort.set("ExposureMode", "Timed"); 
grabber.RemotePort.set("ExposureTime", "30000");
grabber.RemotePort.set("BlackLevel", "10"); 
grabber.RemotePort.set("Gain", "1");
