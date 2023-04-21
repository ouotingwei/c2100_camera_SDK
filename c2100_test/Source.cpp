#include "stdafx.h"
#include "BenanoSDKExample.h"
#include "ScanSDK/BnScanSDK.h"
#include <iostream>


using namespace std;

#include <sstream>
#include <fstream>



CUserPointCloud::CUserPointCloud()
{
	m_PointCount = 0;
	m_PointCloud = nullptr;
	m_Normals = nullptr;
}

CUserPointCloud::~CUserPointCloud()
{
	_Release();
}

void CUserPointCloud::Allocate(unsigned int pointCount, float* XYZArray, float* normalArray)
{
	_Release();

	m_PointCount = pointCount;
	m_PointCloud = new float[m_PointCount * 3];
	m_Normals = new float[m_PointCount * 3];

	if (XYZArray != nullptr)
		memcpy(m_PointCloud, XYZArray, m_PointCount * 3 * sizeof(float));
	else
		memset(m_PointCloud, 0, m_PointCount * 3 * sizeof(float));

	if (normalArray != nullptr)
		memcpy(m_Normals, normalArray, m_PointCount * 3 * sizeof(float));
	else
		memset(m_Normals, 0, m_PointCount * 3 * sizeof(float));
}


unsigned int CUserPointCloud::GetPointCount()
{
	return m_PointCount;
}

float* CUserPointCloud::GetPointCloud()
{
	return m_PointCloud;
}

float* CUserPointCloud::GetNormals()
{
	return m_Normals;
}

void CUserPointCloud::_Release()
{
	m_PointCount = 0;
	if (m_PointCloud != nullptr)
	{
		delete[]m_PointCloud;
		m_PointCloud = nullptr;
	}

	if (m_Normals != nullptr)
	{
		delete[]m_Normals;
		m_Normals = nullptr;
	}
}

CBenanoSDKExample::CBenanoSDKExample(HWND hWnd, CListBox* plistMsg)
{
	m_hWnd = hWnd;

	InitializeCriticalSection(&m_Lock);

	_LoadLibrary();

	m_DeviceMng = nullptr;

	m_ScanManager = nullptr;

	m_OneViewUVZ = nullptr;
}

CBenanoSDKExample::~CBenanoSDKExample()
{
	if (m_ScanManager != nullptr) {
		delete m_ScanManager;
		m_ScanManager = nullptr;
	}

	if (m_OneViewUVZ != nullptr) {
		delete[] m_OneViewUVZ;
		m_OneViewUVZ = nullptr;
	}

	_FreeLibrary();

	EnterCriticalSection(&m_Lock);
	LeaveCriticalSection(&m_Lock);
	DeleteCriticalSection(&m_Lock);

	_DeletePointCloudList();

	_DeleteFeatureSetList();
}

void CBenanoSDKExample::_DeletePointCloudList()
{
	for (size_t i = 0; i < m_PointCloud.size(); i++)
	{
		delete m_PointCloud[i];
	}
	m_PointCloud.clear();
}


void CBenanoSDKExample::_DeleteFeatureSetList()
{
	for (size_t i = 0; i < m_FeatureSets.size(); i++)
	{
		delete m_FeatureSets[i];
	}
	m_FeatureSets.clear();
}

void CBenanoSDKExample::_LoadLibrary()
{
#ifdef _DEBUG
	::SetDllDirectory(L"C:\\Program Files\\Benano\\BenanoScanSDK\\Bin\\Debug\\");
	m_hModules.push_back(::LoadLibrary(L"BenanoScanSDK.dll"));
#else
	::SetDllDirectory(L"C:\\Program Files\\Benano\\BenanoScanSDK\\Bin\\Release\\");
	m_hModules.push_back(::LoadLibrary(L"BenanoScanSDK.dll"));
#endif

}

void CBenanoSDKExample::_FreeLibrary()
{
	for (int i = 0; i < m_hModules.size(); i++)
		::FreeLibrary(m_hModules[i]);
}

bool CBenanoSDKExample::InitialScanner()
{
	try
	{
		CInitialParam initParam;
		initParam.MovableDeviceType = eSDKMovableDeviceType::SDK_NoDevice;
		initParam.CalibrationFilePath = "D:\\Benano\\";
		initParam.MaxSolverCnt = 1;

		m_ScanManager = new CScanManager();
		m_ScanManager->InitialScanner(initParam);

		CPostProcessParam* postProcParam = m_ScanManager->GetParameterController().GetPostProcessParam();
		postProcParam->Data.ZMap_bEnableZMapGeneration = true;
		m_ScanManager->GetParameterController().ApplyPostProcessParam(postProcParam);

		_PostMessage("Initial Scanner Ok");

		return true;
	}
	catch (std::exception& ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);

	}
	return false;



}

void CBenanoSDKExample::CheckDeviceInfo()
{
	try
	{
		stringstream streamStr;

		int camId = 0;
		CDeviceInfo& deviceInfo = m_ScanManager->GetDeviceInfo();
		double resolutionX = deviceInfo.GetCameraResolutionX(camId);
		double resolutionY = deviceInfo.GetCameraResolutionY(camId);

		if (deviceInfo.IsConnected() == false)
		{
			_PostMessage("IsConnected: false");
			return;
		}

		streamStr.str("");
		streamStr << "ModuleType: " << deviceInfo.GetModuleType().c_str();
		_PostMessage(streamStr.str());

		streamStr.str("");
		streamStr << "SerialNumber: " << deviceInfo.GetSerialNumber().c_str();
		_PostMessage(streamStr.str());

		streamStr.str("");
		streamStr << "Camera Count: " << (int)deviceInfo.GetCameraCount();
		_PostMessage(streamStr.str());

		U8 camCnt = deviceInfo.GetCameraCount();
		for (U8 camId = 0; camId < camCnt; camId++)
		{
			streamStr.str("");
			streamStr << "Camera " << (int)camId <<
				", ImgWidth = " << deviceInfo.GetImageWidth(camId) <<
				", ImgHeight = " << deviceInfo.GetImageHeight(camId);
			_PostMessage(streamStr.str());


			deviceInfo.GetCameraResolutionX(camId);
			deviceInfo.GetCameraResolutionY(camId);
		}
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}

}

void CBenanoSDKExample::ReConnect()
{
	try
	{
		m_ScanManager->Reconnect();
		_PostMessage("Success to reconnect");
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}

}

void CBenanoSDKExample::AdjustBrightness()
{
	CLiveCtrl& liveCtrl = m_ScanManager->GetLiveController();

	try
	{
		U8 brightness = liveCtrl.GetBrightness();
		if (liveCtrl.SetBrightness(brightness) == false)
		{
			_PostMessage("Please check the scanner is connected.");
			return;
		}

		e2DLightBrightnessType lightType2D = e2DLightBrightnessType::eHighAngleLightBrightness;
		U8 brightness2D = liveCtrl.GetBrightness2D(lightType2D);
		if (liveCtrl.SetBrightness2D(brightness2D, lightType2D) == false)
		{
			_PostMessage("Please check the scanner is connected.");
			return;
		}

		U8 cameraGain = liveCtrl.GetCameraGain();
		if (liveCtrl.SetCameraGain(cameraGain) == false)
		{
			_PostMessage("Please check the scanner is connected.");
			return;
		}

		eExposureMode exposureMode = liveCtrl.GetExposureMode();
		if (liveCtrl.SetExposureMode(exposureMode) == false)
		{
			_PostMessage("Please check the scanner is connected.");
			return;
		}
		_PostMessage("Adjust brightness finished.");
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}
}

void CBenanoSDKExample::GetLiveImage()
{
	_Get2DLightImage(eSnap2DLightType::eHighAngleWhiteLight);
	_Get3DLightImage();
}


void CBenanoSDKExample::_Get2DLightImage(eSnap2DLightType lightType)
{
	try
	{
		CDeviceInfo& deviceInfo = m_ScanManager->GetDeviceInfo();
		if (deviceInfo.GetCameraCount() == 0)
		{
			_PostMessage("Can't connect to scanner");
			return;
		}

		U8 camIdx = 0;
		U32 imageWidth = deviceInfo.GetImageWidth(camIdx);
		U32 imageHeight = deviceInfo.GetImageHeight(camIdx);
		PU8 imageBufferRaw = new U8[imageWidth * imageHeight];

		CLiveCtrl& liveCtrl = m_ScanManager->GetLiveController();

		_PostMessage("Start 2D Snap");

		liveCtrl.Snap2DLightImage(lightType, camIdx, imageWidth, imageHeight, imageBufferRaw);
		CFileTool& fileTool = m_ScanManager->GetFileTool();
		fileTool.SaveGrayBufferToBitmap("C:\\Live2DLight.bmp", imageWidth, imageHeight, imageBufferRaw);
		_PostMessage("Save live image to C:\\Live2DLight.bmp");

		_PostMessage("Stop 2D Snap");

		delete[]imageBufferRaw;
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}
}

void CBenanoSDKExample::_Get3DLightImage()
{
	try
	{
		CDeviceInfo& deviceInfo = m_ScanManager->GetDeviceInfo();
		if (deviceInfo.GetCameraCount() == 0)
		{
			_PostMessage("Can't connect to scanner");
			return;
		}

		U8 camIdx = 0;
		U32 imageWidth = deviceInfo.GetImageWidth(camIdx);
		U32 imageHeight = deviceInfo.GetImageHeight(camIdx);
		PU8 imageBufferRaw = new U8[imageWidth * imageHeight];
		PU8 imageBufferMarked = new U8[imageWidth * imageHeight * 3];

		CLiveCtrl& liveCtrl = m_ScanManager->GetLiveController();

		if (liveCtrl.InitialLive(eLiveCtrlType::LiveCheckBrightness) == false)
		{
			_PostMessage("Failed to initial live. Please check the scanner is connected.");
			return;
		}

		_PostMessage("Start 3D Live");
		if (liveCtrl.StartLive() == false)
		{
			_PostMessage("Failed to start live. Please check the scanner is connected.");
			return;
		}

		CFileTool& fileTool = m_ScanManager->GetFileTool();
		while (true)
		{
			if (liveCtrl.GetLiveImageRaw(camIdx, imageWidth, imageHeight, imageBufferRaw) == true)
			{
				fileTool.SaveGrayBufferToBitmap("C:\\Live3DLight.bmp", imageWidth, imageHeight, imageBufferRaw);
				_PostMessage("Save live image to C:\\Live3DLight.bmp");
				break;
			}
		}

		if (liveCtrl.IsSupportMarkedImge() == true)
		{
			while (true)
			{
				if (liveCtrl.GetLiveImageMarked(camIdx, imageWidth, imageHeight, imageBufferMarked) == true)
				{
					fileTool.SaveColorBufferToBitmap("C:\\Live3DLightMarked.bmp", imageWidth, imageHeight, imageBufferMarked);
					_PostMessage("Save marked image to C:\\Live3DLightMarked.bmp");
					break;
				}
			}
		}

		liveCtrl.StopLive();
		_PostMessage("Stop 3D Live");

		delete[]imageBufferRaw;
		delete[]imageBufferMarked;
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}
}

void CBenanoSDKExample::ParameterSetting()
{
	try
	{
		_PostMessage("Start parameter setting");

		CParameterCtrl& paramCtrl = m_ScanManager->GetParameterController();

		//scan setting
		CScanParam* scanParam = paramCtrl.GetScanParam();
		if (scanParam != nullptr)
		{
			scanParam->SetImagingMode(eImagingMode::eImagingMode_Fast);
			scanParam->SetScanMode(eScanMode::eScanMode_Confirm);
			scanParam->SetEnableNormalCalculation(true);
			scanParam->SetEnableOutputMesh(true);
			paramCtrl.ApplyScanParam(scanParam);
		}
		else
		{
			_PostMessage("Failed to apply parameter. Please check the scanner is connected.");
			return;
		}

		//post process setting
		CPostProcessParam* postProcParam = paramCtrl.GetPostProcessParam();
		if (postProcParam != nullptr)
		{
			postProcParam->Data.ZMap_bEnableZMapGeneration = true;
			paramCtrl.ApplyPostProcessParam(postProcParam);
		}
		else
		{
			_PostMessage("Failed to apply parameter. Please check the scanner is connected.");
			return;
		}

		_PostMessage("End parameter setting");
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}


}

void CBenanoSDKExample::LoadProjectFile()
{
	try
	{
		CString szFilter = L"Project File(*.fsp)|*.fsp";
		CFileDialog fd(true, NULL, NULL, OFN_HIDEREADONLY, szFilter, NULL);
		if (fd.DoModal() == IDOK)
		{
			CString szFileName = fd.GetPathName();

			CT2CA pszConvertedAnsiString(szFileName);
			string strFilePath(pszConvertedAnsiString);

			CFileTool& fileTool = m_ScanManager->GetFileTool();
			fileTool.LoadProjectFile(strFilePath);
		}
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}
}

void CBenanoSDKExample::SaveProjectFile()
{
	try
	{
		CFileTool& fileTool = m_ScanManager->GetFileTool();
		fileTool.SaveProjectFile("C:\\BenanoSDKEx.fsp");
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}

}

bool CBenanoSDKExample::IsCalibrationReady()
{
	try
	{
		CCalibrator& calibrator = m_ScanManager->GetCalibrator();
		return calibrator.IsCalibrationReady();
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}
	return false;
}

bool CBenanoSDKExample::SetExtrinsicMatrix()
{
	try
	{
		CCalibrator& calibrator = m_ScanManager->GetCalibrator();

		U32 viewCnt = calibrator.GetViewCount();
		if (viewCnt < 1)
			calibrator.CreateViews(1, 0);

		CExtrinsicMatrix extrinsic = calibrator.GetExtrinsicMatrix(0, 0);
		return calibrator.ApplyExtrinsicMatrix(0, 0, &extrinsic);
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}

	return false;
}

bool CBenanoSDKExample::StartCalibration()
{
	try
	{
		_PostMessage("Calibration start");

		CCalibrator& calibrator = m_ScanManager->GetCalibrator();

		CCalibrationParam* param = calibrator.CreateCalibrationParam();
		if (param == nullptr)
			return false;

		if (typeid(*param) == typeid(CCalibNoDeviceParam))
		{
			CCalibNoDeviceParam* concreteParam = static_cast<CCalibNoDeviceParam*>(param);
		}
		else if (typeid(*param) == typeid(CCalibCustomerDeviceParam))
		{
			CCalibCustomerDeviceParam* concreteParam = static_cast<CCalibCustomerDeviceParam*>(param);
			concreteParam->SetViewCount(1);
		}
		else if (typeid(*param) == typeid(CCalibBenano1AxisRotaryTableParam))
		{
			CCalibBenano1AxisRotaryTableParam* concreteParam = static_cast<CCalibBenano1AxisRotaryTableParam*>(param);
			concreteParam->SetAngle(60);
		}
		else
		{
			CCalibNoDeviceParam* concreteParam = static_cast<CCalibNoDeviceParam*>(param);
		}

		calibrator.StartCalibration(param);

		if (calibrator.IsCalibrationStillRunning() == true)
			Sleep(1);

		if (param != nullptr)
		{
			delete param;//User can delete this object after call StartCalibrationFlow
			param = nullptr;
		}

		_PostMessage("Calibration finish");

		return true;
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}

	return false;
}

void CBenanoSDKExample::StopCalibration()
{
	try
	{
		CCalibrator& calibrator = m_ScanManager->GetCalibrator();
		calibrator.StopCalibrationFlow();
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}

}

bool CBenanoSDKExample::IsCalibrationStillRunning()
{
	try
	{
		CCalibrator& calibrator = m_ScanManager->GetCalibrator();
		return calibrator.IsCalibrationStillRunning();
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}
	return false;
}

bool CBenanoSDKExample::GetCalibrationResultStatus()
{
	try
	{
		CCalibrator& calibrator = m_ScanManager->GetCalibrator();
		CFunctionStatus& calibResult = calibrator.GetCalibrationResultStatus();

		return calibResult.IsOk();
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}
	return false;
}

void CBenanoSDKExample::ShowCalibrationErrorMessage()
{
	try
	{
		CCalibrator& calibrator = m_ScanManager->GetCalibrator();
		CFunctionStatus& calibResult = calibrator.GetCalibrationResultStatus();
		if (calibResult.IsOk() == false)
		{
			_PostMessage("Failed to calibrate, please check the following message: ");
			_PostMessage(calibResult.GetException().what());
		}
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}

}

void CBenanoSDKExample::CheckCalibrationData()
{
	try
	{
		CCalibrator& calibrator = m_ScanManager->GetCalibrator();

		CExtrinsicMatrix extrinsicMatrix = calibrator.GetExtrinsicMatrix(0, 0);
		PF64 ratationMatrix = extrinsicMatrix.GetRotationMatrix();
		PF64 translationVector = extrinsicMatrix.GetTranslationVector();

		CIntrinsicMatrix inrinsicMatrix = calibrator.GetIntrinsicMatrix(0);
		F64 Fx = inrinsicMatrix.GetFx();
		F64 Fy = inrinsicMatrix.GetFy();
		F64 U0 = inrinsicMatrix.GetU0();
		F64 V0 = inrinsicMatrix.GetV0();
		F64 skew = inrinsicMatrix.GetSkew();

		CDistortionCoeffs distortion = calibrator.GetDistortionCoeffs(0);
		double* distortionCoeffs = distortion.GetCoeffs();
		U8 coeffsCount = distortion.GetCoeffsCount();

		_PostMessage("Check calibration information finish");
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}

}

void CBenanoSDKExample::StartScan()
{
	try
	{
		CScanCtrl& scanCtrl = m_ScanManager->GetScanController();

		//Step1: set scan result callback
		CScanResultCallback rltCallback;
		rltCallback.SetViewDataResultCallback(this, _cbWrapperOnVieDatawResult);
		rltCallback.SetCanSendNextTriggerCallback(this, _cbWrapperOnCanSendNextTrigger);
		rltCallback.SetIgnoredTriggerCountCallback(this, _cbWrapperOnIgnoredTriggerCount);
		rltCallback.SetScanInterruptedCallback(this, _cbWrapperOnScanInterrupted);
		//rltCallback.SetCanTriggerExternalLightCallback(this, _cbWrapperOnCanTriggerExternalLight);
		rltCallback.SetLossFrameCallback(this, _cbWrapperOnLossFrame);
		//step2: create scan background thread
		scanCtrl.StartScan(rltCallback);

	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}
}

void CBenanoSDKExample::StopScan()
{
	_PostMessage("Start abort scan flow");
	try
	{
		CScanCtrl& scanCtrl = m_ScanManager->GetScanController();
		scanCtrl.StopScan();
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}

	_PostMessage("End abort scan flow");
}

void CBenanoSDKExample::_cbWrapperOnCanTriggerExternalLight(void* cbOwner)
{
	reinterpret_cast<CBenanoSDKExample*>(cbOwner)->_OnCanTriggerExternalLight();
}

void CBenanoSDKExample::_OnCanTriggerExternalLight()
{
	_PostMessage("Can trigger external light");
}


void CBenanoSDKExample::_cbWrapperOnCanSendNextTrigger(void* cbOwner, CCanSendNextTriggerInfo* grabDoneInfo)
{
	reinterpret_cast<CBenanoSDKExample*>(cbOwner)->_OnCanSendNextTrigger(grabDoneInfo);
}

void CBenanoSDKExample::_OnCanSendNextTrigger(CCanSendNextTriggerInfo* grabDoneInfo)
{
	_PostMessage("Can send next trigger");
}

void CBenanoSDKExample::_cbWrapperOnIgnoredTriggerCount(void* cbOwner, unsigned int ignoredTriggerCount)
{
	reinterpret_cast<CBenanoSDKExample*>(cbOwner)->_OnIgnoredTriggerCount(ignoredTriggerCount);
}

void CBenanoSDKExample::_OnIgnoredTriggerCount(unsigned int ignoredTriggerCount)
{
	_PostMessage("Ignored Trigger Count: " + to_string(ignoredTriggerCount));
}

//void CBenanoSDKExample::_cbWrapperOnOneViewResult(void *cbOwner, COneViewResult* viewResult)
//{
//	reinterpret_cast<CBenanoSDKExample*>(cbOwner)->_OnOneViewResult(viewResult);
//}
//
//void CBenanoSDKExample::_OnOneViewResult(COneViewResult* viewResult)
//{
//	EnterCriticalSection(&m_Lock);
//
//	_PostMessage("Start to process one view scan result");
//
//	CFileTool& fileTool = m_ScanManager->GetFileTool();
//
//
//	if (viewResult->Image2D != nullptr)
//	{
//		if (viewResult->ChannelCountOfImage2D == 1)
//		{
//			fileTool.SaveGrayBufferToBitmap("C:\\ViewImage_2D.bmp",
//				viewResult->ImageWidth,
//				viewResult->ImageHeight,
//				viewResult->Image2D);
//		}
//		else if (viewResult->ChannelCountOfImage2D == 3)
//		{
//			fileTool.SaveColorBufferToBitmap(
//				"C:\\ViewImage_2D.bmp",
//				viewResult->ImageWidth,
//				viewResult->ImageHeight,
//				viewResult->Image2D);
//		}
//	}
//
//	if (viewResult->ImageDepth != nullptr)
//	{
//		fileTool.SaveToRawImage(
//			"C:\\ViewImage_Depth.raw",
//			viewResult->ImageWidth,
//			viewResult->ImageHeight,
//			viewResult->ImageDepth);
//
//		//Get Zmap resolution
//		double resX = viewResult->ImageResolutionX;
//		double resY = viewResult->ImageResolutionY;
//
//	}
//
//	//ScanParam?ÑEnableOutputMesh?üËÉΩ?Ä?∫true
//	if (viewResult->PointCloudResult.MeshVertices != nullptr)
//	{
//		fileTool.SavePointCloudToSTLFile("C:\\mesh.stl",
//			viewResult->PointCloudResult.MeshVertices, viewResult->PointCloudResult.MeshCount,
//			viewResult->PointCloudResult.PointCloud, viewResult->PointCloudResult.PointsCount,
//			viewResult->PointCloudResult.Colors, viewResult->PointCloudResult.Normals);
//	}
//
//	if (viewResult->PointCloudResult.PointCloud != nullptr)
//	{
//		fileTool.SavePointCloudToXyzFile(
//			"C:\\PointCloud.xyz",
//			viewResult->PointCloudResult.PointsCount,
//			viewResult->PointCloudResult.PointCloud);
//	}
//
//	if (viewResult->PointCloudResult.Normals != nullptr)
//	{
//		fileTool.SavePointCloudToXyzFile(
//			"C:\\Normals.xyz",
//			viewResult->PointCloudResult.PointsCount,
//			viewResult->PointCloudResult.Normals);
//	}
//
// 
//	size_t cameraCnt = viewResult->CameraResult.size();
//	for (size_t camIdx = 0; camIdx < cameraCnt; camIdx++)
//	{
//		CCameraRawData* camRawData = viewResult->CameraResult[camIdx];
//		PF32 pDepth = camRawData->ImageDepth;
//		U32 width = camRawData->ImageWidth;
//		U32 height = camRawData->ImageHeight;
//
//		if (camRawData->ImageWhiteLightCoaxial != nullptr)
//		{
//			fileTool.SaveGrayBufferToBitmap("C:\\WhiteLightCoaxial.bmp",
//				camRawData->ImageWidth,
//				camRawData->ImageHeight,
//				camRawData->ImageWhiteLightCoaxial);
//		}
//
//		if (camRawData->ImageWhiteLightHighAngle != nullptr)
//		{
//			fileTool.SaveGrayBufferToBitmap("C:\\WhiteLightHighAngle.bmp",
//				camRawData->ImageWidth,
//				camRawData->ImageHeight,
//				camRawData->ImageWhiteLightHighAngle);
//		}
//
//		if (camRawData->ImageWhiteLightLowAngle != nullptr)
//		{
//			fileTool.SaveGrayBufferToBitmap("C:\\WhiteLightLowAngle.bmp",
//				camRawData->ImageWidth,
//				camRawData->ImageHeight,
//				camRawData->ImageWhiteLightLowAngle);
//		}
//
//		U32 externalLightCount = camRawData->GetExternalLightImageCount();
//		for (U32 i = 0; i < externalLightCount; i++)
//		{
//			fileTool.SaveGrayBufferToBitmap("C:\\ExternalLight_" + to_string(i) + ".bmp",
//				camRawData->ImageWidth,
//				camRawData->ImageHeight,
//				camRawData->GetExternalLightImageBuffer(i));
//		}
//	}
//
//	_PostMessage("End to Process one view scan result");
//
//	LeaveCriticalSection(&m_Lock);
//}


void CBenanoSDKExample::_cbWrapperOnScanInterrupted(void* cbOwner, CScanInterruptedInfo* scanInterruptedInfo)
{
	reinterpret_cast<CBenanoSDKExample*>(cbOwner)->_OnScanInterrupted(scanInterruptedInfo);
}

void CBenanoSDKExample::_OnScanInterrupted(CScanInterruptedInfo* scanInterruptedInfo)
{
	if (scanInterruptedInfo != nullptr)
	{
		string msg = "";
		msg += scanInterruptedInfo->ExceptionData.what();
	}
}

void CBenanoSDKExample::_cbWrapperOnFICPResult(void* cbOwner, COneScanResult* scanResult)
{
	reinterpret_cast<CBenanoSDKExample*>(cbOwner)->_OnFICPResult(scanResult);
}

void CBenanoSDKExample::_OnFICPResult(COneScanResult* scanResult)
{
	EnterCriticalSection(&m_Lock);

	_PostMessage("Start to process one scan result");

	CParameterCtrl& paramCtrl = m_ScanManager->GetParameterController();
	if (paramCtrl.GetPostProcessParam()->Data.bDetect3DFeaturePoints)	//If search feature circle point is opened, then put found feature points into alignment class
	{
		int maxDataCount = 3;		//Limit multi view count. Abandoned it if not necessary 
		if (m_FeatureSets.size() >= maxDataCount)
		{
			_PostMessage("Exceed maximun view setting, please check 'maxDataCount'");
			return;
		}

		if (m_FeatureSets.size() < maxDataCount)
		{
			CAlignmentFeatureSet* featureSet = new CAlignmentFeatureSet();
			featureSet->CloneFrom(&scanResult->AlignmentFeatureSet);
			m_FeatureSets.push_back(featureSet);		//Put found features into alignment tool
		}

		if (m_PointCloud.size() < maxDataCount)
		{
			CUserPointCloud* userPointCloud = new CUserPointCloud();
			userPointCloud->Allocate(
				scanResult->PointCloudResult.PointsCount,
				scanResult->PointCloudResult.PointCloud,
				scanResult->PointCloudResult.Normals);
			m_PointCloud.push_back(userPointCloud);		//Save corresponding point cloud in order to exam the alignment results
		}

		_PostMessage("End to set feature point information");
	}
	else
	{
		_PostMessage("Feature detection is not enable");
	}

	_PostMessage("End to process one scan result");

	LeaveCriticalSection(&m_Lock);
}

void CBenanoSDKExample::_cbWrapperOnApplyOneScanCalibration(void* cbOwner, COneScanResult* scanResult)
{
	reinterpret_cast<CBenanoSDKExample*>(cbOwner)->_OnApplyOneScanCalibration(scanResult);
}

void CBenanoSDKExample::_OnApplyOneScanCalibration(COneScanResult* scanResult)
{
	EnterCriticalSection(&m_Lock);

	_PostMessage("Start to process one scan result");

	const int totalViews = 3;	//Total amount of views
	const int multiviewIdx = 1;		//Set view index, i.e., the view you want to solve 

	//Get feature ICP [R|T] from file
	CAlignmentTool multiViewAlignmentTool;
	multiViewAlignmentTool.SetInfoByInput(totalViews);
	for (int view = 0; view < totalViews; view++)
	{
		AlignmentTransformation* transformation = multiViewAlignmentTool.GetTransform(view);
		if (!transformation->LoadTransformationInfoToFile("C:\\MultiViewRT_View" + std::to_string(view) + ".txt"))
		{
			_PostMessage("Cannot open file at 'C:\\MultiViewRT_View" + std::to_string(view) + ".txt'");
			_PostMessage("Failed to apply calibration on current view");
			return;
		}
	}

	//Get scan point cloud result
	CUserPointCloud* userPointCloud = new CUserPointCloud();
	userPointCloud->Allocate(
		scanResult->PointCloudResult.PointsCount,
		scanResult->PointCloudResult.PointCloud,
		scanResult->PointCloudResult.Normals);

	//Apply [R|T] on point cloud
	float* pPointCloud = userPointCloud->GetPointCloud();
	float* pNormal = userPointCloud->GetNormals();
	unsigned int pointCnt = userPointCloud->GetPointCount();

	multiViewAlignmentTool.AlignXyzByTransform(multiviewIdx, pPointCloud, pointCnt);
	multiViewAlignmentTool.AlignNormalByTransform(multiviewIdx, pNormal, pointCnt);


	//Save point cloud and normal results
	CFileTool& fileTool = m_ScanManager->GetFileTool();
	string fileName = "C:\\PointCloud_Alignment_OnCurrent_View" + to_string(multiviewIdx) + ".xyz";
	fileTool.SavePointCloudToXyzFile(fileName, pointCnt, pPointCloud, pNormal, nullptr);

	_PostMessage("End to apply multi-view " + std::to_string(multiviewIdx) + " on current scan");

	_PostMessage("End to process one scan result");

	LeaveCriticalSection(&m_Lock);
}

void CBenanoSDKExample::_cbWrapperOnVieDatawResult(void* cbOwner, CViewDataResult* viewResult)
{
	reinterpret_cast<CBenanoSDKExample*>(cbOwner)->_OnViewDataResult(viewResult);

}

void CBenanoSDKExample::_OnViewDataResult(CViewDataResult* viewResult)
{
	CFileTool& fileTool = m_ScanManager->GetFileTool();

	if (viewResult->PointCloudResult.PointCloud != nullptr)
	{
		fileTool.SavePointCloudToXyzFile("C:\\PointCloud.xyz",
			viewResult->PointCloudResult.PointsCount,
			viewResult->PointCloudResult.PointCloud);
	}

	if (viewResult->ZmapResult->ImageRaw != nullptr)
	{//Need to set ZMap_bEnableZMapGeneration=true at CPostProcessParam

		CImageConverter& imageConverter = m_ScanManager->GetCImageConverter();
		float* raw = viewResult->ZmapResult->ImageRaw;
		U32 width = viewResult->ZmapResult->ImageWidth;
		U32 height = viewResult->ZmapResult->ImageHeight;

		unsigned char* color = new unsigned char[width * height * 3];

		imageConverter.DepthRawToColorHeight(raw, width, height, color);

		fileTool.SaveColorBufferToBitmap("C:\\DepthColorHeight.bmp", width, height, color);

		delete[] color;
	}

}

void CBenanoSDKExample::_cbWrapperOnLossFrame(void* cbOwner, CLossFrameInfo* lossFrameInfo)
{
	reinterpret_cast<CBenanoSDKExample*>(cbOwner)->_OnLossFrame(lossFrameInfo);
}

void CBenanoSDKExample::_OnLossFrame(CLossFrameInfo* lossFrameInfo)
{
	_PostMessage("Loss frame serialNumber:" + to_string(lossFrameInfo->GetScanSerialNumber()));
}

void CBenanoSDKExample::ExecuteDataProcessingTool()
{
	_ExecuteAlignmentTool();
	//_ExecuteFreeMergeTool();
}

void CBenanoSDKExample::_ExecuteAlignmentTool()
{
	if (m_PointCloud.size() < 2)
	{
		_PostMessage("No enought view to align : view count < 2");
		return;
	}

	if (m_PointCloud.size() != m_FeatureSets.size())
	{
		_PostMessage("Point cloud size != feature set size");
		return;
	}

	bool bFeaturePass = true;
	for (int i = 0; i < m_FeatureSets.size(); i++)
	{
		if (m_FeatureSets[i]->FineFeatureGroup.size() == 0)
		{
			_PostMessage("Find no features at View " + std::to_string(i));
			bFeaturePass = false;
		}
	}
	if (!bFeaturePass)
		return;

	int dataCount = (int)m_PointCloud.size();

	CAlignmentTool alignmentTool;

	for (int i = 0; i < dataCount; i++)
	{
		alignmentTool.AddFeature(m_FeatureSets[i]);
	}

	//Calculate transformation
	if (alignmentTool.CalculateTransform() == false)
	{
		string message = "";
		for (int featureSetIdx = 0; featureSetIdx < dataCount; featureSetIdx++)
			message = alignmentTool.GetLatestMessage(featureSetIdx);
		return;
	}

	//Output scanning result by applying feature ICP [R|T]
	CFileTool& fileTool = m_ScanManager->GetFileTool();
	U32 transformCount = alignmentTool.GetFeatureSetCount();
	for (U32 i = 0; i < transformCount; i++)
	{
		//Apply Transformation on original point cloud
		float* pPointCloud = m_PointCloud[i]->GetPointCloud();
		float* pNormal = m_PointCloud[i]->GetNormals();
		unsigned int pointCnt = m_PointCloud[i]->GetPointCount();
		alignmentTool.AlignXyzByTransform(i, pPointCloud, pointCnt);
		alignmentTool.AlignNormalByTransform(i, pNormal, pointCnt);
		string fileName = "C:\\PointCloud_After_MultiViewAlignment_View" + to_string(i) + ".xyz";
		fileTool.SavePointCloudToXyzFile(fileName, pointCnt, pPointCloud, pNormal, nullptr);

		//Save transformation info to file
		AlignmentTransformation* transform = alignmentTool.GetTransform(i);
		transform->SaveTransformationInfoToFile("C:\\MultiViewRT_View" + std::to_string(i) + ".txt");
	}

	_PostMessage("End to export multiview [R|T] and aligned point cloud at 'C:\\'");
}

void CBenanoSDKExample::_ExecuteFreeMergeTool()
{
	CByteImagesMergeTool freeMerge;

	double scaleX = 10.298535;
	double scaleY = 10.298130;
	eFreeMergeDataType dataType = eFreeMergeDataType::eFreeMergeMono;

	CBnByteImage mergedImage;
	if (dataType == eFreeMergeDataType::eFreeMergeColor)
		mergedImage.SetSize(6110, 3380, 3);
	else
		mergedImage.SetSize(6110, 3380, 1);


	CMergeByteImagesInitParam initParm;
	initParm.DataType = dataType;
	initParm.MergedWidth = mergedImage.GetWidth();
	initParm.MergedHeight = mergedImage.GetHeight();
	initParm.MergedBuffer = mergedImage.GetBuffer();
	initParm.ScaleX = scaleX;
	initParm.ScaleY = scaleY;

	freeMerge.Initialize(&initParm);


	double fOffsetX[6] = { 0, 19, 38, 0, 19, 38 };
	double fOffsetY[6] = { 0, 0, 0, 14, 14, 14 };
	double fCenter[2] = { 2420 * scaleX / 2000., 2020 * scaleY / 2000. };

	int viewCnt = 6;
	for (int i = 0; i < viewCnt; i++)
	{
		std::string ss = "D:\\FreeMerge\\View" + to_string(i + 1) + "\\Output_2DG.bmp";
		CBnByteImage image;
		if (image.LoadBitmapImage(ss) == false)
			continue;

		CMergeByteImagesData data;
		data.Width = image.GetWidth();
		data.Height = image.GetHeight();
		data.ImageBuffer = image.GetBuffer();
		data.OffSetXMM = fOffsetX[i] + fCenter[0];
		data.OffsetYMM = fOffsetY[i] + fCenter[1];

		freeMerge.AddToMerge(&data);

	}

	freeMerge.FinalProcessToMergedBufer();

	mergedImage.SaveToBitmapFile("D:\\FreeMerge\\TestMerged.bmp");

}

void CBenanoSDKExample::DotBoardDetection()
{
	_PostMessage("Start to dot board detection");

	CDotBoardDetector& dotBoardDetector = m_ScanManager->GetDotBoardDetector();
	CBnByteImage* image = dotBoardDetector.CreateSnapImage();
	if (image == nullptr)
		return;
	dotBoardDetector.SnapDotBoard(image);

	CBnPoint3d dotBoardCenterPt = dotBoardDetector.DetectDotBoardCenter(image, eDotBoardType::DotBoard_50_100);

	AlignmentTransformation* transform = dotBoardDetector.GetTransform();
	transform->SaveTransformationInfoToFile("C:\\dot_board_RT.txt");

	delete image;

	_PostMessage("End dot board detection");
}

void CBenanoSDKExample::FeaturePointAssambling()
{
	try
	{
		//Reach postprocessing to open feature circle point searching
		CParameterCtrl& paramCtrl = m_ScanManager->GetParameterController();
		CScanParam* scanParam = paramCtrl.GetScanParam();
		CPostProcessParam* postProcParam = paramCtrl.GetPostProcessParam();
		if (postProcParam != nullptr)
		{
			postProcParam->Data.bDetect3DFeaturePoints = true;		//Enable feature circle point searching
			postProcParam->Data.FeaturePointSize = FICPMarkSize::MARKSIZE_12;		//Feature point size (circle diameter)
			postProcParam->Data.LimitAngle = 70;
			paramCtrl.ApplyPostProcessParam(postProcParam);
		}
		else
		{
			_PostMessage("Failed to apply parameter. Please check the scanner is connected.");
			return;
		}
		//If lighting / algorithm setting are needed, please check ParameterSetting()

		//Scan and save feature circle point informations in alignment tool
		CScanCtrl& scanCtrl = m_ScanManager->GetScanController();
		CScanResultCallback rltCallback;
		rltCallback.SetOneScanResultCallback(this, _cbWrapperOnFICPResult);
		scanCtrl.StartScan(rltCallback);

	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}
}

void CBenanoSDKExample::ApplyCalibrationRT()
{
	try
	{
		//Open Normal Calculation
		CParameterCtrl& paramCtrl = m_ScanManager->GetParameterController();
		CScanParam* scanParam = paramCtrl.GetScanParam();
		scanParam->SetEnableNormalCalculation(true);
		CPostProcessParam* postProcParam = paramCtrl.GetPostProcessParam();
		if (postProcParam != nullptr)
		{
			postProcParam->Data.bDetect3DFeaturePoints = false;
			paramCtrl.ApplyPostProcessParam(postProcParam);
		}
		paramCtrl.ApplyScanParam(scanParam);

		CScanCtrl& scanCtrl = m_ScanManager->GetScanController();
		CScanResultCallback rltCallback;
		rltCallback.SetOneScanResultCallback(this, _cbWrapperOnApplyOneScanCalibration);
		scanCtrl.StartScan(rltCallback);
	}
	catch (std::exception ex)
	{
		string msg = "";
		msg += ex.what();
		_PostMessage(msg);
	}


}

void CBenanoSDKExample::_ExternalLightSetting()
{
	CParameterCtrl& paramCtrl = m_ScanManager->GetParameterController();
	CScanParam* scanParam = paramCtrl.GetScanParam();
	if (scanParam != nullptr)
	{
		scanParam->SetExternalLightSyncMode(eExternalLightSyncMode::eExternalLightSyncByTimeDelay);

		//external light setting
		scanParam->CreateExternalLightParameter(1);
		U8 extLightCnt = scanParam->GetExternalLightCount();
		for (U8 index = 0; index < extLightCnt; index++)
		{
			CExternalLightParameter* externalLightParam = scanParam->GetExternalLightParameter(index);
			externalLightParam->CamDelayUs = 100;
			externalLightParam->CoolingTimeUs = 1000;
		}
		paramCtrl.ApplyScanParam(scanParam);
	}
	else
	{
		_PostMessage("Failed to apply parameter. Please check the scanner is connected.");
		return;
	}

	SaveProjectFile();

	_Get2DLightImage(eSnap2DLightType::eExternalLight0);
}


void CBenanoSDKExample::OtherTest()
{
	_TrainTileSurfaceSample();
	_CorrectTileSurfaceSample();
}


void CBenanoSDKExample::_TrainTileSurfaceSample()
{//?æÊ??°Ê≠£?ÑÂ??∏training

	//?àË??ñ‰??ãË®≠ÂÆöÊ?Ê°?
	char szSurfaceTrain[1024] = { 0 };
	char szCalibrationFileName[1024] = { 0 };
	char szSrcTileImage[1024] = { 0 };
	char szDestCorrectedImage[1024] = { 0 };

	double fX0, fY0, fX1, fY1;
	FILE* fileConfig = NULL;

	fopen_s(&fileConfig, "C:\\BenanaoTiltSampleConfig.txt", "r");
	fscanf_s(fileConfig, "%s\n", szSurfaceTrain);
	fscanf_s(fileConfig, "%lf,%lf\n", &fX0, &fY0);
	fscanf_s(fileConfig, "%lf,%lf\n", &fX1, &fY1);
	fscanf_s(fileConfig, "%s\n", szCalibrationFileName);
	fscanf_s(fileConfig, "%s\n", szSrcTileImage);
	fscanf_s(fileConfig, "%s\n", szDestCorrectedImage);

	fclose(fileConfig);

	///1. ËÆÄ?ñÂÖ©ÂºµÂΩ±??
	float* fImgRef, * fImgHeight, * fpTemp;
	uint32_t nVer, nW, nH, nChCnt, nImgArea;
	double fResX, fResY;
	FILE* file = NULL;

	//ËÆÄ?ñÂ??ÉÂπ≥?¢Á?rawÂΩ±Â?
	fopen_s(&file, szSurfaceTrain, "rb");
	fread_s(&nVer, sizeof(nVer), sizeof(UINT32), 1, file);
	fread_s(&nW, sizeof(nW), sizeof(UINT32), 1, file);
	fread_s(&nH, sizeof(nH), sizeof(UINT32), 1, file);
	fread_s(&nChCnt, sizeof(nChCnt), sizeof(UINT32), 1, file);
	fread_s(&fResX, sizeof(fResX), sizeof(double), 1, file);
	fread_s(&fResY, sizeof(fResY), sizeof(double), 1, file);

	nImgArea = nW * nH;
	fImgRef = new float[nImgArea];
	fread_s(fImgRef, nImgArea * sizeof(float), sizeof(float), nImgArea, file);
	fclose(file);


	///2. Â≠∏Á?Âπ≥Èù¢

	const int nRoiPos[2][2] = {	//?°Ê≠£?ÉËÄÉÂ??üÂú®ÂΩ±Â?‰∏≠Á?Â∑¶‰? & ?≥‰?XYÂ∫ßÊ?
		{ fX0, fY0 },{ fX1, fY1 }
	};

	const int nRoiW = abs(nRoiPos[0][0] - nRoiPos[1][0] + 1);	//ROIÂØ¨Â∫¶
	const int nRoiH = abs(nRoiPos[0][1] - nRoiPos[1][1] + 1);	//ROIÈ´òÂ∫¶
	const int nRoiArea = nRoiW * nRoiH;

	CBaseCorrectionTool* tool = new CBaseCorrectionTool(); //Ë≤†Ë≤¨Âπ≥Èù¢?°Ê≠£?ÑClass
	tool->SetDataCount(nRoiArea); //Ë®≠Â??êÂ??æÂÖ•?°Ê≠£?ÑÁ∏ΩÈªûÊï∏

	//fpTemp = fImgRef + (nRoiPos[0][1] * nW + nRoiPos[0][0]);
	for (int i = nRoiPos[0][1]; i <= nRoiPos[1][1]; i++)/*, fImgRef += (nW - nRoiW))*/
	{
		fpTemp = fImgRef + (i * nW + nRoiPos[0][0]);
		for (int j = nRoiPos[0][0]; j <= nRoiPos[1][0]; j++, fpTemp++)
		{//?äROI?ßÁ??∏Ê??æÂÖ•fitter‰∏≠Êì¨??
			if (*fpTemp == -99999)	continue;	//Â¶ÇÊ??áÂà∞Invalid Value, ÂøΩÁï•Ê≠§È?									
			tool->AddData(j * fResX / 1000., i * fResY / 1000., *fpTemp);
		}
	}

	tool->StartFitting(); //?≤Ë?Âπ≥Èù¢?¨Â?											
	CPlaneParams* planeParams = tool->GetPlaneParams(); //?ñÂ?Âπ≥Èù¢?ÉÊï∏

															  //Ê≠§Ê??ÑplaneParamsÂ∞±ÊòØÂ∞çÊ??ÑÂπ≥?¢Â???
	FILE* fileParam = NULL;

	fopen_s(&fileParam, szCalibrationFileName, "w");
	fprintf_s(fileParam, "%f,%f,%f,%f", planeParams->GetCoefficientX(), planeParams->GetCoefficientY(), planeParams->GetCoefficientZ());
	fclose(fileParam);
}

void CBenanoSDKExample::_CorrectTileSurfaceSample()
{
	//?àË??ñË®≠ÂÆöÊ?Ê°?
	char szSurfaceTrain[1024] = { 0 };
	char szCalibrationFileName[1024] = { 0 };
	char szSrcTileImage[1024] = { 0 };
	char szDestCorrectedImage[1024] = { 0 };

	double fX0, fY0, fX1, fY1;
	FILE* fileConfig = NULL;

	fopen_s(&fileConfig, "C:\\BenanaoTiltSampleConfig.txt", "r");
	fscanf_s(fileConfig, "%s\n", szSurfaceTrain);
	fscanf_s(fileConfig, "%lf,%lf\n", &fX0, &fY0);
	fscanf_s(fileConfig, "%lf,%lf\n", &fX1, &fY1);
	fscanf_s(fileConfig, "%s\n", szCalibrationFileName);
	fscanf_s(fileConfig, "%s\n", szSrcTileImage);
	fscanf_s(fileConfig, "%s\n", szDestCorrectedImage);

	fclose(fileConfig);

	//ËÆÄ?ñÂπ≥?¢Â???
	FILE* fileParam = NULL;

	double fParam[4] = { 0 };
	fopen_s(&fileParam, szCalibrationFileName, "r");
	fprintf_s(fileParam, "%f,%f,%f,%f", &(fParam[0]), &(fParam[1]), &(fParam[2]), &(fParam[3]));
	fclose(fileParam);

	//ËÆÄ?ñÈ?Â∫¶ÂΩ±??
	float* fImgHeight, * fpTemp;

	uint32_t nVer, nW, nH, nChCnt, nImgArea;
	double fResX, fResY;
	FILE* file = NULL;
	fopen_s(&file, szSrcTileImage, "rb");
	fread_s(&nVer, sizeof(nVer), sizeof(UINT32), 1, file);
	fread_s(&nW, sizeof(nW), sizeof(UINT32), 1, file);
	fread_s(&nH, sizeof(nH), sizeof(UINT32), 1, file);
	fread_s(&nChCnt, sizeof(nChCnt), sizeof(UINT32), 1, file);
	fread_s(&fResX, sizeof(fResX), sizeof(double), 1, file);
	fread_s(&fResY, sizeof(fResY), sizeof(double), 1, file);

	nImgArea = nW * nH;
	fImgHeight = new float[nImgArea];
	fread_s(fImgHeight, nImgArea * sizeof(float), sizeof(float), nImgArea, file);
	fclose(file);

	//?ùÂ?È´òÂ∫¶ÂΩ±Â??≤Ë??âÂπ≥
	fpTemp = fImgHeight;
	for (int i = 0; i < nH; i++)
	{
		for (int j = 0; j < nW; j++, fpTemp++)
		{
			if (*fpTemp == -99999)	continue;	//Â¶ÇÊ??áÂà∞Invalid Value, ÂøΩÁï•Ê≠§È?										
			*fpTemp = j * fResX / 1000. * fParam[0] //?çÊñ∞Ë®àÁ??∞Á?È´òÂ∫¶
				+ i * fResY / 1000. * fParam[1]
				+ *fpTemp * fParam[2];
		}
	}

	//?≤Â?rawÊ™îÊ?
	FILE* fileFlat = NULL;
	fopen_s(&fileFlat, szDestCorrectedImage, "wb");
	fwrite(&nVer, sizeof(nVer), 1, fileFlat);
	fwrite(&nW, sizeof(nW), 1, fileFlat);
	fwrite(&nH, sizeof(nH), 1, fileFlat);
	fwrite(&nChCnt, sizeof(nChCnt), 1, fileFlat);
	fwrite(&fResX, sizeof(fResX), 1, fileFlat);
	fwrite(&fResY, sizeof(fResY), 1, fileFlat);
	fwrite(fImgHeight, sizeof(float), nW * nH, fileFlat);
	fclose(fileFlat);
}

void CBenanoSDKExample::_PostMessage(string msg)
{
	string* data = new string(msg);
	PostMessageA(m_hWnd, WM_UPDATE_MSG, WPARAM(0), LPARAM(data));
}