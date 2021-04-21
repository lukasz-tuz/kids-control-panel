# uint32_t RgbLed:: rectToRGB(float x, float y)
# {
#     auto cval = [](float theta, float ro, float phase) {
#         float val = sin(0.6 * theta - phase)
#         if (val < 0)
#         val = 0
#         return val
#     }
#     float theta = atan2(y, x) * RAD_TO_DEG
#     float ro = sqrt(x * x + y * y)

#     float r = cval(theta, ro, -PI / 2)
#     float g = cval(theta, ro, 0)
#     float b = cval(theta, ro, PI / 2)
# }

import math
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

def cval(theta, ro, phase, neg_phase=math.pi):
    val = math.sin(0.666 * theta - phase)
    if val < 0:
        val = math.sin(0.666 * theta - neg_phase)

    return max(val, 0)


def rect2rgb(theta, ro):
    # theta = math.degrees(math.atan2(y, x))
    # ro = math.sqrt(x*x + y*y)

    r=cval(theta, ro, -math.pi/2)
    g=cval(theta, ro, 0, 3*math.pi/2)
    b=cval(theta, ro, math.pi/2, 5*math.pi/2)

    return [r, g, b]

angles=[]
red=[]
green=[]
blue=[]

# for x in np.arange(-1, 1, 0.01):
#     for y in np.arange(-1, 1, 0.01):
for theta in np.arange(0, 4*math.pi, 0.01):
    r, g, b=rect2rgb(theta, 1)
    angles.append(theta)
    red.append(r)
    green.append(g)
    blue.append(b)

plt.plot(angles, red, 'r-', angles, green, 'g-', angles, blue, 'b-')
plt.savefig('graph.png')
