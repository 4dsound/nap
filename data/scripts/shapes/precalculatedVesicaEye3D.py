import nap
import math

def getPositions(resolution, hollow):
    positions = []

    # Rough approximation of the  particle count: has to be recounted at the end  with the list of position.
    # if hollow:
    #     particleRange = math.floor(math.sqrt(particleCount))
    # else:
    #     particleRange = math.floor(math.pow(particleCount, 1/3))

    particleRange = resolution

    if hollow:
        for j in range(particleRange + 1):  # nb circle composing surface sphere
            for k in range(2 * particleRange):  # nb point on one circle
                fraction_j = j / particleRange
                fraction_k = k / (2 * particleRange)
                # first sphere below: radius 1, center (-1/2,0,0)
                x = - 1 / 2 + math.cos(2 * math.pi * fraction_j) * math.sin(2 * math.pi * fraction_k)
                y = math.cos(2 * math.pi * fraction_k)
                z = math.sin(2 * math.pi * fraction_j) * math.sin(2 * math.pi * fraction_k)
                # now only keep this particle if within second sphere: means distance to center (+1/2,0,0) is <=1. Actually, here, corresponds to x >0
                dist = math.sqrt((x - 1/2)*(x - 1/2) + y * y + (z - 0)*(z - 0))
                if dist <= 1:
                    positions.append(nap.vec3(x, y, z))
                # second sphere below: radius 1, center (+1/2,0,0)
                x = 1 / 2 + math.cos(2 * math.pi * fraction_j) * math.sin(2 * math.pi * fraction_k)
                y = math.cos(2 * math.pi * fraction_k)
                z = math.sin(2 * math.pi * fraction_j) * math.sin(2 * math.pi * fraction_k)
                # now only keep this particle if within first sphere: means distance to center (-1/2,0,0) is <=1. Actually, here, corresponds to x < 0
                dist = math.sqrt((x + 1 / 2) * (x + 1 / 2) + y * y + (z - 0) * (z - 0))
                if dist <= 1:
                    positions.append(nap.vec3(x, y, z))
        # Add 4 points at edges of the 3D VesicaEye if not there.
        #Points at the edge have as coordinates (0, 0, +- math.sqrt(3) / 2), or (0,  +- math.sqrt(3) / 2, 0) . Can check that they are only there if particleRange divisible by 6
        if not (particleRange % 6 == 0):
            positions.append(nap.vec3(0, 0, math.sqrt(3) / 2))
            positions.append(nap.vec3(0, 0, -math.sqrt(3) / 2))
            positions.append(nap.vec3(0, math.sqrt(3) / 2, 0))
            positions.append(nap.vec3(0, -math.sqrt(3) / 2, 0))


    else: #full shape
        for i in range(particleRange + 1):  # correspond to successive i-nested sphere, sphere of radius i/particleRange
            for j in range(i):  # nb circle
                for k in range(2 * i):  # nb point on one circle
                    fraction_j = j / particleRange
                    fraction_k = k / (2 * particleRange)
                    radius = i / particleRange
                    # first sphere below: radius i/particleRange, center (-1/2,0,0)
                    x = - 1 / 2 + radius * math.cos(2 * math.pi * fraction_j) * math.sin(2 * math.pi * fraction_k)
                    y = radius * math.cos(2 * math.pi * fraction_k)
                    z = radius * math.sin(2 * math.pi * fraction_j) * math.sin(2 * math.pi * fraction_k)
                    # IF only one set particles: now only keep this particle if within second circle AND on good side / middle Vesica: correspond to x>0
                    if x > 0:
                        positions.append(nap.vec3(x, y, z))
                    # second sphere below: radius i/particleRange, center (+1/2,0,0)
                    x = + 1 / 2 + radius * math.cos(2 * math.pi * fraction_j) * math.sin(2 * math.pi * fraction_k)
                    y = radius * math.cos(2 * math.pi * fraction_k)
                    z = radius * math.sin(2 * math.pi * fraction_j) * math.sin(2 * math.pi * fraction_k)
                    # now only keep this particle if within first circle AND on good side / middle Vesica: correspond to x<0
                    if x < 0:
                        positions.append(nap.vec3(x, y, z))
        # Add 4 points at edges of the 3D VesicaEye if not there.
        #Points at the edge have as coordinates (0, 0, +- math.sqrt(3) / 2), or (0,  +- math.sqrt(3) / 2, 0) . Can check that they are only there if particleRange divisible by 6
        if not (particleRange % 6 == 0):
            positions.append(nap.vec3(0, 0, math.sqrt(3) / 2))
            positions.append(nap.vec3(0, 0, -math.sqrt(3) / 2))
            positions.append(nap.vec3(0, math.sqrt(3) / 2, 0))
            positions.append(nap.vec3(0, -math.sqrt(3) / 2, 0))

    return positions
