"""
This file contains unit tests
"""

import temperature as temp
import numpy as np
import math

# Dummy Variables 
# Granite 
k_g = 2.5 #W/m*K
a_g = 1.13*1e-6 #m2/s

# Length of Canister 
L = 5 #m 
# Canister Spacing 
CS = 10 #m
# Drift spacing 
DS = 20 #m 

x1 = 0.1
z1 = 0
a1 = a_g
t1 = 1
y1 = 2.5
L1 = 5
r1 = x1
num1 = 10 # no. of assemblies in canister

poly1 = np.poly1d(math.inf,variable='tp')


def test_line_infiniteq():
    """ Tests if the temperature of the line goes to infinity 
    when the q val goes to infinity
    """
    T_line = temp.Tline(x1,y1,z1,t1,a1,k_g,L1,num1,poly1)
    assert(T_line == math.inf)


def test_point_infiniteq():
    """ Tests if the temperature of the point goes to infinity 
    when the q val goes to infinity
    """
    T_point = temp.Tpoint(x1,y1+L,t1,a1,k_g,num1,poly1)
    assert(T_point == math.inf)


def test_infline_infiniteq():
    """ Tests if the temperature of the infinite line goes 
    to infinity when the q val goes to infinity
    """
    T_infline = temp.Tinfline(x1+10,z1,t1,a1,k_g,100,poly1)
    assert(T_infline == math.inf)


def test_line_x_decreasing_away():
    """ Tests if the line temperature of the medium decreases 
    as it moves away in the x-direction  from the location of 
    the heat source 
    """
    x = np.arange(0.5,10,0.5)
    n = len(x)
    T_line = np.zeros(n)
    for i in range(n):
        T_line[i] = temp.Tline(x[i],y1,z1,t1,a1,k_g,L1,num1,poly1)
    count = 0
    for i in range(1,n):
        if T_line[i]> T_line[i-1]:
            count += 1
    assert(count == 0)

def test_line_y_decreasing_away():
    """ Tests if the line temperature of the medium decreases 
    as it moves away in the y-direction  from the location of 
    the heat source 
    """
    y = np.arange(0.5,10,0.5)
    n = len(y)
    T_line = np.zeros(n)
    for i in range(n):
        T_line[i] = temp.Tline(x1,y[i],z1,t1,a1,k_g,L1,num1,poly1)
    count = 0
    for i in range(1,n):
        if T_line[i]> T_line[i-1]:
            count += 1
    assert(count == 0)

def test_line_z_decreasing_away():
    """ Tests if the line temperature of the medium decreases 
    as it moves away in the y-direction  from the location of 
    the heat source 
    """
    z = np.arange(0.5,10,0.5)
    n = len(z)
    T_line = np.zeros(n)
    for i in range(n):
        T_line[i] = temp.Tline(x1,y1,z[i],t1,a1,k_g,L1,num1,poly1)
    count = 0
    for i in range(1,n):
        if T_line[i]> T_line[i-1]:
            count += 1
    assert(count == 0)

def test_point_x_decreasing_away():
    """ Tests if the point temperature of the medium decreases 
    as it moves away in the x-direction from the location of 
    the heat source 
    """
    x = np.arange(0.5,10,0.5)
    n = len(x)
    T_point = np.zeros(n)
    for i in range(n):
        T_point[i] = temp.Tpoint(x[i],y1+L,t1,a1,k_g,num1,poly1)
    count = 0
    for i in range(1,n):
        if T_point[i]> T_point[i-1]:
            count += 1
    assert(count == 0)

def test_point_y_decreasing_away():
    """ Tests if the point temperature of the medium decreases 
    as it moves away in the y-direction from the location of 
    the heat source 
    """
    y = np.arange(0.5,10,0.5)
    n = len(y)
    T_point = np.zeros(n)
    for i in range(n):
        T_point[i] = temp.Tpoint(x1,y[i],t1,a1,k_g,num1,poly1)
    count = 0
    for i in range(1,n):
        if T_point[i]> T_point[i-1]:
            count += 1
    assert(count == 0)

def test_infline_x_decreasing_away():
    """ Tests if the infinite line temperature of the medium 
    decreases as it moves away in the x-direction  from the 
    location of the heat source 
    """
    x = np.arange(0.5,10,0.5)
    n = len(x)
    T_infline = np.zeros(n)
    for i in range(n):
        T_infline[i] = temp.Tinfline(x[i],z1,t1,a1,k_g,100,poly1)
    count = 0
    for i in range(1,n):
        if T_infline[i]> T_infline[i-1]:
            count += 1
    assert(count == 0)


def test_infline_z_decreasing_away():
    """ Tests if the infinite line temperature of the medium 
    decreases as it moves away in the z-direction  from the 
    location of the heat source 
    """
    z = np.arange(0.5,10,0.5)
    n = len(z)
    T_infline = np.zeros(n)
    for i in range(n):
        T_infline[i] = temp.Tinfline(x1,z[i],t1,a1,k_g,100,poly1)
    count = 0
    for i in range(1,n):
        if T_infline[i]> T_infline[i-1]:
            count += 1
    assert(count == 0)