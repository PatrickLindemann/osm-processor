import numpy as np

CENTER = 0
RADIUS = 1

def mercator(lon, lat):
    x = np.radians(lon) - np.radians(CENTER)
    y = np.log(np.tan(45 + np.radians(lat)/ 2))
    return (x, y)

SIZE = 100
points_lon = [np.random.uniform(-180.0, 180.0) for _ in range(SIZE)]
points_lat = [np.random.uniform(-90.0, 90.0) for _ in range(SIZE)]

points = [(points_lon[i], points_lat[i]) for i in range(SIZE)]
projected = [mercator(points_lon[i], points_lat[i]) for i in range(SIZE)]

print(mercator(-180.0, -90.0), mercator(180.0, 90.0))

for i in range(SIZE):
    pass
    # print(points[i], '->', projected[i])