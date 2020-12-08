import("stdfaust.lib");

ratio = vslider("ratio", 4, 1, 20, 0.01);
thresh = vslider("threshold", -6, -90, 0, 0.1);
attack = vslider("attack", 0.0008, 0, 0.2, 0.0001);
release = vslider("release", 0.5, 0, 1, 0.0001);

process = co.compressor_mono(ratio, thresh, attack, release);
