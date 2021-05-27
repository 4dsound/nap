import nap
import math

maxParticleCount = 128

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


def addTestLine(environment, enableEffects, name):
    properties = nap.EnvironmentInstanceProperties()
    properties.addString("nap::ParameterComponent", "Name", name)
    properties.addInt("nap::spatial::SpatialAudioComponent", "MaxParticleCount", maxParticleCount)
    soundObject = environment.createEntity("SoundObject", properties)
    controlComponent = soundObject.findComponent("nap::spatial::EnvironmentControlComponentInstance")
    controlComponent.setParameterOption("shapeType", "testline")
    controlComponent.setParameterFloat("hue", 0.5)
    controlComponent.addTestSignal()
    controlComponent.setParameterBool("receives/testSignal/enable", True)

    # add effects
    controlComponent.addInputEffect(granulator)
    controlComponent.addInputEffect(inputDistanceIntensity)
    controlComponent.addInputEffect(inputDistanceDamping)
    controlComponent.addInputEffect(spatialDelay)
    controlComponent.addEffect(doppler)
    controlComponent.addEffect(spatialPhaser)
    controlComponent.addEffect(gainScaling)
    controlComponent.addEffect(reverb)
    controlComponent.addPerceptionEffect(distanceIntensity)
    controlComponent.addPerceptionEffect(distanceDamping)
    controlComponent.addPerceptionEffect(elevationFilterUp)
    controlComponent.addPerceptionEffect(elevationFilterDown)
    controlComponent.addPerceptionEffect(distanceDiffusion)

    # enable effects
    if (enableEffects == True):
        controlComponent.setParameterBool("effect/granulator/enable", True)
        controlComponent.setParameterBool("effect/inputDistanceAttenuation/enable", True)
        controlComponent.setParameterBool("effect/inputDistanceDamping/enable", True)
        controlComponent.setParameterBool("effect/spatialDelay/enable", True)
        controlComponent.setParameterBool("effect/doppler/enable", True)
        controlComponent.setParameterBool("effect/gainScaling/enable", True)
        controlComponent.setParameterBool("effect/reverb/enable", True)
        controlComponent.setParameterBool("effect/distanceIntensity/enable", True)
        controlComponent.setParameterBool("effect/distanceDamping/enable", True)
        controlComponent.setParameterBool("effect/elevationFilterAbove/enable", True)
        controlComponent.setParameterBool("effect/elevationFilterBelow/enable", True)
        controlComponent.setParameterBool("effect/distanceDiffusion/enable", True)

        controlComponent.setParameterFloat("effect/granulator/dryWet", 1)
        controlComponent.setParameterFloat("effect/spatialDelay/dryWet", 1)
        controlComponent.setParameterFloat("effect/reverb/dryWet", 1)

    return controlComponent


def init(entity):
    print("stresstest environment init")
    environment.setCurrentState("starting")
    environment = entity.findComponent("nap::spatial::EnvironmentComponentInstance")
    source = addTestLine(environment, True, "source")
    # space = addTestLine(environment, True, "space")
    # space.connectInput(source)
    environment.setCurrentState("running")
