package com.github.regwhitton.videocaptureinventory;

import java.util.ArrayList;
import java.util.List;

public class Device {
    public final String name;
    public final List<Format> formats;

    Device(String name) {
        this.name = name;
        formats = new ArrayList<>();
    }
}
