/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
package com.github.regwhitton.videocaptureinventory;

/**
 * Represents a frame size that the device supports.
 */
public class DiscreteFormat extends Format {
    private final int width;
    private final int height;

    DiscreteFormat(int width, int height) {
        this.width = width;
        this.height = height;
    }

    boolean sameSize(Format other) {
        if (!(other instanceof DiscreteFormat)) {
            return false;
        }
        DiscreteFormat f = (DiscreteFormat) other;
        return width == f.width && height == f.height;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }
}
