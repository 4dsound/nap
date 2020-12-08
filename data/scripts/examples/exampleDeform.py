import nap

instances = []

class Deform:

    def __init__(self, entity, name, parameterManager):
        self.entity = entity
        self.name = name
        self.parameterManager = parameterManager

        # Add any necessary custom initalisation (like custom parameters) here.

        # custom parameter example:
        self.tilt = parameterManager.addParameterFloat("tilt", 1.0, -1.0, 1.0, False)
        self.amount = parameterManager.addParameterFloat("amount", 0.0, 0.0, 1.0, False)

    # ___apply function___
    # Should return a new list of particle transforms.
    # "particles" is a list of nap.SpatialTransforms.
    # SpatialTransforms have the following members:
    # .Position (vec3): the position of the particle in meters
    # .Scale (vec3): the dimensions of the particle in meters
    # .Rotation (vec4): the orientation of the particle, in angle-axis format.
    def apply(self, particles):

        deformedParticles = []

        for particle in particles:
            verticalOffset = particle.Position.x * self.tilt.Value * self.amount.Value
            position = nap.vec3(particle.Position.x , particle.Position.y + verticalOffset, particle.Position.z)
            deformedParticles.append(nap.SpatialTransform(position, particle.Scale, particle.Rotation))

        return deformedParticles

    # ___update function___
    # If you want to do certain actions based on passed time, you can implement this function.
    def update(self, deltaTime):
        pass
