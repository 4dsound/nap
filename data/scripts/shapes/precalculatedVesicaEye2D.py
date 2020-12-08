import nap
import math

def getPositions(resolution, hollow):
    positions = []

    # Rough approximation of the  particle count: has to be recounted at the end  with the list of position.
    #if hollow:
    #    particleRange = particleCount #particle range multiple of 6 for circle ?
    #else:
    #    particleRange = math.floor(math.sqrt(particleCount))

    particleRange = resolution

    if hollow:
        for i in range(particleRange + 1):
            fraction = i / particleRange
            # first circle below: radius 1, center (-1/2,0,0)
            x = - 1 / 2 + math.cos(2 * math.pi * fraction)
            y = 0
            z = math.sin(2 * math.pi * fraction)
            # now only keep this particle if within second circle: means distance to center (+1/2,0,0) is <=1. Actually, here, corresponds also to x >0
            if x >= 0:
                positions.append(nap.vec3(x, y, z))
            # second circle below: radius 1, center (1/2,0,0)
            x = 1 / 2 + math.cos(2 * math.pi * fraction)
            y = 0
            z = math.sin(2 * math.pi * fraction)
            # now only keep this particle if within first circle: means distance to center (-1/2,0,0) is <=1. Actually, here, corresponds also to the condition x < 0

            # CASI: 'dist' undefined. Produces an error. Changed condition to 'x < 0'.
            if x < 0:
                positions.append(nap.vec3(x, y, z))
                
        # Add two points at edges of the eye if not there.
        # A point at the edge of the EYE correspond to an angle Pi/3, hence a fraction = 1 /6 ie i = 3 j.  Would happens if particleRange is divisible by 3 (since only for external layer)
        if not (particleRange % 3 == 0):
            positions.append(nap.vec3(0, 0, math.sqrt(3) / 2))
            positions.append(nap.vec3(0, 0, -math.sqrt(3) / 2))

    else: #full shape
        for i in range(particleRange + 1): #correspond to successive i-nested circles, circles of radius i/particleRange
            for j in range(2 * i): #correspond to a point on such a circle
                fraction = j / (2 * i)
                radius = i / particleRange
                # First circle below: radius radius and center (-1/2,0,0)
                x = - 1 / 2 + radius * math.cos(2 * math.pi * fraction)
                y = 0
                z = radius * math.sin(2 * math.pi * fraction)
                # Now only keep this particle if within second circle AND on the good side (so EYE shape in the middle Vesica): boils down to the condition x>0
                if x > 0 :
                    positions.append(nap.vec3(x, y, z))
                # Second circle below: radius radius and center (+1/2,0,0)
                x = 1 / 2 + radius * math.cos(2 * math.pi * fraction)
                y = 0
                z = radius * math.sin(2 * math.pi * fraction)
                # now only keep this particle if within first circle AND on good side (so EYE shape in the middle Vesica): : boils down to the condition x<0
                if x < 0:
                    positions.append(nap.vec3(x, y, z))
        # Add two points at edges of the eye if not there.
        # A point at the edge of the EYE correspond to an angle Pi/3, hence a fraction = 1 /6 ie i = 3 j.  Would happens if particleRange is divisible by 3 (since only for external layer)
        if not (particleRange % 3 == 0):
            positions.append(nap.vec3(0, 0, math.sqrt(3) / 2))
            positions.append(nap.vec3(0, 0, -math.sqrt(3) / 2))

    return positions
