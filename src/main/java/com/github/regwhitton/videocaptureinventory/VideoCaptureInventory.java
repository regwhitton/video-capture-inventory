/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
package com.github.regwhitton.videocaptureinventory;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import cz.adamh.utils.NativeUtils;

/**
 * The source for inventories of the video capture devices connected to the
 * local machine. Returned by {@link #get()}.
 */
public abstract class VideoCaptureInventory {

    private static final Class<?> impl = getImpl();

    private final List<Device> devices = new ArrayList<>();

    /**
     * Gets an instance populated with Video Capture device information.
     *
     * @throws InventoryException upon failure to retrieve device information.
     *
     * @return the populated instance.
     */
    public static VideoCaptureInventory get() throws InventoryException {
        VideoCaptureInventory vci = newInstance();
        int result = vci.populate();
        if (result != 0) {
            throw new InventoryException(vci.formatErrorCodeMessage(result));
        }
        return vci;
    }

    /**
     * The list of attached {@link Device}s.
     * 
     * @return the list of attached {@link Device}s.
     */
    public List<Device> getDevices() {
        return Collections.unmodifiableList(devices);
    }

    /**
     * Implemented by extending class to populated the instance.
     *
     * @return 0 on success, or a platform dependent code on failure.
     */
    abstract int populate();

    /**
     * Gives a message about the error code and the details on how to interprit it.
     *
     * @param code operating system dependent error code.
     *
     * @return the formatted message.
     */
    abstract String formatErrorCodeMessage(int code);

    /**
     * Used by extending class to add a {@link Device} to the
     * {@link VideoCaptureInventory}.
     */
    void addDevice(int deviceId, String name) {
        devices.add(new Device(deviceId, name));
    }

    /**
     * Used by extending class to add a {@link DiscreteFormat} to the last added
     * {@link Device}.
     */
    void addFormat(int width, int height) {
        addFormat(new DiscreteFormat(width, height));
    }

    /**
     * Used by extending class to add a {@link StepwiseFormat} to the last added
     * {@link Device}.
     */
    void addFormat(int minWidth, int maxWidth, int stepWidth,
            int minHeight, int maxHeight, int stepHeight) {
        addFormat(new StepwiseFormat(minWidth, maxWidth, stepWidth, minHeight, maxHeight, stepHeight));
    }

    private void addFormat(Format format) {
        Device lastDevice = devices.get(devices.size() - 1);
        if (lastDevice.getFormats().stream().noneMatch(f -> f.sameSize(format))) {
            lastDevice.addFormat(format);
        }
    }

    /**
     * Determines the extending class for this environment.
     *
     * @return the extending class for use on this platform.
     */
    private static Class<?> getImpl() {
        String baseName = VideoCaptureInventory.class.getName();
        String name = System.getProperty("os.name");

        if (name.startsWith("Windows")) {
            return forName(baseName + "Win");
        } else if (name.startsWith("Linux")) {
            return forName(baseName + "Linux");
        } else {
            throw new UnsupportedOperationException("VideoCaptureInventory not implemented for " + name);
        }
    }

    private static Class<?> forName(String name) {
        try {
            return Class.forName(name);
        } catch (ClassNotFoundException ex) {
            throw new IllegalStateException(ex);
        }
    }

    private static VideoCaptureInventory newInstance() throws InventoryException {
        try {
            return (VideoCaptureInventory) impl.newInstance();
        } catch (InstantiationException | IllegalAccessException ex) {
            throw new InventoryException("Cannot create instance of " + impl.getName(), ex);
        }
    }

    /**
     * Load the named library. First tries the file system (java.library.path
     * property, PATH or LD_LIBRARY_PATH environment variables) in case the shared
     * library has been unpacked to a fixed place for us). If not found then the
     * classpath is tried and if found the library is unpacked into a temporary
     * directory.
     *
     * @param name of the library (without the ".DLL" suffix on Windows, or the
     *        "lib" prefix and ".so" suffix on Linux).
     */
    static void loadLibrary(String name) {
        try {
            System.loadLibrary(name);
        } catch (UnsatisfiedLinkError e) {
            try {
                NativeUtils.loadLibraryFromJar("/" + System.mapLibraryName(name));
            } catch (IOException ioe) {
                throw new RuntimeException(ioe);
            }
        }
    }
}
