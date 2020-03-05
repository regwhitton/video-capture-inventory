/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
package com.github.regwhitton.videocaptureinventory;

@SuppressWarnings("serial")
public class InventoryException extends Exception {
    InventoryException(String message) {
        super(message);
    }

    InventoryException(String message, Throwable t) {
        super(message, t);
    }
}
