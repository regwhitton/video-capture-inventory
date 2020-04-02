/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
package com.github.regwhitton.videocaptureinventory;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Represents a video capture device connected to the local machine.
 */
public class Device {
    private final int deviceId;
    private final String name;
    private final List<Format> formats;

    Device(int deviceId, String name) {
        this.deviceId = deviceId;
        this.name = name;
        formats = new ArrayList<>();
    }

    /**
     * The device id that should be passed to OpenCV's
     * {@code VideoCapture.open(deviceId)}. On Linux the ids are not necessarily
     * contiguous.
     * 
     * @return the device id that should be passed to OpenCV's
     *         {@code VideoCapture.open(deviceId)}
     */
    public int getDeviceId() {
        return deviceId;
    }

    /**
     * The name of the device.
     * 
     * @return the name of the device as reported by the operating system.
     */
    public String getName() {
        return name;
    }

    /**
     * The formats supported by this device.
     * 
     * @return formats supported by this device.
     */
    public List<Format> getFormats() {
        return Collections.unmodifiableList(formats);
    }

    void addFormat(Format format) {
        formats.add(format);
    }
}
