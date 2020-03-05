/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
package com.github.regwhitton.videocaptureinventory;

public class Format {
    public final int width;
    public final int height;

    Format(int width, int height) {
        this.width = width;
        this.height = height;
    }

    public boolean equals(Object other) {
        Format f = (Format)other;
        return width == f.width && height == f.height;
    }

    public int hashCode() {
        int hash = 7;
        hash = 31 * hash + width;
        hash = 31 * hash + height;
        return hash;
    }
}
