/*-
 * Copyright 2020 Reg Whitton
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * Video Capture Inventory aims to provide OpenCV users with a mechanism for
 * finding out which devices and frame sizes are available.
 * 
 * <pre>
 *  VideoCaptureInventory vci = VideoCaptureInventory.get();
 *  for (Device d : vci.getDevices()) {
 *      for (Format f : d.getFormats()) {
 *          if (f instanceof DiscreteFormat) {
 *              DiscreteFormat df = (DiscreteFormat) f;
 *              ...
 *          } else {
 *              StepwiseFormat sf = (StepwiseFormat) f;
 *              ...
 *          }
 *      }
 *  }
 * </pre>
 */
package com.github.regwhitton.videocaptureinventory;