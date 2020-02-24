package com.github.regwhitton.videocaptureinventory;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import cz.adamh.utils.NativeUtils;

public abstract class VideoCaptureInventory {

    private static final Class<?> impl = getImpl();

    public final List<Device> devices = new ArrayList<>();

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
     * Implemented by extending class to populated the instance.
     *
     * @return 0 on success, or a platform dependant code on failure.
     */
    protected abstract int populate();

    /**
     * Gives a message about the error code and the details on how to interprit it.
     *
     * @param code operating system dependant error code.
     *
     * @return the formatted message.
     */
    protected abstract String formatErrorCodeMessage(int code);

    /**
     * Used by extending class to add device to inventory.
     *
     * @param name of the device.
     */
    protected void addDevice(String name) {
        devices.add(new Device(name));
    }

    /**
     * Used by extending class to add a format to the last added device.
     *
     * @param width of the frame in this format.
     * @param height of the frame in this format.
     */
    protected void addFormat(int width, int height) {
        Format f = new Format(width, height);

        Device lastDevice = devices.get(devices.size() - 1);
        List<Format> formats = lastDevice.formats;
        if (!formats.contains(f)) {
            formats.add(f);
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
        }
        else if (name.startsWith("Linux")) {
            return forName(baseName + "Linux");
        }
        else {
            throw new UnsupportedOperationException("VideoCaptureInventory not implemented for " + name);
        }
    }

    private static Class<?> forName(String name) {
        try {
            return Class.forName(name);
        }
        catch(ClassNotFoundException ex) {
            throw new IllegalStateException(ex);
        }
    }

    private static VideoCaptureInventory newInstance() throws InventoryException {
        try {
            return (VideoCaptureInventory)impl.newInstance();
        }
        catch(InstantiationException|IllegalAccessException ex) {
            throw new InventoryException("Cannot create instance of "+ impl.getName(), ex);
        }
    }

    /**
     * Load the named library.  First tries the file system (java.library.path property,
     * PATH or LD_LIBRARY_PATH environment variables) in case the shared library has been
     * unpacked to a fixed place for us). If not found then the classpath is tried and
     * if found the library is unpacked into a temporary directory.
     *
     * @param name of the library (without the ".DLL" suffix on Windows,
     *             or the "lib" prefix and ".so" suffix on Linux).
     */
    protected static void loadLibrary(String name) {
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
