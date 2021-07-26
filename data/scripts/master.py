import nap

class SatelliteMaster:
    def __init__(self, parameters, dsp):
        self.compressor = dsp.getObject("SatelliteCompressor").getChannel(0)
        parameters.connect("masterLimiterRatio", self.compressor.setRatio)
        parameters.connect("masterLimiterAttack", self.compressor.setAttack)
        parameters.connect("masterLimiterRelease", self.compressor.setRelease)
        parameters.connect("masterLimiterThreshold", self.compressor.setThreshold)

class SubMaster:
    def __init__(self, parameters, dsp):
        self.compressor = dsp.getObject("SubCompressor").getChannel(0)
        parameters.connect("masterLimiterRatio", self.compressor.setRatio)
        parameters.connect("masterLimiterAttack", self.compressor.setAttack)
        parameters.connect("masterLimiterRelease", self.compressor.setRelease)
        parameters.connect("masterLimiterThreshold", self.compressor.setThreshold)
        self.filter = dsp.getObject("SubFilter")
        parameters.connect("masterSubCutoff", self.filter.setFrequency)
