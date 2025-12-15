#!/bin/env python3
# (c) Trin Wasinger 2025
# Use this utility to generate firefly curves
# It creates c(0) and c(1) closed loop paths
# To save a file, run `./fireflies.py > data/world/fireflies.json`
from json import dumps as stringify
from math import pi, sin, cos, floor, ceil
from random import random as randf, seed as srand
srand(0xdeadbeef)

center=(32,5,32)
rmax=30
rmin=16
dy=2

curves=12

points = []
n=curves*3+1
for i in range(n):
    t = 2*pi*i/n
    points.append((
        (randf()*(rmax-rmin) + rmin)*cos(t) + center[0],
        dy*randf() - dy/2 + center[1],
        (randf()*(rmax-rmin) + rmin)*sin(t) + center[2]
    ))

# Ensure c(1)
for i in range(len(points)):
    if not i % 3 and i and i + 1 < len(points):
        points[i] = (
            points[i - 1][0] + (points[i+1][0] - points[i - 1][0])/2,
            points[i - 1][1] + (points[i+1][1] - points[i - 1][1])/2,
            points[i - 1][2] + (points[i+1][2] - points[i - 1][2])/2
        )
points[0] = points[-1] = (
    points[1][0] + (points[-2][0] - points[1][0])/2,
    points[1][1] + (points[-2][1] - points[1][1])/2,
    points[1][2] + (points[-2][2] - points[1][2])/2
)

print(stringify({'type': 'fireflies', 'speed': 0.5, 'points': points}, indent=' '*4))

if False:
    from matplotlib import pyplot as plt
    plt.xlim(0,64)
    plt.ylim(0,64)

    for i in range(len(points)):
        plt.scatter(points[i][0], points[i][2], color='tab:blue' if i%3 else 'tab:red')

    plt.plot([point[0] for point in points], [point[2] for point in points])

    import numpy
    t = numpy.linspace(0, 2*pi)
    plt.plot(rmin*numpy.cos(t) + center[0], rmin*numpy.sin(t) + center[2])
    plt.plot(rmax*numpy.cos(t) + center[0], rmax*numpy.sin(t) + center[2])

