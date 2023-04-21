#pragma once
#ifndef _BenanoSDKExample_H_
#define _BenanoSDKExample_H_

#include <windows.h>
#include <vector>
#include <string>
#include <stdint.h>

using namespace std;

namespace benano_scansdk {
	class CLossFrameInfo;
	class CDeviceManager;
	class CScanManager;
	class COneViewResult;
	class COneScanResult;
	class CViewDataResult;
	class CAlignmentFeatureSet;
	class CCanSendNextTriggerInfo;
	class CScanInterruptedInfo;
	class CCameraRawData;
	class CDataProcessTool;
	enum eSnap2DLightType;
}
using namespace benano_scansdk;

class CUserPointCloud
{
public:

	CUserPointCloud();

	~CUserPointCloud();

	void Allocate(unsigned int pointCount, float* XYZArray, float* normalArray);

	unsigned int GetPointCount();

	float* GetPointCloud();

	float* GetNormals();

private:

	void _Release();

	unsigned int m_PointCount;

	float* m_PointCloud;

	float* m_Normals;

};

class CBenanoSDKExample final
{
public:

	CBenanoSDKExample(HWND hWnd, CListBox* plistMsg);

	~CBenanoSDKExample();


#pragma region Initial & Release

	bool InitialScanner();

	void CheckDeviceInfo();

	void ReConnect();

#pragma endregion



#pragma region Live

	void AdjustBrightness();

	void GetLiveImage();

#pragma endregion


#pragma region Setting&File

	void ParameterSetting();

	void LoadProjectFile();

	void SaveProjectFile();

#pragma endregion


#pragma region Calibration

	bool IsCalibrationReady();

	bool StartCalibration();

	void StopCalibration();

	bool IsCalibrationStillRunning();

	bool GetCalibrationResultStatus();

	void CheckCalibrationData();

	void ShowCalibrationErrorMessage();

	bool SetExtrinsicMatrix();

	void DotBoardDetection();

	void FeaturePointAssambling();

	void ApplyCalibrationRT();

#pragma endregion



#pragma region Scan

	void StartScan();

	void StopScan();

#pragma endregion

	void OtherTest();

	void ExecuteDataProcessingTool();

private:

	void _LoadLibrary();

	void _FreeLibrary();

	CRITICAL_SECTION m_Lock;

	CDeviceManager* m_DeviceMng;

	CScanManager* m_ScanManager;

	static void _cbWrapperOnCanTriggerExternalLight(void* cbOwner);

	void _OnCanTriggerExternalLight();

	static void _cbWrapperOnCanSendNextTrigger(void* cbOwner, CCanSendNextTriggerInfo* grabDoneInfo);

	void _OnCanSendNextTrigger(CCanSendNextTriggerInfo* grabDoneInfo);

	static void _cbWrapperOnIgnoredTriggerCount(void* cbOwner, unsigned int ignoredTriggerCount);

	void _OnIgnoredTriggerCount(unsigned int ignoredTriggerCount);

	static void _cbWrapperOnScanInterrupted(void* cbOwner, CScanInterruptedInfo* scanInterruptedInfo);

	void _OnScanInterrupted(CScanInterruptedInfo* scanInterruptedInfo);

	static void _cbWrapperOnFICPResult(void* cbOwner, COneScanResult* scanResult);

	void _OnFICPResult(COneScanResult* scanResult);

	static void _cbWrapperOnApplyOneScanCalibration(void* cbOwner, COneScanResult* scanResult);

	void _OnApplyOneScanCalibration(COneScanResult* scanResult);

	static void _cbWrapperOnVieDatawResult(void* cbOwner, CViewDataResult* viewResult);

	void _OnViewDataResult(CViewDataResult* viewResult);

	static void _cbWrapperOnLossFrame(void* cbOwner, CLossFrameInfo* viewResult);

	void _OnLossFrame(CLossFrameInfo* viewResult);


	vector<HMODULE> m_hModules;

	HWND m_hWnd;

	void _Get2DLightImage(eSnap2DLightType lightType);

	void _Get3DLightImage();

	vector<CAlignmentFeatureSet*> m_FeatureSets;

	vector<CUserPointCloud*> m_PointCloud;

	float* m_OneViewUVZ;

	void _DeletePointCloudList();

	void _DeleteFeatureSetList();

	void _ExecuteAlignmentTool();

	void _ExecuteFreeMergeTool();

	void _ExternalLightSetting();

	void _TrainTileSurfaceSample();

	void _CorrectTileSurfaceSample();

	void _PostMessage(string msg);

};

#endif#pragma once
