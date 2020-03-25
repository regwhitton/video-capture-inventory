/*
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include "com_github_regwhitton_videocaptureinventory_VideoCaptureInventoryLinux.h"

/**
 * Used by JNI implementations to call methods in the Java VideoCaptureInventory class.
 */
class JavaVideoCaptureInventoryProxy
{
    private:
    JNIEnv *env;
    jobject target;
    jmethodID addDeviceMethodId;
    jmethodID addDiscreteFormatMethodId;
    jmethodID addStepwiseFormatMethodId;

    public:
    JavaVideoCaptureInventoryProxy(JNIEnv *env, jobject javaVideoCaptureInventory)
    {
        this->env = env;
        this->target = javaVideoCaptureInventory;

        jclass targetClass = env->GetObjectClass(target);
        this->addDeviceMethodId = env->GetMethodID(targetClass, "addDevice", "(ILjava/lang/String;)V");
        this->addDiscreteFormatMethodId = env->GetMethodID(targetClass, "addFormat", "(II)V");
        this->addStepwiseFormatMethodId = env->GetMethodID(targetClass, "addFormat", "(IIIIII)V");
    }

    void AddDevice(jint deviceId, jstring name)
    {
        env->CallObjectMethod(this->target, this->addDeviceMethodId, deviceId, name);
    }

    void AddFormat(jint width, jint height)
    {
        env->CallObjectMethod(this->target, this->addDiscreteFormatMethodId, width, height);
    }

    void AddFormat(jint minWidth, jint maxWidth, jint stepWidth, jint minHeight, jint maxHeight, jint stepHeight)
    {
        env->CallObjectMethod(this->target, this->addStepwiseFormatMethodId,
             minWidth, maxWidth, stepWidth, minHeight, maxHeight,  stepHeight);
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

    int Populate()
    {
        return DescribeDevices();
    }

    private:
    int DescribeDevices()
    {
        char video_file_path[20];

        for (int deviceId = 0; deviceId < 64 ; deviceId++) {
            snprintf(video_file_path, sizeof video_file_path, "/dev/video%d", deviceId);
            int ret = DescribeDevice(deviceId, video_file_path);
            if (ret > 0) {
                return ret;
            }
        }
        return 0;
    }

    int DescribeDevice(int deviceId, const char* video_file_path)
    {
        int fd;

        if ((fd = open(video_file_path, O_RDONLY)) == -1) {
            return errno == ENOENT ? 0 : errno;
        }

        int ret = DescribeDevice(deviceId, fd);

        close(fd);
        return ret;
    }

    int DescribeDevice(int deviceId, int fd)
    {
        struct v4l2_capability  cap;

        if(ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
            return errno;
        }

        if ((cap.device_caps & V4L2_CAP_VIDEO_CAPTURE) == 0) {
            // Doesn't do video capture
            return 0;
        }

        struct v4l2_input inp;

        for (inp.index = 0; ; inp.index++) {
            if (ioctl(fd, VIDIOC_ENUMINPUT, &inp) == -1) {
                if (errno == EINVAL) {
                    break;
                }
                return errno;
            }

            proxy->AddDevice(deviceId, toJavaString(inp.name));

            int ret = DescribeFormats(fd);
            if (ret > 0) {
                return ret;
            }
        }
        return 0;
    }

    int DescribeFormats(int fd)
    {
        struct v4l2_fmtdesc fmt;

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        for (fmt.index = 0; ; fmt.index++) {

            if(ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == -1) {
                if (errno == EINVAL) {
                    break;
                }
                return errno;
            }

            struct v4l2_frmsizeenum frmsize;

            frmsize.pixel_format = fmt.pixelformat;
            for (frmsize.index = 0; ; frmsize.index++) {
                if(ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == -1) {
                    if (errno == EINVAL) {
                        break;
                    }
                    return errno;
                }

                if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                    proxy->AddFormat(frmsize.discrete.width, frmsize.discrete.height);
                }
                else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
                    proxy->AddFormat(
                        frmsize.stepwise.min_width, frmsize.stepwise.max_width, frmsize.stepwise.step_width,
                        frmsize.stepwise.min_height, frmsize.stepwise.max_height, frmsize.stepwise.step_height);
                }
                else {
                    proxy->AddFormat(
                        frmsize.stepwise.min_width, frmsize.stepwise.max_width, 1,
                        frmsize.stepwise.min_height, frmsize.stepwise.max_height, 1);
                }
            }
        }
        return 0;
    }

    jstring toJavaString(__u8* str)
    {
        return (env)->NewStringUTF((char*)str);
    }
};

JNIEXPORT jint JNICALL Java_com_github_regwhitton_videocaptureinventory_VideoCaptureInventoryLinux_populate
  (JNIEnv * env, jobject thisObject)
{
    JavaVideoCaptureInventoryProxy proxy(env, thisObject);
    VideoCaptureInventory vci(env, &proxy);
    return (jint) vci.Populate();
}

JNIEXPORT jstring JNICALL Java_com_github_regwhitton_videocaptureinventory_VideoCaptureInventoryLinux_errorMessage
  (JNIEnv * env, jobject thisObject, jint code)
{
    return (env)->NewStringUTF(strerror(code));
}
