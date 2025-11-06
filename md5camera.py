#!/bin/env python3
# (c) Trin Wasinger 2025
# Use this utility to generate MD5Camera "movie" tracks
# Edit the second to last line to add instructions (spin around point, move on line, hold position)
# To save a file, run `./md5camera.py > your_track.txt`

from typing import Tuple, List
from math import pi, cos, sin, sqrt

Point = Tuple[float, float, float]

def normalize(point: Point) -> Point:
    return tuple(y/sqrt(sum(x**2 for x in point)) for y in point)

# Spin in a circle and observe an origin from an optional y offset
def spin(origin: Point, *, r: float = 10.0, dy: float = 0.0, frames: int = 0, fov: float = 45.0) -> List[Tuple[Point, Point, Point, float]]:
  frames = frames or round(2*pi*r)*8
  return [(
      (origin[0] + r*cos(n/frames*2*pi), origin[1] + dy, origin[2] + r*sin(n/frames*2*pi)),
      normalize((-r*cos(n/frames*2*pi), -dy, -r*sin(n/frames*2*pi))),
      (0.0, 1.0, 0.0),
      fov
  ) for n in range(frames)]

# Move along a line while looking at a direction
def line(start: Point, end: Point, camDir: Point, frames: int = 0, fov: float = 45.0) -> List[Tuple[Point, Point, Point, float]]:
  frames = frames or round(sqrt(sum((a-b)**2 for a,b in zip(start, end)))*8)
  return [(
      (start[0] + n/frames*(end[0] - start[0]), start[1] + n/frames*(end[1] - start[1]), start[2] + n/frames*(end[2] - start[2])),
      normalize(camDir),
      (0.0, 1.0, 0.0),
      fov
  ) for n in range(frames)]

# Repeats a config unchanged for several frames
def hold(eyePos: Point, camDir: Point, upVec: Point, frames: int, fov: float = 45.0) -> List[Tuple[Point, Point, Point, float]]:
    return [(eyePos, normalize(camDir), normalize(upVec), fov)]*frames

# Combine components into a md5camera movie
def composite(components: List[List[Tuple[Point, Point, Point, float]] | Tuple[Point, Point, Point, float]]):
    components = [y for x in components for y in (x if type(x) == list else [x])]
    return f'{len(components)}\n' + '\n'.join(' '.join(str(round(x,6)) for x in (*eyePos, *camDir, *upVec, fov)) for eyePos, camDir, upVec, fov in components)

print(composite([
    # Edit here
    hold((31,1.75,14), (1,-0.1,1), (0,1,0), 500), spin((32, 5, 32), r=7, dy=3), hold((20.5, 15, 20.5), (0, -1, 0), (1, 0, 0), 100), line((-10, 7, -10), (50, 7, 50), (1,-0.5,1)), hold((16, 5, 16), (-1, -0.25, -1), (0, 1, 0), 250)
]))