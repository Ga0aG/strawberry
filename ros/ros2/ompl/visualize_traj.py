#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import re
import sys
import numpy as np
import matplotlib.pyplot as plt

ps = []
with open(sys.argv[1]) as file:
    for line in file.readlines():
        m = re.search("RealVectorState \[.*\]",line)
        if m:
            v = re.search("\[.*\]",m.group(0)).group(0)[1:-1]
            p = eval(v.replace(" ",","))
            ps.append(p)

# 给定点位
points = np.array(ps)


# 创建3D图形对象
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

r = 1
u = np.linspace(0, 2 * np.pi, 100)
v = np.linspace(0, np.pi, 100)
x = r * np.outer(np.cos(u), np.sin(v))
y = r * np.outer(np.sin(u), np.sin(v))
z = r * np.outer(np.ones(np.size(u)), np.cos(v))

# Plot the surface
ax.plot_surface(x, y, z, color='linen', alpha=0.5)

# plot circular curves over the surface
theta = np.linspace(0, 2 * np.pi, 100)
z = np.zeros(100)
x = r * np.sin(theta)
y = r * np.cos(theta)

ax.plot(x, y, z, color='black', alpha=0.75)
ax.plot(z, x, y, color='black', alpha=0.75)

## add axis lines
zeros = np.zeros(1000)
line = np.linspace(-10,10,1000)

# 绘制连接点的线条
for i in range(len(points)-1):
    ax.plot([points[i][0], points[i+1][0]], [points[i][1], points[i+1][1]], [points[i][2], points[i+1][2]], 'b')

# 绘制点位
ax.scatter(points[:, 0], points[:, 1], points[:, 2], c='r', marker='o')

# 设置坐标轴标签
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

# 显示图形
plt.show()