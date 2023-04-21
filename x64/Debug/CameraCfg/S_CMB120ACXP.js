//var FPS = 186.9;
//Frame Grabber Free Run, On Stand By To Receive Image
//Triggers Are Sent To The Camera Input I/O, Camera Will Readout To Frame Grabber
//User Has Own Triggering Mechanism


var grabber = grabbers[0];

//Device Settings (Frame Grabber)
grabber.DevicePort.set("CameraControlMethod", "NC");


//Remote Device Setting (Camera)
//grabber.RemotePort.set("CxpLinkConfigurationPreferredSwitch", "CXP6_X4");
grabber.RemotePort.set("CxpLinkConfiguration", "CXP6_X4");
grabber.RemotePort.set("Width", "4096");
grabber.RemotePort.set("Height", "3072");
grabber.RemotePort.set("OffsetX", "0");
grabber.RemotePort.set("OffsetY", "0");
grabber.RemotePort.set("PixelFormat", "Mono8");
grabber.RemotePort.set("TestPattern", "GreyBar");
grabber.RemotePort.set("ReverseX", "False");
grabber.RemotePort.set("ReverseY", "False");
//grabber.RemotePort.set("AcquisitionFrameRate", "186.9");
grabber.RemotePort.set("TriggerMode", "On");
grabber.RemotePort.set("TriggerSource", "Line2");
//grabber.RemotePort.set("TriggerActivation", "FallingEdge");
grabber.RemotePort.set("ExposureMode", "Timed"); 
grabber.RemotePort.set("ExposureTime", "2000");
grabber.RemotePort.set("BlackLevel", "10"); 
grabber.RemotePort.set("Gain", "1");
grabber.RemotePort.set("LineInverter", "True");



