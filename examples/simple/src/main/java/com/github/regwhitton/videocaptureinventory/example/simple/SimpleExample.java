/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
package com.github.regwhitton.videocaptureinventory.example.simple;

import com.github.regwhitton.videocaptureinventory.Device;
import com.github.regwhitton.videocaptureinventory.DiscreteFormat;
import com.github.regwhitton.videocaptureinventory.Format;
import com.github.regwhitton.videocaptureinventory.InventoryException;
import com.github.regwhitton.videocaptureinventory.StepwiseFormat;
import com.github.regwhitton.videocaptureinventory.VideoCaptureInventory;

public class SimpleExample {

    public static void main(String[] args) throws InventoryException {
        VideoCaptureInventory vci = VideoCaptureInventory.get();
        System.out.println("Number of cameras: " + vci.getDevices().size());
        for (Device d : vci.getDevices()) {
            System.out.println("DeviceId: " + d.getDeviceId() + ", Camera: " + d.getName());
            for (Format f : d.getFormats()) {
                if (f instanceof DiscreteFormat) {
                    DiscreteFormat df = (DiscreteFormat) f;
                    System.out.println("  " + df.getWidth() + " x " + df.getHeight());
                } else {
                    StepwiseFormat sf = (StepwiseFormat) f;
                    System.out.println("  " + sf.getMinWidth() + "->" + sf.getMaxWidth() + " step " + sf.getStepWidth() +
                        "  X  " + sf.getMinHeight() + "->" + sf.getMaxHeight() + " step " + sf.getStepHeight());
                }
            }
        }
    }
}
