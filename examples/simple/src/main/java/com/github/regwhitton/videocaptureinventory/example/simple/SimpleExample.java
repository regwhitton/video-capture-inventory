/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
package com.github.regwhitton.videocaptureinventory.example.simple;

import java.io.*;

import com.github.regwhitton.videocaptureinventory.*;

public class SimpleExample {

    public static void main(String[] args) throws InventoryException {
        VideoCaptureInventory vci = VideoCaptureInventory.get();
        System.out.println("Number of cameras: " + vci.devices.size());
        for(Device d : vci.devices){
            System.out.println("Camera: " + d.name);
            for(Format f : d.formats){
                System.out.println("  " + f.width + " x " + f.height);
            }
        }
    }
}
