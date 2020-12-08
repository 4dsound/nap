import("stdfaust.lib");

cutoffFrequency = hslider("cutoffFrequency", 20000, 20, 20000, 0.001) : si.smoo;
process = fi.lowpass(2, cutoffFrequency);
