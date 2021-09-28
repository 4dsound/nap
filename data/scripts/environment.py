import nap
import environmentSettings as settings

# the transformation chains of group transformation objects.
groupTransformations = []
particleLevelGroupTransformations = []

# all sound objects and their names, for follow and group transformations
soundObjects = []
soundObjectNames = []

# unique id count
uniqueID = 0


def createSources(environment, count):
    inputEffects = []; effects = []; perceptionEffects = []; shapes = []; shapeTransformations = []; transformations = []

    effects.append("DopplerEffect")
    effects.append("DistanceIntensityEffect")
    effects.append("DistanceDampingEffect")
    effects.append("ElevationFilterUpEffect")
    effects.append("DistanceDiffusionEffect")

    shapes.append("MonoShape")
    shapes.append("StereoShape")
    shapes.append("RowShape")
    shapes.append("QuadShape")
    shapes.append("BasicCircleShape")
    shapes.append("BasicCubeShape")

    shapeTransformations.append("VibrateShapeTransformation")
    shapeTransformations.append("ShakeShapeTransformation")
    shapeTransformations.append("ParticleDimensionsShapeTransformation")
    shapeTransformations.append("ParticleScaleShapeTransformation")
    shapeTransformations.append("ParticleSpatialDynamicsShapeTransformation")

    transformations.append("InputPositionTransformation")
    transformations.append("InputPositionOffsetTransformation")
    transformations.append("InputDimensionsTransformation")
    transformations.append("InputScaleTransformation")
    transformations.append("PositionRotationTransformation")
    transformations.append("InvertTransformation")
    transformations.append("PathTransformation")
    transformations.append("ModulationTransformation")
    transformations.append("OrientationTransformation")
    transformations.append("SpatialDynamicsTransformation")

    objects = []

    for i in range(count):
        objects.append(createSoundObject(environment, "source", i + 1, 1, True, True, 8, inputEffects, effects, perceptionEffects, shapes, shapeTransformations, transformations))

    return objects


def createSoundEntities(environment, count, particleCount):
    inputEffects = []; effects = []; perceptionEffects = []; shapes = []; shapeTransformations = []; transformations = []

    inputEffects.append("GranulatorEffect")
    inputEffects.append("InputDistanceIntensityEffect")
    inputEffects.append("InputDistanceDampingEffect")
    inputEffects.append("SpatialDelayEffect")

    effects.append("DopplerEffect")
    effects.append("GainScalingEffect")
    effects.append("ReverbEffect")
    effects.append("DistanceIntensityEffect")
    effects.append("DistanceDampingEffect")
    effects.append("ElevationFilterUpEffect")
    effects.append("DistanceDiffusionEffect")

    shapes.append("MonoShape")
    shapes.append("StereoShape")
    shapes.append("RowShape")
    shapes.append("QuadShape")
    shapes.append("BasicCircleShape")
    shapes.append("BasicCubeShape")
    shapes.append("AdaptiveCubeShape")
    shapes.append("FibonacciSphereShape")
    shapes.append("SpeakerSetupShape")

    shapeTransformations.append("WaveShapeTransformation")
    shapeTransformations.append("VibrateShapeTransformation")
    shapeTransformations.append("BuzzShapeTransformation")
    shapeTransformations.append("ShakeShapeTransformation")
    shapeTransformations.append("AttractShapeTransformation")
    shapeTransformations.append("ParticleDimensionsShapeTransformation")
    shapeTransformations.append("ParticleScaleShapeTransformation")
    shapeTransformations.append("ParticleSpatialDynamicsShapeTransformation")

    transformations.append("InputPositionTransformation")
    transformations.append("InputPositionOffsetTransformation")
    transformations.append("InputDimensionsTransformation")
    transformations.append("InputScaleTransformation")
    transformations.append("PositionRotationTransformation")
    transformations.append("InvertTransformation")
    transformations.append("PathTransformation")
    transformations.append("ModulationTransformation")
    transformations.append("ShakeTransformation")
    transformations.append("SpasmTransformation")
    transformations.append("PlasmaTransformation")
    transformations.append("OrientationTransformation")
    transformations.append("SpatialDynamicsTransformation")

    objects = []

    for i in range(count):
        objects.append(createSoundObject(environment, "soundentity", i + 1, 2, True, True, particleCount, inputEffects, effects, perceptionEffects, shapes, shapeTransformations, transformations))

    return objects


def createSpaces(environment, count, particleCount):
    inputEffects = []; effects = []; perceptionEffects = []; shapes = []; shapeTransformations = []; transformations = []

    inputEffects.append("InputDistanceIntensityEffect")
    inputEffects.append("InputDistanceDampingEffect")
    inputEffects.append("SpatialDelayEffect")

    effects.append("ReverbEffect47")

    shapes.append("SpeakerSetupShape")

    objects = []

    for i in range(count):
        objects.append(createSoundObject(environment, "space", i + 1, 3, False, False, particleCount, inputEffects, effects, perceptionEffects, shapes, shapeTransformations, transformations))

    return objects


# Creates a sound object.
# prefix (string): the prefix of the sound objects
# index (int): the index of the sound objects
# category (int): the display category
# visible (bool): whether the sound object is visible by default
# externalInput (bool): whether the newly created sound object has external input
# maxParticleCount (int): the maximum particle count
# inputEffects (list of strings): the chain of input effects
# effects (list of strings): the chain of effects
# perceptionEffects (list of strings): the chain of perceptionEffects
# shapes (list of strings): the list of shapes
# shapeTransformations (list of resourceptrs): the list of shapeTransformations
# transformations (list of resourceptrs): the list of transformations
def createSoundObject(environment, prefix, index, category, visible, externalInput, maxParticleCount, inputEffects, effects, perceptionEffects, shapes, shapeTransformations, transformations):

    # Create sound objects.
    name = prefix + str(index)
    global uniqueID
    properties = nap.EnvironmentInstanceProperties()
    properties.addString("nap::ParameterComponent", "Name", name)
    properties.addInt("nap::spatial::SpatialAudioComponent", "MaxParticleCount", maxParticleCount)
    properties.addString("nap::spatial::DisplaySettingsComponent", "DisplayName", name)
    properties.addInt("nap::spatial::DisplaySettingsComponent", "DisplayIndex", index)
    properties.addInt("nap::spatial::DisplaySettingsComponent", "UniqueId", uniqueID)
    properties.addInt("nap::spatial::DisplaySettingsComponent", "Category", category)
    properties.addBool("nap::spatial::DisplaySettingsComponent", "Visible", visible)
    uniqueID = uniqueID + 1 # increment global unique ID counter

    # Create sound object.
    soundObject = environment.createEntity("SoundObject", properties)

    # Find the control component.
    controlComponent = soundObject.findComponent("nap::spatial::EnvironmentControlComponentInstance")

    # Add effects, shapes, shapetransformations and transforamtions.
    for inputEffect in inputEffects:
        controlComponent.addInputEffect(environment.findResource(inputEffect))

    for effect in effects:
        controlComponent.addEffect(environment.findResource(effect))

    for perceptionEffect in perceptionEffects:
        controlComponent.addPerceptionEffect(environment.findResource(perceptionEffect))

    for shape in shapes:
        controlComponent.addShape(environment.findResource(shape))

    for shapeTransformation in shapeTransformations:
        controlComponent.addShapeTransformation(environment.findResource(shapeTransformation))

    for transformation in transformations:
        controlComponent.addTransformation(environment.findResource(transformation))

    # Add external input and set default settings.
    if externalInput:
        controlComponent.addExternalInput()
        controlComponent.setParameterBool("externalInputEnable", True)
        controlComponent.setParameterInt("externalInputStartChannel", index)

    # Append to lists.
    soundObjects.append(soundObject)
    soundObjectNames.append(name)

    return soundObject


# Connects all objects in list a to all objects in list b, unless the objects are equal.
def connect(environment, a, b):
    for objectA in a:
        for objectB in b:
            if objectA is not objectB:
                objectB.findComponent("nap::spatial::EnvironmentControlComponentInstance").connectInput(objectA.findComponent("nap::spatial::EnvironmentControlComponentInstance"))


def createGroups(environment, count):
    for i in range(count):
        properties = nap.EnvironmentInstanceProperties()
        properties.addString("nap::ParameterComponent", "Name", "group" + str(i+1))
        groupObject = environment.createEntity("groupTransformationObject", properties)
        groupTransformations.append(groupObject.findComponentByID("shapeTransformationChain"))
        particleLevelGroupTransformations.append(groupObject.findComponentByID("particleTransformationChain"))


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


def sendInitMessage(environment):
    sendOSC(environment, "/from_4dengine/groupsCount", settings.GROUPS_COUNT)
    sendOSC(environment, "/from_4dengine/sourcesCount", settings.SOURCES_COUNT)
    sendOSC(environment, "/from_4dengine/soundentitiesCount", settings.SOUND_ENTITIES_COUNT)
    sendOSC(environment, "/from_4dengine/spacesCount", settings.SPACES_COUNT)
    sendOSC(environment, "/from_4dengine/soundentitiesMaxParticleCount", settings.SOUND_ENTITIES_MAX_PARTICLE_COUNT)
    sendOSC(environment, "/from_4dengine/spacesMaxParticleCount", settings.SPACES_MAX_PARTICLE_COUNT)


def init(entity):
    environment = entity.findComponent("nap::spatial::EnvironmentComponentInstance")

    environment.setCurrentState("starting")

    # create objects
    sources = createSources(environment, settings.SOURCES_COUNT)
    soundentities = createSoundEntities(environment, settings.SOUND_ENTITIES_COUNT, settings.SOUND_ENTITIES_MAX_PARTICLE_COUNT)
    spaces = createSpaces(environment, settings.SPACES_COUNT, settings.SPACES_MAX_PARTICLE_COUNT)

    # connect objects
    connect(environment, sources, soundentities)
    connect(environment, soundentities, soundentities)
    connect(environment, sources, spaces)
    connect(environment, soundentities, spaces)

    # add group and follow transformations
    createGroups(environment, settings.GROUPS_COUNT)
    addFollowAndGroupTransformationsToAllSoundObjects()

    sendInitMessage(environment)

    environment.setCurrentState("running")
