import nap
import math

# ___getPositions function___
# Returns a list of particle positions for the given 'resolution'. Parameters:
#
# Resolution (int): indicates the resolution of the shape. Higher resolution means more particles.
# Hollow (bool): indicates whether the shape is hollow or not. Can be freely interpreted (or ignored).
#
# During initialisation, getParticles() will get called with resolution increasing from 1 onwards
# until the maximum particle count has been reached, both for hollow=true and for hollow=false. The results are saved.
# This way, all possible particle options will be known at init-time, and don't have to be calculated in real-time.
#
# Returned particle positions are in "object space", meaning that they will still be scaled, rotated and positioned in world space.
def getPositions(resolution, hollow):

    # "positions" is a list of nap.vec3s.
    positions = []

    # This example shape generates a cross by placing particles on the corners of concentric squares.
    for i in range(resolution):
        length = (i + 1) / float(resolution)
        positions.append(nap.vec3(-length, 0, -length))
        positions.append(nap.vec3(-length, 0, length))
        positions.append(nap.vec3(length, 0, -length))
        positions.append(nap.vec3(length, 0, length))

    # Return the positions list.
    return positions

# ___getVisualizationVertices function___
# If you want your shape to be visualised, return its vertices here.
def getVisualizationVertices():
    return getPositions(1, False)

# ___getVisualizaitonEdges function___
# If you want your shape to be visualised, return the edges here as a list of pairs of indices.
def getVisualizationEdges():
    return [[0,3],[1,2]]
