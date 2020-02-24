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

    int Populate()
    {
        return DescribeDevices();
    }

    private:
    int DescribeDevices()
    {
        int fd;
        char video_file_path[20];

        for (int device = 0; ; device++) {
            snprintf(video_file_path, sizeof video_file_path, "/dev/video%d", device);
            if ((fd = open(video_file_path, O_RDONLY)) == -1) {
                return errno == ENOENT ? 0 : errno;
            }

            int ret = DescribeDevice(fd);

            close(fd);
            if (ret > 0) {
                return ret;
            }
        }
    }

    int DescribeDevice(int fd)
    {
        struct v4l2_capability  cap;

        if(ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
            return errno == ENOENT ? 0 : errno;
        }

        if ((cap.device_caps & V4L2_CAP_VIDEO_CAPTURE) == 0) {
            // Doesn't do video capture
            return 0;
        }

        struct v4l2_input inp;

        for (inp.index = 0; ; inp.index++) {
            if (ioctl(fd, VIDIOC_ENUMINPUT, &inp) == -1) {
                if (errno == EINVAL) {
                    continue;
                }
                return errno;
            }

            proxy->AddDevice(toJavaString(inp.name));

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
                    continue;
                }
                return errno;
            }

            struct v4l2_frmsizeenum frmsize;

            frmsize.pixel_format = fmt.pixelformat;
            for (frmsize.index = 0; ; frmsize.index++) {
                if(ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == -1) {
                    if (errno == EINVAL) {
                        continue;
                    }
                    return errno;
                }
                proxy->AddFormat(frmsize.discrete.width, frmsize.discrete.height);
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
