import("stdfaust.lib");

fb1 = hslider("fb1", 0.5, 0, 1, 0.001) : si.smoo;
fb2 = hslider("fb2", 0.5, 0, 1, 0.001) : si.smoo;
damping = hslider("damping", 0.0, 0, 1, 0.001) : si.smoo;
spread = hslider("spread", 0.0, 0, 100, 0.001);
dryWet = hslider("dryWet", 1, 0, 1, 0.001) : si.smoo;

process = _ <: _ * (1 - dryWet), re.mono_freeverb(fb1, fb2, damping, spread) * dryWet * 0.2 :> _;
