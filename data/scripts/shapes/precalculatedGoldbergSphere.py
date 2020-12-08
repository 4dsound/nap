import nap
import math

#Say if a point p is inside triangle (-0.5,0),(0.5,0),(0,sqrt(3) / 2)
def insideT(p):
    isInside = False
    if (p[1] <= math.sqrt(3) * (p[0] + 0.5)) and (p[1] <= - math.sqrt(3) * (p[0] - 0.5)) and (p[1] >= 0):
        isInside = True
    return isInside

#Say is a point p is on the border of Triangle (-0.5,0),(0.5,0),(0,sqrt(3) / 2)
def isBorder(p):
    isOn = False
    if (abs(p[1] - math.sqrt(3) * (p[0] + 0.5)) <= 0.01) or (abs(p[1] + math.sqrt(3) * (p[0] - 0.5)) <= 0.01) or (abs(p[1]) <= 0.01):
        isOn = True
    return isOn


#To Map (points of) the fixed triangle to a spherical triangle using a double spherical linear interpolation

# barycentric coords for triangle (-0.5,0),(0.5,0),(0,sqrt(3)/2)
def barycentricCoords(p):
    x, y = p
    l3 = y * 2. / math.sqrt(3.)
    l2 = x + 0.5 * (1 - l3)     # 0.5*(l2 - l1) = x
    l1 = 1 - l2 - l3
    return l1, l2, l3

#Scalar product
def scalProd(p1, p2):
    return sum([p1[i] * p2[i] for i in range(len(p1))])

# uniform interpolation of arc defined by p0, p1 (around origin)
# t=0 -> p0, t=1 -> p1
#spherical linear interpolation
def slerp(p0, p1, t):
    assert abs(scalProd(p0, p0) - scalProd(p1, p1)) < 1e-7  # check same size vectors
    cosA = scalProd(p0, p1) / scalProd(p0, p0)  #  alpha is angle between 2 vectors
    sinA = math.sqrt(1 - cosA * cosA)
    alpha = math.atan2(sinA, cosA)
    l0 = math.sin((1 - t) * alpha)
    l1 = math.sin(t * alpha)
    return tuple([(l0 * p0[i] + l1 * p1[i]) / sinA for i in range(len(p0))])


# map 2D point p to spherical triangle s1,s2,s3 (3D vectors of equal length) . OK !
def mapGridpoint2Sphere(p, s1, s2, s3):
    l1, l2, l3 = barycentricCoords(p)
    if abs(l3 - 1) < 1e-10:
        return s3
    l2s = l2 / (l1 + l2)
    p12 = slerp(s1, s2, l2s) # certain amount of time along the vector s2-s1
    return slerp(p12, s3, l3)  # certain amount of time along the vector s3-p12 ...


# ---------


def getPositions(resolution, hollow):

    positions = []

    #  These are the vertices and triangles faces of the icosahedron
    #  It gives the finest regular division of the sphere surface into triangles (congruent equilateral triangles)
    # Will later map points in a hexagonal mesh in a fixed triangle to one of the spherical triangle from the icosahedron face using a double spherical linear interpolation

    c = 1 / math.sqrt(5)
    topPoints = [(0, 0, 1)] + [(2 * c * math.cos(i * 2 * math.pi / 5.),  2 * c * math.sin(i * 2 * math.pi / 5.), c) for i in range(5)]
    bottomPoints = [(-x, y, -z) for (x, y, z) in topPoints]
    icoPoints = topPoints + bottomPoints #the vertices
    #  Below the triangular faces given as indices
    icoTriangles = [(0, i + 1, (i + 1) % 5 + 1) for i in range(5)] + \
                 [(6, i + 7, (i + 1) % 5 + 7) for i in range(5)] + \
                 [(i + 1, (i + 1) % 5 + 1, (7 - i) % 5 + 7) for i in range(5)] + \
                 [(i + 1, (7 - i) % 5 + 7, (8 - i) % 5 + 7) for i in range(5)]

    ##Particle Count is here a rough approximation. Should be computed afterwards and updated accordingly. Also could be bounded so no bug
    particleRange = resolution
    nSphere = 1 + math.floor( particleRange / 3)  # So the number of spheres does not augment too fast. The proportion of nested spheres compared to the resolution could be slightly adjusted if not satisfying

    if hollow:
        nSphere = 1
    else:
        positions.append(nap.vec3(0, 0, 0))  # center point


    for m in range(1, nSphere + 1):  # the first one is the smaller, the last one the bigger
        res = m  # For now, this is the resolution of the m-sphere m, but could be adjusted. (3*( m-1) etc)
        # Though, distances should be more carefully checked, To ensure that points in the mesh of each spheres are at equal distance from each other (meaning the shorter distance), and between each sphere)
        if hollow:
            res = particleRange
        verticesIn = []  # To check doubles on border triangles. Reset.
        radius = m / nSphere  # radius current sphere, from >0 to 1

        # 2D Hexagonal MESH within a triangle.
        # For each hexagon of the mesh, only pin 2 vertices: upper and upper right vertices (since by the mesh, the others would be covered by the other hexagons). It corresponds to ie + v2 and +v1
        # The left down corner of the triangle is the center of the first hexagon.
        for j in range(res + 1):
            for k in range(res + 1):
                h_h = 1 / (2 * res)  # half height hexagon, depends on resolution mesh here.
                r_h = 2 * h_h / math.sqrt(3) # radius hexagon
                v1 = (r_h * math.sqrt(3) / 2, r_h / 2)
                v2 = (0, r_h)
                # pMesh : point 2D Mesh in triangle (-0.5,0),(0.5,0),(0,sqrt(3) / 2): Has to shift previous mesh in the y axis
                if (k % 2 == 0):
                    center_hexa = (- 0.5 + 2 * j * h_h, 3 * r_h * k / 2 )
                else:
                    center_hexa = (- 0.5 + (2 * j + 1) * h_h, 3 * r_h * k / 2)
                pMesh1 = (v1[0] + center_hexa[0], v1[1] + center_hexa[1])
                pMesh2 = (v2[0] + center_hexa[0], v2[1] + center_hexa[1])

                #check if such a point is within the triangle (-0.5,0),(0.5,0),(0,sqrt(3) / 2)
                if insideT(pMesh1):

                    for i in range(len(icoTriangles)):  # Map to each spherical triangle above
                        tri = icoTriangles[i]  # current triangle
                        # Corresponding point on sphere in this spherical triangle: pShere
                        pSphere = mapGridpoint2Sphere(pMesh1, icoPoints[tri[0]], icoPoints[tri[1]], icoPoints[tri[2]])

                        #  For edges of each triangles, has to check if not already in the list of points, check doubles.
                        if isBorder(pMesh1):
                            if not (pSphere in verticesIn):  # Python is okay for float check?
                                positions.append(nap.vec3(radius * pSphere[0], radius * pSphere[1], radius * pSphere[2]))
                                verticesIn.append(pSphere)
                        else:
                            positions.append(nap.vec3(radius * pSphere[0], radius * pSphere[1], radius * pSphere[2]))
                # Same for second point of the mesh pMesh2
                if insideT(pMesh2):
                    for i in range(len(icoTriangles)):  # map to each spherical triangle above
                        tri = icoTriangles[i]  # current triangle
                        # corresponding point on sphere in this spherical triangle: pShere
                        pSphere = mapGridpoint2Sphere(pMesh2, icoPoints[tri[0]], icoPoints[tri[1]], icoPoints[tri[2]])

                        #  For edges of each triangles, has to check if not already there.
                        if isBorder(pMesh2):
                            if not (pSphere in verticesIn):
                                positions.append(nap.vec3(radius * pSphere[0], radius * pSphere[1], radius * pSphere[2]))
                                verticesIn.append(pSphere)
                        else:
                            positions.append(nap.vec3(radius * pSphere[0], radius * pSphere[1], radius * pSphere[2]))

    for position in positions:
        position *= math.sqrt(3) / 2.

    return positions




# ___getVisualizationVertices function___
# If you want your shape to be visualised, return its vertices here.
def getVisualizationVertices():

    positions = []

    for j in range(21):
        for i in range(51):
            y = math.cos((i / 50.) * 2. * math.pi)
            t = math.sin((i / 50.) * 2. * math.pi)
            directionX = math.sin((j / 20.) * 2. * math.pi)
            directionZ = math.cos((j / 20.) * 2. * math.pi)
            x = directionX * t
            z = directionZ * t
            positions.append(0.5 * math.sqrt(3.) * nap.vec3(x,y,z))

    return positions
