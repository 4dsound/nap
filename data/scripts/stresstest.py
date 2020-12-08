import nap
import math

maxParticleCount = 1024

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
