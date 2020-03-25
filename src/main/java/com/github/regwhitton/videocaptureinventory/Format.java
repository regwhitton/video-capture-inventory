/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
package com.github.regwhitton.videocaptureinventory;

/**
 * Represents a format supported by a device. See the subclasses for details.
 */
public abstract class Format {

    abstract boolean sameSize(Format other);
}
