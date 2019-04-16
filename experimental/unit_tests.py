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
    T_line = temp.Tline(x1,y1,z1,t1,a1,k_g,L1,num1,poly1)
    assert(T_line == math.inf)

def test_point_infiniteq():
    T_point = temp.Tpoint(x1,y1+L,t1,a1,k_g,num1,poly1)
    assert(T_point == math.inf)

def test_infline_infiniteq():
    T_infline = temp.Tinfline(x1+10,z1,t1,a1,k_g,100,poly1)
    assert(T_infline == math.inf)