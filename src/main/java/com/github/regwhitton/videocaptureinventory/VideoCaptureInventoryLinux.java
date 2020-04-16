/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
package com.github.regwhitton.videocaptureinventory;

class VideoCaptureInventoryLinux extends VideoCaptureInventory {

    static {
        VideoCaptureInventory.loadLibrary("vidcapinv");
    }
     
    protected native int populate();

    protected String formatErrorCodeMessage(int code) {
        return String.format("Native shared library returned error code while getting video capture devices: %d : %s",
            code, errorMessage(code));
    }

    private native String errorMessage(int code);
}
