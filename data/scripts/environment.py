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
    createSoundObjects(environment, count, False, 8)

def createSoundEntities(environment, count):
    createSoundObjects(environment, count, True, settings.SOUND_ENTITIES_MAX_PARTICLE_COUNT)


# keep global unique id count
uniqueID = 0

# count (int): the number of sound objects to create
# connect (bool): whether the newly created sound objects should be connected to all other sound objects
def createSoundObjects(environment, count, connect, maxParticleCount):
    addedSoundObjects = []

    # create sources.
    for i in range(count):
        index = i + 1

        if connect:
            prefix = "soundentity"
            category = 2
        else:
            prefix = "source"
            category = 1

        name = prefix + str(index)

        # create sound objects
        global uniqueID
        properties = nap.EnvironmentInstanceProperties()
        properties.addString("nap::ParameterComponent", "Name", name)
        properties.addInt("nap::spatial::SpatialAudioComponent", "MaxParticleCount", maxParticleCount)
        properties.addString("nap::spatial::DisplaySettingsComponent", "DisplayName", name)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "DisplayIndex", index)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "UniqueId", uniqueID)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "Category", category)
        properties.addBool("nap::spatial::DisplaySettingsComponent", "Visible", True)
        uniqueID = uniqueID + 1 # increment global unique ID counter

        soundObject = environment.createEntity("SoundObject", properties)
        controlComponent = soundObject.findComponent("nap::spatial::EnvironmentControlComponentInstance")

        # set hue
        controlComponent.setParameterFloat("hue", 0)

        # add effects
        if connect: # sound entities
            controlComponent.addInputEffect(environment.findResource("GranulatorEffect"))
            controlComponent.addInputEffect(environment.findResource("InputDistanceIntensityEffect"))
            controlComponent.addInputEffect(environment.findResource("InputDistanceDampingEffect"))
            controlComponent.addInputEffect(environment.findResource("SpatialDelayEffect"))
            controlComponent.addEffect(environment.findResource("DopplerEffect"))
            controlComponent.addEffect(environment.findResource("SpatialPhaserEffect"))
            controlComponent.addEffect(environment.findResource("GainScalingEffect"))
            controlComponent.addEffect(environment.findResource("ReverbEffect"))
            controlComponent.addEffect(environment.findResource("DistanceIntensityEffect"))
            controlComponent.addEffect(environment.findResource("DistanceDampingEffect"))
            controlComponent.addEffect(environment.findResource("ElevationFilterUpEffect"))
            controlComponent.addEffect(environment.findResource("ElevationFilterDownEffect"))
            controlComponent.addEffect(environment.findResource("DistanceDiffusionEffect"))
        else: # sources
            controlComponent.addEffect(environment.findResource("DopplerEffect"))
            controlComponent.addEffect(environment.findResource("DistanceIntensityEffect"))
            controlComponent.addEffect(environment.findResource("DistanceDampingEffect"))
            controlComponent.addEffect(environment.findResource("ElevationFilterUpEffect"))
            controlComponent.addEffect(environment.findResource("ElevationFilterDownEffect"))
            controlComponent.addEffect(environment.findResource("DistanceDiffusionEffect"))

        # add shapes
        controlComponent.addShape(environment.findResource("PointShape"))
        controlComponent.addShape(environment.findResource("CircleShape"))
        controlComponent.addShape(environment.findResource("LineShape"))
        controlComponent.addShape(environment.findResource("TriangleShape"))
        controlComponent.addShape(environment.findResource("SquareShape"))
        controlComponent.addShape(environment.findResource("PentagonShape"))
        controlComponent.addShape(environment.findResource("HexagonShape"))
        controlComponent.addShape(environment.findResource("TetrahedronShape"))
        controlComponent.addShape(environment.findResource("OctahedronShape"))
        controlComponent.addShape(environment.findResource("CubeShape"))
        controlComponent.addShape(environment.findResource("IcosahedronShape"))
        controlComponent.addShape(environment.findResource("DodecahedronShape"))
        controlComponent.addShape(environment.findResource("CircleTorusShape"))
        controlComponent.addShape(environment.findResource("SpiralTorusShape"))
        controlComponent.addShape(environment.findResource("ArchimedeanSpiralShape"))
        controlComponent.addShape(environment.findResource("DoubleSpiralShape"))
        controlComponent.addShape(environment.findResource("TripleSpiralShape"))
        controlComponent.addShape(environment.findResource("InfiniteSpiralShape"))
        controlComponent.addShape(environment.findResource("HelixShape"))
        controlComponent.addShape(environment.findResource("GoldbergSphereShape"))
        controlComponent.addShape(environment.findResource("PyramidShape"))
        controlComponent.addShape(environment.findResource("WallsShape"))
        controlComponent.addShape(environment.findResource("StereoShape"))
        controlComponent.addShape(environment.findResource("MonoShape"))
        controlComponent.addShape(environment.findResource("SolidCubeShape"))
        controlComponent.addShape(environment.findResource("AdaptiveCubeShape"))
        controlComponent.addShape(environment.findResource("TestLineShape"))
        controlComponent.addShape(environment.findResource("FibonacciSphereShape"))
        controlComponent.addShape(environment.findResource("SpeakerSetupShape"))
        controlComponent.addShape(environment.findResource("QuadShape"))
        controlComponent.addShape(environment.findResource("BasicCubeShape"))
        controlComponent.addShape(environment.findResource("RowShape"))
        controlComponent.addShape(environment.findResource("BasicCircleShape"))

        # add shape transformations
        controlComponent.addShapeTransformation(environment.findResource("WaveShapeTransformation"))
        controlComponent.addShapeTransformation(environment.findResource("VibrateShapeTransformation"))
        controlComponent.addShapeTransformation(environment.findResource("BuzzShapeTransformation"))
        controlComponent.addShapeTransformation(environment.findResource("ShakeShapeTransformation"))
        controlComponent.addShapeTransformation(environment.findResource("AttractShapeTransformation"))
        controlComponent.addShapeTransformation(environment.findResource("ParticleDimensionsShapeTransformation"))
        controlComponent.addShapeTransformation(environment.findResource("ParticleScaleShapeTransformation"))
        controlComponent.addShapeTransformation(environment.findResource("ParticleSpatialDynamicsShapeTransformation"))

        # add transformations
        controlComponent.addTransformation(environment.findResource("InputPositionTransformation"))
        controlComponent.addTransformation(environment.findResource("InputPositionOffsetTransformation"))
        controlComponent.addTransformation(environment.findResource("InputDimensionsTransformation"))
        controlComponent.addTransformation(environment.findResource("InputScaleTransformation"))
        controlComponent.addTransformation(environment.findResource("PositionRotationTransformation"))
        controlComponent.addTransformation(environment.findResource("InvertTransformation"))
        controlComponent.addTransformation(environment.findResource("PathTransformation"))
        controlComponent.addTransformation(environment.findResource("ModulationTransformation"))
        controlComponent.addTransformation(environment.findResource("ShakeTransformation"))
        controlComponent.addTransformation(environment.findResource("SpasmTransformation"))
        controlComponent.addTransformation(environment.findResource("PlasmaTransformation"))
        controlComponent.addTransformation(environment.findResource("OrientationTransformation"))
        controlComponent.addTransformation(environment.findResource("SpatialDynamicsTransformation"))

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

        # enable gainscaling by default for spaces
        if connect:
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


def createSpaces(environment, count, maxParticleCount):
    for i in range(count):
        index = i + 1
        name = "space" + str(index)

        # create reverb sound objects
        global uniqueID
        properties = nap.EnvironmentInstanceProperties()
        properties.addString("nap::ParameterComponent", "Name", name)
        properties.addInt("nap::spatial::SpatialAudioComponent", "MaxParticleCount", maxParticleCount)
        properties.addString("nap::spatial::DisplaySettingsComponent", "DisplayName", name)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "DisplayIndex", index)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "UniqueId", uniqueID)
        properties.addInt("nap::spatial::DisplaySettingsComponent", "Category", 3)
        properties.addBool("nap::spatial::DisplaySettingsComponent", "Visible", False)
        uniqueID = uniqueID + 1 # increment global unique ID counter

        soundObject = environment.createEntity("SoundObject", properties)
        controlComponent = soundObject.findComponent("nap::spatial::EnvironmentControlComponentInstance")

        # set hue
        controlComponent.setParameterFloat("hue", 0)
        controlComponent.setParameterOption("shapeType", "speakerSetup")

        # add effects
        controlComponent.addInputEffect(environment.findResource("InputDistanceIntensityEffect"))
        controlComponent.addInputEffect(environment.findResource("InputDistanceDampingEffect"))
        controlComponent.addInputEffect(environment.findResource("SpatialDelayEffect"))
        controlComponent.addEffect(environment.findResource("ReverbEffect47"))

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

        for l in range(len(particleLevelGroupTransformations)):
            controlComponent.addExternalShapeTransformation("group" + str(l+1), particleLevelGroupTransformations[l], True, True)

def sendOSC(environment, tag, argument):
    oscInitMessage = nap.EnvironmentOSCMessage(tag)
    oscInitMessage.addValue(argument)
    environment.sendOSC(oscInitMessage)

def init(entity):
    environment = entity.findComponent("nap::spatial::EnvironmentComponentInstance")

    environment.setCurrentState("starting")
    createSources(environment, settings.SOURCES_COUNT)
    createSoundEntities(environment, settings.SOUND_ENTITIES_COUNT)
    createSpaces(environment, settings.SPACES_COUNT, settings.SPACES_MAX_PARTICLE_COUNT)
    createGroups(environment, settings.GROUPS_COUNT)
    addFollowAndGroupTransformationsToAllSoundObjects()

    # send the environment initialized OSC messages
    sendOSC(environment, "/from_4dengine/groupsCount", settings.GROUPS_COUNT)
    sendOSC(environment, "/from_4dengine/sourcesCount", settings.SOURCES_COUNT)
    sendOSC(environment, "/from_4dengine/soundentitiesCount", settings.SOUND_ENTITIES_COUNT)
    sendOSC(environment, "/from_4dengine/spacesCount", settings.SPACES_COUNT)
    sendOSC(environment, "/from_4dengine/soundentitiesMaxParticleCount", settings.SOUND_ENTITIES_MAX_PARTICLE_COUNT)
    sendOSC(environment, "/from_4dengine/spacesMaxParticleCount", settings.SPACES_MAX_PARTICLE_COUNT)

    environment.setCurrentState("running")
