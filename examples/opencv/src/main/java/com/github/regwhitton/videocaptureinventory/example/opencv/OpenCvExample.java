/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
package com.github.regwhitton.videocaptureinventory.example.opencv;

import static org.bytedeco.opencv.global.opencv_imgcodecs.IMWRITE_JPEG_QUALITY;
import static org.bytedeco.opencv.global.opencv_imgcodecs.imwrite;
import static org.bytedeco.opencv.global.opencv_videoio.CAP_PROP_FRAME_HEIGHT;
import static org.bytedeco.opencv.global.opencv_videoio.CAP_PROP_FRAME_WIDTH;

import java.util.List;

import org.bytedeco.opencv.opencv_core.Mat;
import org.bytedeco.opencv.opencv_videoio.VideoCapture;

import com.github.regwhitton.videocaptureinventory.Device;
import com.github.regwhitton.videocaptureinventory.DiscreteFormat;
import com.github.regwhitton.videocaptureinventory.Format;
import com.github.regwhitton.videocaptureinventory.InventoryException;
import com.github.regwhitton.videocaptureinventory.StepwiseFormat;
import com.github.regwhitton.videocaptureinventory.VideoCaptureInventory;

/**
 * Example that captures a frame from each camera at each frame size. On Linux
 * stepwise devices just the smallest and largest frame sizes are used.
 * <p>
 * Shows the use of video-capture-inventory with OpenCV VideoCapture
 * <a href= "http://bytedeco.org/javacpp-presets/opencv/apidocs/index.html">via
 * JavaCpp Presets</a>.
 * </p>
 * <h2>Windows</h2> OpenCV always outputs a trivial warning to standard output
 * on Windows. This can't be disabled via the Java API, but can be turned off by
 * setting the environment variable OPENCV_VIDEOIO_PRIORITY_MSMF to any value.
 * 
 * <h2>Raspberry Pi/Linux</h2> Since the Raspbian-2019-07-10 release, the Linux
 * USB camera driver is often left is a bad state on exit. Possibly a result
 * switching camera and/or frame size quickly multiple times. See <a href=
 * "https://raspberrypi.stackexchange.com/questions/105358/raspberry-pi4-error-while-using-2-usb-cameras-vidioc-qbuf-invalid-argument">
 * Stack Exchange post</a>. Re-using the same VideoCapture instance seems to
 * help. Once in bad state, errors are reported about the pixel format or device
 * being busy. Normality might restored by:
 * <ul>
 * <li>rebooting</li>
 * <li>unplugging and replugging in USB camera</li>
 * <li>retrying (don't why, sometimes just goes away)</li>
 * <li>using: {@code sudo rmmod uvcvideo} and
 * {@code sudo modprobe uvcvideo}</li>
 * </ul>
 */
public class OpenCvExample implements AutoCloseable {

    public static void main(String[] args) throws InventoryException {
        String targetDirectory = args[0];

        try (OpenCvExample oce = new OpenCvExample(targetDirectory)) {
            oce.takeSnaps();
        }
    }

    private static final int[] IMWRITE_COMPRESSION_PARAMS = new int[] { IMWRITE_JPEG_QUALITY, 100 };

    private final VideoCapture videoCapture;
    private final String directory;

    private OpenCvExample(String directory) {
        videoCapture = new VideoCapture();
        this.directory = directory;
    }

    public void close() {
        videoCapture.release();
        videoCapture.close();
    }

    private void takeSnaps() throws InventoryException {

        VideoCaptureInventory vci = VideoCaptureInventory.get();

        List<Device> devices = vci.getDevices();
        for (int d = 0; d < devices.size(); d++) {
            Device dev = devices.get(d);
            System.out.println("DeviceId: " + dev.getDeviceId() + ", Camera: " + dev.getName());

            for (Format f : dev.getFormats()) {
                if (f instanceof DiscreteFormat) {
                    DiscreteFormat df = (DiscreteFormat) f;
                    takeSnap(dev.getDeviceId(), dev.getName(), df.getWidth(), df.getHeight());
                } else {
                    StepwiseFormat sf = (StepwiseFormat) f;
                    takeSnap(dev.getDeviceId(), dev.getName(), sf.getMinWidth(), sf.getMinHeight());
                    takeSnap(dev.getDeviceId(), dev.getName(), sf.getMaxWidth(), sf.getMaxHeight());
                }
            }
        }
    }

    private void takeSnap(int deviceId, String deviceName, int width, int height) {

        String imageFile = imageFilename(deviceName, width, height);

        if (!videoCapture.open(deviceId)) {
            System.out.println("  could not open deviceId: " + deviceId);
            return;
        }

        videoCapture.set(CAP_PROP_FRAME_WIDTH, width);
        videoCapture.set(CAP_PROP_FRAME_HEIGHT, height);

        try (Mat mat = new Mat()) {
            System.out.print("  " + imageFile + " ");

            if (!videoCapture.read(mat)) {
                System.out.println("- read failed");
                return;
            }

            if (!imwrite(imageFile, mat, IMWRITE_COMPRESSION_PARAMS)) {
                System.out.println("- failed to write frame");
                return;
            }

            System.out.println("- done");
        }
    }

    private String imageFilename(String deviceName, int width, int height) {
        return directory + "/" + deviceName + "-" + width + "x" + height + ".jpg";
    }
}
