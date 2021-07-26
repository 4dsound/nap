import nap
import math

import environmentSettings as settings

groupTransformations = []
particleLevelGroupTransformations = []
soundObjects = []
soundObjectNames = []

def createGroups(environment, count):
    for i in range(count):
        properties = nap.EnvironmentInstanceProperties()
        properties.addString("nap::ParameterComponent", "Name", "group" + str(i+1))
        groupObject = environment.createEntity("groupTransformationObject", properties)
        groupTransformations.append(groupObject.findComponentByID("shapeTransformationChain"))
        particleLevelGroupTransformations.append(groupObject.findComponentByID("particleTransformationChain"))

def createSimpleSources(environment, count):
    createSoundObjects(environment, count, False, True, 8)

def createSources(environment, count):
    createSoundObjects(environment, count, False, False, settings.SOURCES_MAX_PARTICLE_COUNT)

def createSpaces(environment, count):
    createSoundObjects(environment, count, True, False, settings.SPACES_MAX_PARTICLE_COUNT)


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

reverb47 = nap.ReverbEffect47()
reverb47.Name = "reverb"

reverb = nap.ReverbEffect()
reverb.Name = "reverb"
reverb.PredelayBufferSize = 32768
# reverb.LPFType = nap.FilterChainType.LowPass6dB
# reverb.HPFType = nap.FilterChainType.HighPass6dB
# Note: enum properties can't be overwritten from python. Fortunately, the default values (6dB filters) are desired here.

distanceIntensity = nap.DistanceIntensityEffect()
distanceIntensity.Name = "distanceAttenuation"

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
def createSoundObjects(environment, count, connect, simple, maxParticleCount):
    addedSoundObjects = []

    # create sources.
    for i in range(count):
        index = i + 1

        if connect:
            prefix = "space"
            uniqueID = i + 1
            category = 3
        else:
            if simple:
                uniqueID = settings.SPACES_COUNT + i + 1
                prefix = "simplesource"
                category = 1
            else:
                uniqueID = settings.SPACES_COUNT + settings.SIMPLE_SOURCES_COUNT + i + 1
                prefix = "source"
                category = 2


        name = prefix + str(index)

        # create sound objects
        properties = nap.EnvironmentInstanceProperties()
        properties.addString("nap::ParameterComponent", "Name", name)
        properties.addInt("nap::spatial::SpatialAudioComponent", "MaxParticleCount", maxParticleCount)
        properties.addString("nap::spatial::DisplaySettingsComponent", "DisplayName", name)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "DisplayIndex", index)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "UniqueId", uniqueID)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "Category", category)
        properties.addBool("nap::spatial::DisplaySettingsComponent", "Visible", True)

        soundObject = environment.createEntity("SoundObject", properties)
        controlComponent = soundObject.findComponent("nap::spatial::EnvironmentControlComponentInstance")

        # set hue
        controlComponent.setParameterFloat("hue", 0)

        # add effects
        if connect: # space sound objects
            controlComponent.addInputEffect(granulator)
            controlComponent.addInputEffect(inputDistanceIntensity)
            controlComponent.addInputEffect(inputDistanceDamping)
            controlComponent.addInputEffect(spatialDelay)
            controlComponent.addEffect(doppler)
            controlComponent.addEffect(spatialPhaser)
            controlComponent.addEffect(gainScaling)
            controlComponent.addEffect(reverb)
            controlComponent.addEffect(distanceIntensity)
            controlComponent.addEffect(distanceDamping)
            controlComponent.addEffect(elevationFilterUp)
            controlComponent.addEffect(elevationFilterDown)
            controlComponent.addEffect(distanceDiffusion)
        else: # source sound objects
            if not simple:
                controlComponent.addEffect(granulator)
                controlComponent.addEffect(spatialDelay)
            controlComponent.addEffect(doppler)
            if not simple:
                controlComponent.addEffect(gainScaling)
                controlComponent.addEffect(reverb)
            controlComponent.addEffect(distanceIntensity)
            controlComponent.addEffect(distanceDamping)
            controlComponent.addEffect(elevationFilterUp)
            controlComponent.addEffect(elevationFilterDown)
            controlComponent.addEffect(distanceDiffusion)

        # add external input
        controlComponent.addExternalInput()

        # set input channel
        controlComponent.setParameterBool("externalInputEnable", True)
        controlComponent.setParameterInt("externalInputStartChannel", index)

        # add granulator source
        # granulator = nap.GranulatorSource()
        # granulator.Name = "granulatorSource"
        # granulator.AudioFiles = [ "audio/bronsbow.wav", "audio/hang.wav" ]
        # soundObject.findComponent("nap::spatial::SpatialAudioComponentInstance").addSource(granulator)

        # add test source
        # soundObject.findComponent("nap::spatial::SpatialAudioComponentInstance").addTestSignal()

        # enable gainscaling by default
        if not simple:
            controlComponent.setParameterBool("effect/gainScaling/enable", True)

        # append to list
        soundObjects.append(soundObject)
        addedSoundObjects.append(soundObject)
        soundObjectNames.append(name)

    # connect every added soundobjects to all other soundobjects.
    if connect:
        for i in range(len(addedSoundObjects)):
            for j in range(len(soundObjects)):
                if addedSoundObjects[i] is not soundObjects[j]:
                    addedSoundObjects[i].findComponent("nap::spatial::EnvironmentControlComponentInstance").connectInput(soundObjects[j].findComponent("nap::spatial::EnvironmentControlComponentInstance"))


def createReverbs(environment, count, maxParticleCount):
    for i in range(count):
        index = i + 1
        name = "spatialverb" + str(index)
        uniqueID = settings.SIMPLE_SOURCES_COUNT + settings.SOURCES_COUNT + settings.SPACES_COUNT + i + 1

        # create reverb sound objects
        properties = nap.EnvironmentInstanceProperties()
        properties.addString("nap::ParameterComponent", "Name", name)
        properties.addInt("nap::spatial::SpatialAudioComponent", "MaxParticleCount", maxParticleCount)
        properties.addString("nap::spatial::DisplaySettingsComponent", "DisplayName", name)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "DisplayIndex", index)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "UniqueId", uniqueID)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "Category", 4)
        properties.addBool("nap::spatial::DisplaySettingsComponent", "Visible", False)

        soundObject = environment.createEntity("SoundObject", properties)
        controlComponent = soundObject.findComponent("nap::spatial::EnvironmentControlComponentInstance")

        # set hue
        controlComponent.setParameterFloat("hue", 0)
        controlComponent.setParameterOption("shapeType", "speakerSetup")

        # add effects
        controlComponent.addInputEffect(inputDistanceIntensity)
        controlComponent.addInputEffect(inputDistanceDamping)
        controlComponent.addInputEffect(spatialDelay)
        controlComponent.addEffect(reverb47)

        for j in range(len(soundObjects)):
            input = soundObjects[j].findComponent("nap::spatial::EnvironmentControlComponentInstance")
            controlComponent.connectInput(input)


def addFollowAndGroupTransformationsToAllSoundObjects():
    # add switchexternaltransformation for follow transformations and an externaltransformation for group transformations to all sound objects
    for i in range(len(soundObjects)):

        followTransformations = []
        followNames = []
        for j in range(len(soundObjects)):
            if i != j:
                followNames.append(soundObjectNames[j])
                followTransformations.append(soundObjects[j].findComponentByID("FollowTransformationChainComponent"))

        controlComponent = soundObjects[i].findComponent("nap::spatial::EnvironmentControlComponentInstance")
        controlComponent.addSwitchExternalTransformation("follow", followTransformations, followNames)

        for k in range(len(groupTransformations)):
            controlComponent.addExternalTransformation("group" + str(k+1), groupTransformations[k], True, True)

        shapeComponent = soundObjects[i].findComponentByID("ShapeComponent")

        for l in range(len(particleLevelGroupTransformations)):
            controlComponent.addExternalShapeTransformation("group" + str(l+1), particleLevelGroupTransformations[l], True, True)


def init(entity):
    environment = entity.findComponent("nap::spatial::EnvironmentComponentInstance")

    environment.setCurrentState("starting")
    createSimpleSources(environment, settings.SIMPLE_SOURCES_COUNT)
    createSources(environment, settings.SOURCES_COUNT)
    createSpaces(environment, settings.SPACES_COUNT)
    createReverbs(environment, settings.REVERB_COUNT, settings.REVERB_MAX_PARTICLE_COUNT)
    createGroups(environment, settings.GROUPS_COUNT)
    addFollowAndGroupTransformationsToAllSoundObjects()

    # send the environment initialized OSC message
    oscInitMessage = nap.EnvironmentOSCMessage("/environment/init")
    oscInitMessage.addValue(settings.SOURCES_COUNT)
    oscInitMessage.addValue(settings.SPACES_COUNT)
    oscInitMessage.addValue(settings.GROUPS_COUNT)
    oscInitMessage.addValue(settings.SOURCES_MAX_PARTICLE_COUNT)
    oscInitMessage.addValue(settings.SPACES_MAX_PARTICLE_COUNT)
    environment.sendOSC(oscInitMessage)

    environment.setCurrentState("running")
