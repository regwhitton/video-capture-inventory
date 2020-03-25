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
 * Example showing use of <a href=
 * "http://bytedeco.org/javacpp-presets/opencv/apidocs/index.html">OpenCV via
 * JavaCpp Presets</a>.
 * <p>
 * OpenCV always outputs a trivial warning to standard output on Windows. This
 * can't be disabled via the Java API, but can be turned off by setting the
 * environment variable OPENCV_VIDEOIO_PRIORITY_MSMF to any value.
 * </p>
 * 
 * @see http://bytedeco.org/javacpp-presets/opencv/apidocs/index.html
 */
public class OpenCvExample {

    private static final int[] IMWRITE_COMPRESSION_PARAMS = new int[] { IMWRITE_JPEG_QUALITY, 100 };

    public static void main(String[] args) throws InventoryException {
        takeSnaps(args[0]);
    }

    private static void takeSnaps(String directory) throws InventoryException {
        VideoCaptureInventory vci = VideoCaptureInventory.get();
        List<Device> devices = vci.getDevices();
        for (int d = 0; d < devices.size(); d++) {
            Device device = devices.get(d);
            System.out.println("DeviceId: " + device.getDeviceId() + ", Camera: " + device.getName());
            for (Format f : device.getFormats()) {
                if (f instanceof DiscreteFormat) {
                    DiscreteFormat df = (DiscreteFormat) f;

                    String imageFile = directory + "/" + device.getName() + "-" + df.getWidth() + "x" + df.getHeight()
                        + ".jpg";
                    takeSnap(device.getDeviceId(), df.getWidth(), df.getHeight(), imageFile);
                } else {
                    StepwiseFormat sf = (StepwiseFormat) f;

                    String minImageFile = directory + "/" + device.getName() + "-min-" + sf.getMinWidth() + "x" + sf
                        .getMinHeight() + ".jpg";
                    takeSnap(device.getDeviceId(), sf.getMinWidth(), sf.getMinHeight(), minImageFile);

                    String maxImageFile = directory + "/" + device.getName() + "-max-" + sf.getMaxWidth() + "x" + sf
                        .getMaxHeight() + ".jpg";
                    takeSnap(device.getDeviceId(), sf.getMaxWidth(), sf.getMaxHeight(), maxImageFile);
                }
            }
        }
    }

    private static void takeSnap(int deviceId, int width, int height, String imageFile) {
        VideoCapture videoCapture = new VideoCapture();
        try {
            videoCapture.set(CAP_PROP_FRAME_WIDTH, width);
            videoCapture.set(CAP_PROP_FRAME_HEIGHT, height);

            if (!videoCapture.open(deviceId)) {
                System.out.println("  could not open deviceId: " + deviceId);
                return;
            }

            try (Mat mat = new Mat()) {
                System.out.print("  " + imageFile + " ");

                videoCapture.read(mat);
                if (mat.empty()) {
                    System.out.println("- failed, blank frame grabbed");
                    return;
                }

                if (!imwrite(imageFile, mat, IMWRITE_COMPRESSION_PARAMS)) {
                    System.out.println("- failed to write frame");
                    return;
                }

                System.out.println("- done");
            }
        } finally {
            videoCapture.release();
            videoCapture.close();
        }
    }
}
