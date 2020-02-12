package com.github.regwhitton.videocaptureinventory.example.opencv;

import static org.bytedeco.opencv.global.opencv_imgcodecs.imwrite;
import static org.bytedeco.opencv.global.opencv_videoio.CAP_PROP_FRAME_HEIGHT;
import static org.bytedeco.opencv.global.opencv_videoio.CAP_PROP_FRAME_WIDTH;

import java.io.File;

import org.bytedeco.opencv.opencv_core.Mat;
import org.bytedeco.opencv.opencv_videoio.VideoCapture;
import com.github.regwhitton.videocaptureinventory.*;

public class OpenCvExample {

    public static void main(String[] args) throws InventoryException {
        takeSnaps(args[0]);
    }

    private static void takeSnaps(String directory) throws InventoryException {
        VideoCaptureInventory vci = VideoCaptureInventory.get();
        for(int d = 0; d < vci.devices.size(); d++){
            Device device = vci.devices.get(d);
            for(Format f : device.formats) {
                String imageFile = directory + "/" + device.name + "-" + f.width + "x" + f.height + ".jpg";
                takeSnap(d, f.width, f.height, imageFile);
            }
        }
    }

    private static void takeSnap(int camera, int width, int height, String imageFile) {
        try (VideoCapture videoCapture = new VideoCapture()) {
            if (videoCapture.open(camera)) {
                try (Mat mat = new Mat()) {
                    videoCapture.set(CAP_PROP_FRAME_WIDTH, width);
                    videoCapture.set(CAP_PROP_FRAME_HEIGHT, height);

                    videoCapture.read(mat);

                    imwrite(imageFile, mat);
                }
            }
        }
    }
}
