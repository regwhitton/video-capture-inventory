/*
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
#include <initguid.h>
#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <dshow.h>
#include <strmif.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mfapi.h>
#include <wchar.h>
#include <string.h>
#include "com_github_regwhitton_videocaptureinventory_VideoCaptureInventoryWin.h"

/*
 * Missing or wrong in MinGW 8.1.0 g++ (as available on Github hosted runner).
 */
EXTERN_GUID(tmp_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, 0xc60ac5fe,0x252a,0x478f,0xa0,0xef,0xbc,0x8f,0xa5,0xf7,0xca,0xd3);
EXTERN_GUID(tmp_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID, 0x8ac3587a,0x4ae7,0x42d8,0x99,0xe0,0x0a,0x60,0x13,0xee,0xf9,0x0f);
EXTERN_GUID(tmp_MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, 0x60d0e559,0x52f8,0x4fa2,0xbb,0xce,0xac,0xdb,0x34,0xa8,0xec,0x1);
EXTERN_GUID(tmp_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, 0x58f0aad8,0x22bf,0x4f8a,0xbb,0x3d,0xd2,0xc4,0x97,0x8c,0x6e,0x2f);

HRESULT WINAPI MFEnumDeviceSources(IMFAttributes *pAttributes,IMFActivate ***pppSourceActivate,UINT32 *pcSourceActivate);

/**
 * Used by JNI implementations to call methods in the Java VideoCaptureInventory class.
 */
class JavaVideoCaptureInventoryProxy
{
    private:
    JNIEnv *env;
    jobject target;
    jmethodID addDeviceMethodId;
    jmethodID addFormatMethodId;

    public:
    JavaVideoCaptureInventoryProxy(JNIEnv *env, jobject javaVideoCaptureInventory)
    {
        this->env = env;
        this->target = javaVideoCaptureInventory;

        jclass targetClass = env->GetObjectClass(target);
        this->addDeviceMethodId = env->GetMethodID(targetClass, "addDevice", "(ILjava/lang/String;)V");
        this->addFormatMethodId = env->GetMethodID(targetClass, "addFormat", "(II)V");
    }

    void AddDevice(jint deviceId, jstring name)
    {
        env->CallObjectMethod(this->target, this->addDeviceMethodId, deviceId, name);
    }

    void AddFormat(jint width, jint height)
    {
        env->CallObjectMethod(this->target, this->addFormatMethodId, width, height);
    }
};

class VideoCaptureInventory
{
    private:
    JNIEnv *env;
    JavaVideoCaptureInventoryProxy *proxy;

    public:
    VideoCaptureInventory(JNIEnv *env, JavaVideoCaptureInventoryProxy *proxy)
    {
        this->env = env;
        this->proxy = proxy;
    }

    HRESULT Populate()
    {
        return DescribeDevices();
    }

    private:
    HRESULT DescribeDevices()
    {
        HRESULT hr;
       
        if (SUCCEEDED(hr = CoInitializeEx(NULL, COINIT_MULTITHREADED))) {
            hr = DescribeDevicesUsingComThread();
            CoUninitialize();
        }
        return hr;
    }

    HRESULT DescribeDevicesUsingComThread()
    {
        HRESULT hr;
        IMFActivate **ppDeviceSources = NULL;
        UINT32 deviceCount;
    
        if (SUCCEEDED(hr = GetEnumDeviceSources(&ppDeviceSources, &deviceCount))) {
            for (DWORD deviceId=0; deviceId<deviceCount; deviceId++) {
                if (FAILED(hr = DescribeDevice(deviceId, ppDeviceSources[deviceId])))
                    break;
            }
            FreeEnumDeviceSources(ppDeviceSources, deviceCount);
        }
        return hr;
    }

    /**
     * On success, FreeEnumDeviceSources() should be used to release deviceSources.
     */
    HRESULT GetEnumDeviceSources(IMFActivate ***pppDeviceSources, UINT32 *pcDeviceSources)
    {
        HRESULT hr;
        IMFAttributes *pAttributes = NULL;

        if (SUCCEEDED(hr = MFCreateAttributes(&pAttributes, 1))) {
            if (SUCCEEDED(hr = pAttributes->SetGUID(tmp_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                           tmp_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))) {
                hr = MFEnumDeviceSources(pAttributes, pppDeviceSources, pcDeviceSources);
            }
            SafeRelease(&pAttributes);
        }
        return hr;
    }

    void FreeEnumDeviceSources(IMFActivate **ppDeviceSources, UINT32 deviceCount)
    {
        for (DWORD i=0; i < deviceCount; i++) {
            SafeRelease(&ppDeviceSources[i]);
        }
        CoTaskMemFree(ppDeviceSources);
    }

    HRESULT DescribeDevice(DWORD deviceId, IMFActivate *pDeviceSource)
    {
        HRESULT hr;
        char* pFriendlyName = NULL;
        char* pSymbolicLink = NULL;

        if (SUCCEEDED(hr = GetAttribute(pDeviceSource, tmp_MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &pFriendlyName))) {
            proxy->AddDevice(deviceId, env->NewStringUTF(pFriendlyName));
            free(pFriendlyName);

            if (SUCCEEDED(hr = GetAttribute(pDeviceSource, tmp_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
                            &pSymbolicLink))) {
                hr = DescribeDeviceFormats(pSymbolicLink);
                free(pSymbolicLink);
            }
        }

        return hr;
    }

    HRESULT DescribeDeviceFormats(char* pSymbolicLink)
    {
        HRESULT hr;
        TCHAR* pDeviceInstanceId;

        if (FAILED(hr = SymbolicLinkToDeviceInstanceId(pSymbolicLink, &pDeviceInstanceId))) 
            return hr;

        IMoniker* pMoniker;
        if (SUCCEEDED(hr = FindDeviceMoniker(pDeviceInstanceId, &pMoniker))) {
            hr = DescribeDeviceFormats(pMoniker);
            pMoniker->Release();
        }

        free(pDeviceInstanceId);
        return hr;
    }

    HRESULT DescribeDeviceFormats(IMoniker *pMoniker)
    {
        IBaseFilter *pFilter;
        HRESULT hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void **)&pFilter);
        if (FAILED(hr))
            return hr;

        hr = DescribeDeviceFormats(pFilter);

        pFilter->Release();
        return hr;
    }

    HRESULT DescribeDeviceFormats(IBaseFilter *pFilter)
    {
        IEnumPins *pEnum = NULL;
        HRESULT hr = pFilter->EnumPins(&pEnum);
        if (FAILED(hr))
            return hr;

        IPin *pPin = NULL;
        while (pEnum->Next(1, &pPin, 0) == S_OK)
        {
            hr = DescribeDeviceFormats(pPin);
            pPin->Release();
            if (FAILED(hr))
                break;
        }
        pEnum->Release();
        return hr;
    }

    HRESULT DescribeDeviceFormats(IPin *pPin)
    {
        /* Doesn't need to be released */
        PIN_INFO PinInfo;
        HRESULT hr = pPin->QueryPinInfo(&PinInfo);
        if (FAILED(hr))
            return hr;

        // Only interested in Video Capture aspect of devices (could be "Still").
        if (PinInfo.dir != PINDIR_OUTPUT || wcscmp(PinInfo.achName, L"Capture") != 0)
            return S_OK;

        IEnumMediaTypes *mediaTypesEnumerator = NULL;
        hr = pPin->EnumMediaTypes(&mediaTypesEnumerator);
        if (FAILED(hr))
            return hr;

        AM_MEDIA_TYPE *mediaType = NULL;
        while (mediaTypesEnumerator->Next(1, &mediaType, NULL) == S_OK)
        {
            hr = DescribeDeviceFormat(mediaType);
            _DeleteMediaType(mediaType);
            if (FAILED(hr))
                break;
        }
        mediaTypesEnumerator->Release();
        return hr;
    }

    HRESULT DescribeDeviceFormat(AM_MEDIA_TYPE *mediaType)
    {
        // Only interested in video formats.
        if ((mediaType->formattype == FORMAT_VideoInfo) &&
            (mediaType->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
            (mediaType->pbFormat != NULL))
        {
            VIDEOINFOHEADER *vih = (VIDEOINFOHEADER *)mediaType->pbFormat;
            LONG width = vih->bmiHeader.biWidth;
            LONG height = vih->bmiHeader.biHeight;

            proxy->AddFormat(width, height);
        }
        return S_OK;
    }

    /**
     * On success, *ppMoniker needs to be released.
     */
    HRESULT FindDeviceMoniker(TCHAR* pDeviceInstanceId, IMoniker** ppMoniker)
    {
        HRESULT hr;
        IEnumMoniker *pEnum;

        if (FAILED(hr = CreateDeviceMonikerEnumerator(&pEnum)))
            return hr;
 
        IMoniker *pMoniker = NULL;
        while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
        {
            BOOL monikerHasDevInsId;
            if (FAILED(hr = HadDeviceInstanceId(pMoniker, pDeviceInstanceId, &monikerHasDevInsId)))
                return hr;

            if (monikerHasDevInsId) {
                pEnum->Release();
                *ppMoniker = pMoniker;
                return S_OK;
            }
        }
        pEnum->Release();
        // We should be able to find the moniker, as we know this device exists.
        return VFW_E_NOT_FOUND;
    }

    /**
     * On success, *ppEnum needs to be released.
     */
    HRESULT CreateDeviceMonikerEnumerator(IEnumMoniker **ppEnum)
    {
        // Create the System Device Enumerator.
        ICreateDevEnum *pDevEnum;
        HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
                                    CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

        if (FAILED(hr))
            return hr;

        // Create an enumerator for the category.
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, ppEnum, 0);
        if (hr == S_FALSE) {
            // The category is empty. Treat as an error.
            hr = VFW_E_NOT_FOUND;
        }

        pDevEnum->Release();
        return hr;
    }

    HRESULT HadDeviceInstanceId(IMoniker* pMoniker, TCHAR* pDeviceInstanceId, BOOL* pResult)
    {
        HRESULT hr;

        char* pDevicePath;
        if (FAILED(hr = GetMonikerDevicePath(pMoniker, &pDevicePath)))
            return hr;

        TCHAR* pDevInsId;
        if (FAILED(hr = SymbolicLinkToDeviceInstanceId(pDevicePath, &pDevInsId)))
            return hr;

        *pResult = _tcscmp(pDevInsId, pDeviceInstanceId) == 0;
        return S_OK;
    }

    /**
     * On success, *pDevicePath needs to be freed.
     */
    HRESULT GetMonikerDevicePath(IMoniker *pMoniker, char** ppDevicePath)
    {
        HRESULT hr;
        IPropertyBag *pPropBag;

        if (SUCCEEDED(hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag)))) {
            hr = GetPropertyFromPropertyBag(pPropBag, L"DevicePath", ppDevicePath);
            pPropBag->Release();
        }
        return hr;
    }

    /**
     * On success, *ppValue needs to be freed.
     */
    HRESULT GetPropertyFromPropertyBag(IPropertyBag *pPropBag, LPCOLESTR propertyName, char** ppValue)
    {
        HRESULT hr;
        VARIANT var;

        VariantInit(&var);
        hr = pPropBag->Read(propertyName, &var, 0);
        if (SUCCEEDED(hr))
        {
            *ppValue = WideCharToMultiByteString(var.bstrVal);
        }
        VariantClear(&var);
        return hr;
    }

    /**
     * On success, *ppDeviceInstanceId needs to be freed.
     */
    HRESULT SymbolicLinkToDeviceInstanceId(char* pSymbolicLink, TCHAR** ppDeviceInstanceId)
    {
        HDEVINFO deviceInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
        if (deviceInfoSet == INVALID_HANDLE_VALUE) {
            return E_FAIL;
        }

        HRESULT hr = SymbolicLinkToDeviceInstanceId(deviceInfoSet, pSymbolicLink, ppDeviceInstanceId);

        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return hr;
    }

    /**
     * On success, *ppDeviceInstanceId needs to be freed.
     */
    HRESULT SymbolicLinkToDeviceInstanceId(HDEVINFO deviceInfoSet, char* pSymbolicLink, TCHAR** ppDeviceInstanceId)
    {
        if (!SetupDiOpenDeviceInterfaceA(deviceInfoSet, pSymbolicLink, 0, NULL)) {
            return E_FAIL;
        }

        SP_DEVINFO_DATA DeviceInfoData;
        ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
        DeviceInfoData.cbSize = sizeof (SP_DEVINFO_DATA);

        // Only expecting one device in enumerator with this symbolic link.
        if (!SetupDiEnumDeviceInfo(deviceInfoSet, 0, &DeviceInfoData)) {
            return E_FAIL;
        }

        ULONG sizeInTChars;
        if (CR_SUCCESS != CM_Get_Device_ID_Size(&sizeInTChars, DeviceInfoData.DevInst, 0)) {
            return E_FAIL;
        }

        TCHAR* pDeviceInstanceId = (TCHAR*) malloc((sizeInTChars+1)*sizeof(TCHAR));
        if (CR_SUCCESS != CM_Get_Device_ID(DeviceInfoData.DevInst, pDeviceInstanceId , sizeInTChars+1, 0)) {
            free(pDeviceInstanceId);
            return E_FAIL;
        }
        pDeviceInstanceId[sizeInTChars] = 0;

        *ppDeviceInstanceId = pDeviceInstanceId;
        return S_OK;
    }
                
    /**
     * On success, *ppValue needs to be freed.
     */
    HRESULT GetAttribute(IMFActivate *pDeviceSource, REFGUID guidKey, char ** ppValue)
    {
        LPWSTR  pWideValue = NULL;
        UINT32  valueLength;

        HRESULT hr = pDeviceSource->GetAllocatedString(guidKey, &pWideValue, &valueLength);
        if (FAILED(hr))
            return hr;

        *ppValue = WideCharToMultiByteString(pWideValue);
        CoTaskMemFree(pWideValue);
        return S_OK;
    }

    /**
     * Value returned needs to be freed.
     */
    char* WideCharToMultiByteString(LPWSTR  pWideCharString)
    {
        int len = wcslen(pWideCharString);
        int bytesNeeded = WideCharToMultiByte(CP_UTF8, 0, pWideCharString, len, NULL, 0, NULL, NULL);
        char * pMultiByteString = (char *)malloc(bytesNeeded + 1);
        WideCharToMultiByte(CP_UTF8, 0, pWideCharString, len, pMultiByteString, bytesNeeded, NULL, NULL);
        pMultiByteString[bytesNeeded] = 0;
        return pMultiByteString;
    }

    /** Delete a media type structure that was allocated on the heap. */
    void _DeleteMediaType(AM_MEDIA_TYPE *pmt)
    {
        if (pmt != NULL)
        {
            _FreeMediaType(*pmt);
            CoTaskMemFree(pmt);
        }
    }

    /** Release the format block for a media type. */
    void _FreeMediaType(AM_MEDIA_TYPE &mt)
    {
        if (mt.cbFormat != 0)
        {
            CoTaskMemFree((PVOID)mt.pbFormat);
            mt.cbFormat = 0;
            mt.pbFormat = NULL;
        }
        if (mt.pUnk != NULL)
        {
            // pUnk should not be used.
            mt.pUnk->Release();
            mt.pUnk = NULL;
        }
    }

    template <class T> void SafeRelease(T **ppT)
    {
        if (*ppT)
        {
            (*ppT)->Release();
            *ppT = NULL;
        }
    }
};

JNIEXPORT jint JNICALL Java_com_github_regwhitton_videocaptureinventory_VideoCaptureInventoryWin_populate
  (JNIEnv * env, jobject thisObject)
{
    JavaVideoCaptureInventoryProxy proxy(env, thisObject);
    VideoCaptureInventory vci(env, &proxy);
    return (jint) vci.Populate();
}
