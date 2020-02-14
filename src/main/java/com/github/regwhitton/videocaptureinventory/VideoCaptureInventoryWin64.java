package com.github.regwhitton.videocaptureinventory;

class VideoCaptureInventoryWin64 extends VideoCaptureInventory {

    static {
        VideoCaptureInventory.loadLibrary("VideoCaptureInventoryWin64");
    }
     
    protected native int populate();

    protected String formatErrorCodeMessage(int code) {
        return String.format("Native DLL returned Windows error code while getting video capture devices: 0x%08X."
            + " Refer to https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/18d8fbe8-a967-4f1c-ae50-99ca8e491d2d",
            code);
        }
}
