import nap
import math

maxParticleCount = 1024

# define effects outside of the python function scope, so they remain alive (necessary if new effectprocessors are added later).
granulator = nap.GranulatorEffect()
granulator.Name = "granulator"
granulator.CircularBufferSize = 524288
granulator.CircularBufferChannelCount = 8
granulator.InternalBufferSize = 32
granulator.VoiceCount = 50

inputDistanceIntensity = nap.InputDistanceIntensityEffect()
inputDistanceIntensity.Name = "inputDistanceAttenuation"

inputDistanceDamping = nap.InputDistanceDampingEffect()
inputDistanceDamping.Name = "inputDistanceDamping"

spatialDelay = nap.SpatialDelayEffect()
spatialDelay.Name = "spatialDelay"

doppler = nap.DopplerEffect()
doppler.Name = "doppler"

spatialPhaser = nap.SpatialPhaserEffect()
spatialPhaser.Name = "spatialPhaser"
spatialPhaser.DelayLineSize = 16384
spatialPhaser.ParallelDelayCount = 8

gainScaling = nap.GainScalingEffect()
gainScaling.Name = "gainScaling"

reverb = nap.ReverbEffect()
reverb.Name = "reverb"
reverb.PredelayBufferSize = 32768
# reverb.LPFType = nap.FilterChainType.LowPass6dB
# reverb.HPFType = nap.FilterChainType.HighPass6dB
# Note: enum properties can't be overwritten from python. Fortunately, the default values (6dB filters) are desired here.

distanceIntensity = nap.DistanceIntensityEffect()
distanceIntensity.Name = "distanceIntensity"

distanceDamping = nap.DistanceDampingEffect()
distanceDamping.Name = "distanceDamping"

elevationFilterUp = nap.ElevationFilterUpEffect()
elevationFilterUp.Name = "elevationFilterAbove"

elevationFilterDown = nap.ElevationFilterDownEffect()
elevationFilterDown.Name = "elevationFilterBelow"

distanceDiffusion = nap.DistanceDiffusionEffect()
distanceDiffusion.Name = "distanceDiffusion"




def init(entity):
    print("stresstest environment init")

    environment = entity.findComponent("nap::spatial::EnvironmentComponentInstance")
    properties = nap.EnvironmentInstanceProperties()
    properties = nap.EnvironmentInstanceProperties()
    properties.addString("nap::ParameterComponent", "Name", "testline")
    properties.addInt("nap::spatial::SpatialAudioComponent", "MaxParticleCount", maxParticleCount)
    soundObject = environment.createEntity("SoundObject", properties)
    soundObject.findComponent("nap::spatial::SpatialAudioComponentInstance").addTestSignal()
    parameterComponent = soundObject.findComponent("nap::ParameterComponentInstance")
    parameterComponent.findParameter("shapeType").setOption("testline")
    parameterComponent.findParameter("receives/testSignal/enable").setValue(True)
    parameterComponent.findParameter("color").setValue(nap.vec3(1, 1, 1))
    
    
    # add effects
    spatialAudioComponent = soundObject.findComponent("nap::spatial::SpatialAudioComponentInstance")

    spatialAudioComponent.addInputEffect(granulator)
    spatialAudioComponent.addInputEffect(inputDistanceIntensity)
    spatialAudioComponent.addInputEffect(inputDistanceDamping)
    spatialAudioComponent.addInputEffect(spatialDelay)
    spatialAudioComponent.addEffect(doppler)
    spatialAudioComponent.addEffect(spatialPhaser)
    spatialAudioComponent.addEffect(gainScaling)
    spatialAudioComponent.addEffect(reverb)
    spatialAudioComponent.addPerceptionEffect(distanceIntensity)
    spatialAudioComponent.addPerceptionEffect(distanceDamping)
    spatialAudioComponent.addPerceptionEffect(elevationFilterUp)
    spatialAudioComponent.addPerceptionEffect(elevationFilterDown)
    spatialAudioComponent.addPerceptionEffect(distanceDiffusion)

