/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
package com.github.regwhitton.videocaptureinventory;

/**
 * Represents the range of sizes that a device supports. Widths and heights
 * range in steps from the minimum to the maximum. Only used under Linux for
 * device such as the Raspberry Pi Camera.
 */
public class StepwiseFormat extends Format {
    private final int minWidth;
    private final int maxWidth;
    private final int stepWidth;
    private final int minHeight;
    private final int maxHeight;
    private final int stepHeight;

    StepwiseFormat(int minWidth, int maxWidth, int stepWidth, int minHeight, int maxHeight, int stepHeight) {
        this.minWidth = minWidth;
        this.maxWidth = maxWidth;
        this.stepWidth = stepWidth;
        this.minHeight = minHeight;
        this.maxHeight = maxHeight;
        this.stepHeight = stepHeight;
    }

    boolean sameSize(Format other) {
        if (!(other instanceof StepwiseFormat)) {
            return false;
        }
        StepwiseFormat f = (StepwiseFormat) other;
        return minWidth == f.minWidth && maxWidth == f.maxWidth && stepWidth == f.stepWidth &&
            minHeight == f.minHeight && maxHeight == f.maxHeight && stepHeight == f.stepHeight;
    }

    public int getMinWidth() {
        return minWidth;
    }

    public int getMaxWidth() {
        return maxWidth;
    }

    public int getStepWidth() {
        return stepWidth;
    }

    public int getMinHeight() {
        return minHeight;
    }

    public int getMaxHeight() {
        return maxHeight;
    }

    public int getStepHeight() {
        return stepHeight;
    }
}
