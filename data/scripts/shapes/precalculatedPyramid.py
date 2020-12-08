import nap
import math

def getPositions(resolution, hollow):

    positions = []
    slicesCount = resolution
    scale = nap.vec3(0.5, 0.5 / math.sqrt(2), 0.5) # scale from domain [-1,1] to inside sphere with radius 1.

    # create top particle
    positions.append(scale * nap.vec3(0., 1., 0))

    # create square slices
    for i in range(1, slicesCount + 1):
        planeSize = i / slicesCount
        for j in range(i+1):
            for k in range(i+1):
                # only draw outer particles if hollow is true
                if not hollow or (i == slicesCount or j == 0 or j == i or k == 0 or k == i):
                    x = planeSize * (-1. + (j / float(i)) * 2.)
                    y = 1. - i / float(slicesCount) * 2.
                    z = planeSize * (-1. + (k / float(i)) * 2.)
                    positions.append(scale * nap.vec3(x, y, z))

    return positions

# ___getVisualizationVertices function___
# If you want your shape to be visualised, return its vertices here.
def getVisualizationVertices():
    return getPositions(1, False)

# # ___getVisualizaitonEdges function___
# # If you want your shape to be visualised, return the edges here as a list of pairs of indices.
def getVisualizationEdges():
    return [[0,1],[0,2],[0,3],[0,4],[1,2],[1,3],[2,4],[3,4]]
