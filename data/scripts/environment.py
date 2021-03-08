import nap
import math

import environmentSettings as settings

groupTransformations = []
particleLevelGroupTransformations = []
soundObjects = []

def createGroups(environment, count):
    for i in range(count):
        properties = nap.EnvironmentInstanceProperties()
        properties.addString("nap::ParameterComponent", "Name", "group" + str(i+1))
        groupObject = environment.createEntity("groupTransformationObject", properties)
        groupTransformations.append(groupObject.findComponentByID("shapeTransformationChain"))
        particleLevelGroupTransformations.append(groupObject.findComponentByID("particleTransformationChain"))

def createSources(environment, count):
    createSoundObjects(environment, count, False, settings.SOURCES_MAX_PARTICLE_COUNT)

def createSpaces(environment, count):
    createSoundObjects(environment, count, True, settings.SPACES_MAX_PARTICLE_COUNT)


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




# count (int): the number of sound objects to create
# connect (bool): whether the newly created sound objects should be connected to all other sound objects
def createSoundObjects(environment, count, connect, maxParticleCount):

    addedSoundObjects = []

    # create sources.
    for i in range(count):

        # name sources 'source' and spaces 'space'
        if connect:
            prefix = "space"
        else:
            prefix = "source"

        index = i + 1

        if connect:
            uniqueID = i + 1
        else:
            uniqueID = settings.SPACES_COUNT + i + 1

        name = prefix + str(index)

        # create sound objects
        properties = nap.EnvironmentInstanceProperties()
        properties.addString("nap::ParameterComponent", "Name", name)
        properties.addInt("nap::spatial::SpatialAudioComponent", "MaxParticleCount", maxParticleCount)
        properties.addString("nap::spatial::DisplaySettingsComponent", "DisplayName", name)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "DisplayIndex", index)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "UniqueId", uniqueID)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "Category", 2 if connect else 1)

        soundObject = environment.createEntity("SoundObject", properties)

        # set hue
        soundObject.findComponent("nap::ParameterComponentInstance").findParameter("hue").setValue((len(soundObjects) / float(settings.SOURCES_COUNT + settings.SPACES_COUNT)) * 255)



        # add effects
        spatialAudioComponent = soundObject.findComponent("nap::spatial::SpatialAudioComponentInstance")

        if connect: # space sound objects
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
        else: # source sound objects (for now: just without spatial phaser)
            spatialAudioComponent.addInputEffect(granulator)
            spatialAudioComponent.addInputEffect(inputDistanceIntensity)
            spatialAudioComponent.addInputEffect(inputDistanceDamping)
            spatialAudioComponent.addInputEffect(spatialDelay)
            spatialAudioComponent.addEffect(doppler)
            spatialAudioComponent.addEffect(gainScaling)
            spatialAudioComponent.addEffect(reverb)
            spatialAudioComponent.addPerceptionEffect(distanceIntensity)
            spatialAudioComponent.addPerceptionEffect(distanceDamping)
            spatialAudioComponent.addPerceptionEffect(elevationFilterUp)
            spatialAudioComponent.addPerceptionEffect(elevationFilterDown)
            spatialAudioComponent.addPerceptionEffect(distanceDiffusion)



        # add external input
        spatialAudioComponent.addExternalInput()


        # set input channel
        parameterComponent = soundObject.findComponent("nap::ParameterComponentInstance")
        parameterComponent.findParameter("externalInputEnable").setValue(True)
        parameterComponent.findParameter("externalInputStartChannel").setValue(index)

        # add granulator source
        # granulator = nap.GranulatorSource()
        # granulator.Name = "granulatorSource"
        # granulator.AudioFiles = [ "audio/bronsbow.wav", "audio/hang.wav" ]
        # soundObject.findComponent("nap::spatial::SpatialAudioComponentInstance").addSource(granulator)

        # add test source
        # soundObject.findComponent("nap::spatial::SpatialAudioComponentInstance").addTestSignal()

        # enable gainscaling by default
        # soundObject.findComponent("nap::ParameterComponentInstance").findParameter("effect/gainScaling/enable").setValue(True)

        # append to list
        soundObjects.append(soundObject)
        addedSoundObjects.append(soundObject)

    # connect every added soundobjects to all other soundobjects.
    if connect:
        for i in range(len(addedSoundObjects)):
            for j in range(len(soundObjects)):
                if addedSoundObjects[i] is not soundObjects[j]:
                    addedSoundObjects[i].findComponent("nap::spatial::SpatializationComponentInstance").connectInput(soundObjects[j].findComponent("nap::spatial::SpatializationComponentInstance"), 0.0)


def addFollowAndGroupTransformationsToAllSoundObjects():
    # add switchexternaltransformation for follow transformations and an externaltransformation for group transformations to all sound objects
    for i in range(len(soundObjects)):

        followTransformations = []
        followNames = []
        for j in range(len(soundObjects)):
            if i != j:
                followNames.append("source" + str(j+1))
                followTransformations.append(soundObjects[j].findComponentByID("FollowTransformationChainComponent"))

        soundObjectTransformationChainComponent = soundObjects[i].findComponentByID("SoundObjectTransformationChainComponent")
        soundObjectTransformationChainComponent.addSwitchExternalTransformation("follow", followTransformations, followNames)

        for k in range(len(groupTransformations)):
            soundObjectTransformationChainComponent.addExternalTransformation("group" + str(k+1), groupTransformations[k], True, True)

        shapeComponent = soundObjects[i].findComponentByID("ShapeComponent")

        for l in range(len(particleLevelGroupTransformations)):
            shapeComponent.addExternalShapeTransformation("group" + str(l+1), particleLevelGroupTransformations[l], True, True)


def init(entity):
    print("environment init")

    environment = entity.findComponent("nap::spatial::EnvironmentComponentInstance")

    createSources(environment, settings.SOURCES_COUNT)
    createSpaces(environment, settings.SPACES_COUNT)
    createGroups(environment, settings.GROUPS_COUNT)
    addFollowAndGroupTransformationsToAllSoundObjects()

    # send the environment initialized OSC message
    oscInitMessage = nap.EnvironmentOSCMessage("/environment/init")
    oscInitMessage.addInt(settings.SOURCES_COUNT)
    oscInitMessage.addInt(settings.SPACES_COUNT)
    oscInitMessage.addInt(settings.GROUPS_COUNT)
    oscInitMessage.addInt(settings.SOURCES_MAX_PARTICLE_COUNT)
    oscInitMessage.addInt(settings.SPACES_MAX_PARTICLE_COUNT)
    environment.sendOSC(oscInitMessage)
