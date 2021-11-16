import nap
import math

maxParticleCount = 1024


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
    controlComponent.addInputEffect(environment.findResource("GranulatorEffect"))
    controlComponent.addInputEffect(environment.findResource("InputDistanceIntensityEffect"))
    controlComponent.addInputEffect(environment.findResource("InputDistanceDampingEffect"))
    controlComponent.addInputEffect(environment.findResource("SpatialDelayEffect"))
    controlComponent.addEffect(environment.findResource("DopplerEffect"))
    controlComponent.addEffect(environment.findResource("GainScalingEffect"))
    controlComponent.addEffect(environment.findResource("ReverbEffect"))
    controlComponent.addEffect(environment.findResource("DistanceIntensityEffect"))
    controlComponent.addEffect(environment.findResource("DistanceDampingEffect"))
    controlComponent.addEffect(environment.findResource("ElevationFilterUpEffect"))
    controlComponent.addEffect(environment.findResource("ElevationFilterDownEffect"))
    controlComponent.addEffect(environment.findResource("DistanceDiffusionEffect"))

    # add shape
    controlComponent.addShape(environment.findResource("TestLineShape"))

    # enable effects
    if (enableEffects == True):
        controlComponent.setParameterBool("effect/granulator/enable", True)
        controlComponent.setParameterBool("effect/inputDistanceAttenuation/enable", True)
        controlComponent.setParameterBool("effect/inputDistanceDamping/enable", True)
        controlComponent.setParameterBool("effect/spatialDelay/enable", True)
        controlComponent.setParameterBool("effect/doppler/enable", True)
        controlComponent.setParameterBool("effect/gainScaling/enable", True)
        controlComponent.setParameterBool("effect/reverb/enable", True)
        controlComponent.setParameterBool("effect/distanceAttenuation/enable", True)
        controlComponent.setParameterBool("effect/distanceDamping/enable", True)
        controlComponent.setParameterBool("effect/elevationFilterAbove/enable", True)
        controlComponent.setParameterBool("effect/elevationFilterBelow/enable", True)
        controlComponent.setParameterBool("effect/distanceDiffusionEffect/enable", True)

        controlComponent.setParameterFloat("effect/granulator/dryWet", 1)
        controlComponent.setParameterFloat("effect/spatialDelay/dryWet", 1)
        controlComponent.setParameterFloat("effect/reverb/dryWet", 1)

    return controlComponent


def init(entity):
    print("stresstest environment init")
    environment = entity.findComponent("nap::spatial::EnvironmentComponentInstance")
    environment.setCurrentState("starting")
    source = addTestLine(environment, True, "source")
    # space = addTestLine(environment, True, "space")
    # space.connectInput(source)
    environment.setCurrentState("running")
