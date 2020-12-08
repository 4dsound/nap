import nap
import math

class Shape:

    # ___init function___
    # Parameters can be initialised here.
    def __init__(self, entity, name, parameterManager):
        # store entity, name and parameter manager in class
        self.entity = entity
        self.name = name
        self.parameterManager = parameterManager

        # Create any custom parameter here, using the parameterManager.
        # Parameters are automatically added to the UI & OSC interfaces.
        # Documentation of the interface of the ParameterManager class is included at the end of this file.
        self.wobbliness = self.parameterManager.addParameterFloat("wobbliness", 0., 0., 1., False) # amount of modulation of particle positions
        self.bubbliness = self.parameterManager.addParameterFloat("bubbliness", 0., 0., 1., False) # amount of modulation of particle dimensions


    # ___getParticles function___
    # Should return a list of transforms (position, dimensions & rotation) for the given resolution option. Parameters:
    #
    # particleCount (int): indicates the desired number of particles. The amount of transforms returned should be around this number.
    # speedAdjustedTime (float): position in time in "seconds". The passing of speedAdjustedTime is influenced by the built-in shape speed parameter.
    #
    # Returned particle transforms are in "object space", meaning that they will still be scaled, rotated and positioned in world space.
    def getParticles(self, particleCount, speedAdjustedTime):

        # "particles" is a list of nap.SpatialTransform that will be returned at the end of this function
        # SpatialTransforms have the following members:
        # .Position (vec3): the position of the particle in meters
        # .Scale (vec3): the dimensions of the particle in meters
        # .Rotation (vec4): the orientation of the particle, in angle-axis format.
        particles = []

        # This example shape generates a line with modulation of position & dimensions based on custom parameters.
        for i in range(particleCount):
            position = nap.vec3(-1 + 2 * (i / float(max(1, particleCount - 1))), 0, 0)
            positionOffset = self.wobbliness.Value * nap.vec3(0, math.cos(i + speedAdjustedTime * 3.0), math.sin(i + 0.5 + speedAdjustedTime * 3.0))
            dimensions = nap.vec3(0.25, 0.25, 0.25)
            dimensionsMultiplier = 1.0 + 3.0 * self.bubbliness.Value * ((0.9 * i + speedAdjustedTime) % 1.0)
            rotation = nap.vec4(0., 0., 1., 0.)
            particles.append(nap.SpatialTransform(position + positionOffset, dimensions * dimensionsMultiplier, rotation))

        return particles

    # ___update function___
    # If you want to do certain actions based on the 'real-life' passed time instead of the speedAdjustedTime (for example listen to sensor data),
    # you can implement this function. The update function is always called, even when the shape is not active.
    def update(self, deltaTime):
        pass

    # ___getVisualizationVertices function___
    # If you want your shape to be visualised, return its vertices here.
    def getVisualizationVertices(self):
        return [nap.vec3(-1,0,0), nap.vec3(1,0,0)]

    # # ___getVisualizationEdges function___
    # # If you want your shape to be visualised, return the edges here as a list of pairs of indices.
    def getVisualizationEdges(self):
        return [[0,1]]


# ##############
# Interface of ParameterManager class from C++. All below functions are exposed to python.
#
# /**
#  * Adds a float parameter.
#  * If 'shared' is true, it creates a parameter that is shared with other shapes. (for example, all of the 'theworks' shapes have a 'hollow' parameter.
#  */
# ParameterFloat* addParameterFloat(const std::string& name, float defaultValue, float min, float max, bool shared = false);
#
# /**
#  * Adds an int parameter.
#  */
# ParameterInt* addParameterInt(const std::string& name, int defaultValue, int min, int max, bool shared = false);
#
# /**
#  * Adds a vec3 parameter.
#  */
# ParameterVec3* addParameterVec3(const std::string& name, const glm::vec3& defaultValue, float min, float max, bool shared = false);
#
# /**
#  * Adds a bool parameter.
#  */
# ParameterBool* addParameterBool(const std::string& name, bool defaultValue, bool shared = false);
#
# /**
#  * Adds a string parameter.
#  */
# ParameterString* addParameterString(const std::string& name, const std::string& defaultValue, bool shared = false);
#
# /**
#  * Adds an option list parameter.
#  */
# ParameterOptionList* addParameterOptionList(const std::string& name, const std::string& defaultValue, const std::vector<std::string>& options, bool shared = false);
#
# ##############
