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

reverb = nap.ReverbEffect47()
reverb.Name = "reverb"

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
            controlComponent.addEffect(granulator)
            controlComponent.addEffect(spatialDelay)
            controlComponent.addEffect(doppler)
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
    print("environment init")

    environment = entity.findComponent("nap::spatial::EnvironmentComponentInstance")

    environment.setCurrentState("starting")
    createSources(environment, settings.SOURCES_COUNT)
    createSpaces(environment, settings.SPACES_COUNT)
    createGroups(environment, settings.GROUPS_COUNT)
    addFollowAndGroupTransformationsToAllSoundObjects()

    # comment in below code to automatically expose speaker amplitudes over osc
#    speakerSetupDataExposer = environment.findEntity("SpeakerSetupDataExposer")
#    exposedDataComponent = speakerSetupDataExposer.findComponent("nap::spatial::ExposedDataComponentInstance")
#    exposedDataComponent.addOSCOutput("speakerAmplitudes")

    # send the environment initialized OSC message
    oscInitMessage = nap.EnvironmentOSCMessage("/environment/init")
    oscInitMessage.addValue(settings.SOURCES_COUNT)
    oscInitMessage.addValue(settings.SPACES_COUNT)
    oscInitMessage.addValue(settings.GROUPS_COUNT)
    oscInitMessage.addValue(settings.SOURCES_MAX_PARTICLE_COUNT)
    oscInitMessage.addValue(settings.SPACES_MAX_PARTICLE_COUNT)
    environment.sendOSC(oscInitMessage)

    environment.setCurrentState("running")
