import("stdfaust.lib");

cutoffFrequency = hslider("cutoffFrequency", 20, 20, 20000, 0.001) : si.smoo;
process = fi.highpass(2, cutoffFrequency);
