#include <initguid.h>
#include <windows.h>
#include <dshow.h>
#include <strmif.h>
#include <wchar.h>
#include <string.h>
#include "com_github_regwhitton_videocaptureinventory_VideoCaptureInventoryWin64.h"

#pragma comment(lib, "strmiids")

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
        this->addDeviceMethodId = env->GetMethodID(targetClass, "addDevice", "(Ljava/lang/String;)V");
        this->addFormatMethodId = env->GetMethodID(targetClass, "addFormat", "(II)V");
    }

    void AddDevice(jstring name)
    {
        env->CallObjectMethod(this->target, this->addDeviceMethodId, name);
    }

    void AddFormat(LONG width, LONG height)
    {
        env->CallObjectMethod(this->target, this->addFormatMethodId,
            (jint)width, (jint)height);
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
        return DescribeDevicesUsingComThread();
    }

    private:
    HRESULT DescribeDevicesUsingComThread()
    {
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr))
            return hr;
 
        hr = DescribeDevices();

        CoUninitialize();
        return hr;
    }

    HRESULT DescribeDevices()
    {
        IEnumMoniker *pEnum;
        HRESULT hr = CreateDeviceEnumerator(&pEnum);
        if (FAILED(hr))
            return hr;
 
        IMoniker *pMoniker = NULL;
        while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
        {
            hr = DescribeDevice(pMoniker);
            pMoniker->Release();
            if (FAILED(hr))
                break;
        }
        pEnum->Release();
        return hr;
    }

    HRESULT CreateDeviceEnumerator(IEnumMoniker **ppEnum)
    {
        // Create the System Device Enumerator.
        ICreateDevEnum *pDevEnum;
        HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
                                    CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
        if (FAILED(hr))
            return hr;

        // Create an enumerator for the category.
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, ppEnum, 0);
        if (hr == S_FALSE)
        {
            hr = VFW_E_NOT_FOUND; // The category is empty. Treat as an error.
        }

        pDevEnum->Release();
        return hr;
    }

    HRESULT DescribeDevice(IMoniker *pMoniker)
    {
        IPropertyBag *pPropBag;
        HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
        if (FAILED(hr))
            return hr;

        hr = DescribeDeviceFromPropertyBag(pPropBag);
        if(SUCCEEDED(hr)){
            hr = DescribeDeviceFormats(pMoniker);
        }

        pPropBag->Release();
        return hr;
    }

    HRESULT DescribeDeviceFromPropertyBag(IPropertyBag *pPropBag)
    {
        VARIANT var;
        VariantInit(&var);

        // Not always available - but if it is should be more detailed than FriendlyName.
        // https://docs.microsoft.com/en-us/windows/win32/directshow/selecting-a-capture-device
        HRESULT hr = pPropBag->Read(L"Description", &var, 0);
        if (SUCCEEDED(hr))
        {
            proxy->AddDevice(toJavaString(var.bstrVal));
            VariantClear(&var);
            return hr;
        }

        hr = pPropBag->Read(L"FriendlyName", &var, 0);
        if (SUCCEEDED(hr))
        {
            proxy->AddDevice(toJavaString(var.bstrVal));
        }
        VariantClear(&var);
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
    }

    jstring toJavaString(BSTR bstr)
    {
        int len = SysStringLen(bstr);
        // special case because a NULL BSTR is a valid zero-length BSTR,
        // but regular string functions would balk on it
        if (len == 0) {
            return env->NewStringUTF("");
        }

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, bstr, len, NULL, 0, NULL, NULL);
        
        jstring rtn = 0;
        char* buffer = (char *)malloc(size_needed + 1);
        if(WideCharToMultiByte(CP_UTF8, 0, bstr, len, buffer, size_needed, NULL, NULL) > 0){
            buffer[size_needed] = 0;
            rtn = (env)->NewStringUTF(buffer);
        }

        free(buffer);
        return rtn;
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
};

JNIEXPORT jint JNICALL Java_com_github_regwhitton_videocaptureinventory_VideoCaptureInventoryWin64_populate
  (JNIEnv * env, jobject thisObject)
{
    JavaVideoCaptureInventoryProxy proxy(env, thisObject);
    VideoCaptureInventory vci(env, &proxy);
    return (jint) vci.Populate();
}
