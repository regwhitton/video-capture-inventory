package com.github.regwhitton.videocaptureinventory;

// Turn off the warning about the failure to disable the serialization version checking.
@SuppressWarnings("serial")
public class InventoryException extends Exception {
    InventoryException(String message) {
        super(message);
    }

    InventoryException(String message, Throwable t) {
        super(message, t);
    }
}
