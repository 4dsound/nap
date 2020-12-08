import nap

class Shape:

    def __init__(self, entity, name, parameterManager):
        self.entity = entity
        self.name = name
        self.parameterManager = parameterManager


    # ___getPositions function___
    def getParticles(self, particleCount, speedAdjustedTime):
        particles = []

        if particleCount == 1:
            particles.append(nap.SpatialTransform(nap.vec3(0., 0., 0.), nap.vec3(0.25, 0.25, 0.25), nap.vec4(0,0,1,0)))
            return particles

        countMinusLast = particleCount - 1

        # prevent divide by 0
        if countMinusLast <= 0:
            countMinusLast = 1

        spacing = 2. / countMinusLast

        for i in range(countMinusLast):
            particles.append(nap.SpatialTransform(nap.vec3(-1. + spacing * i, 0., 0.), nap.vec3(0.25, 0.25, 0.25), nap.vec4(0,0,1,0)))

        # add last particle
        particles.append(nap.SpatialTransform(nap.vec3(1., 0., 0.), nap.vec3(0.25, 0.25, 0.25), nap.vec4(0,0,1,0)))
        
        return particles


    # ___update function___
    # If you want to do certain actions based on the 'real-life' passed time instead of the speedAdjustedTime, you can implement this function.
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
