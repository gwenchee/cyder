""" This file contains the temperature functions 
"""
import pandas as pd
from pyne.material import Material as m
from pyne import nucname
import numpy as np
from matplotlib import pyplot as plt
from scipy.optimize import curve_fit
from sympy import *
from scipy import special as s 
import scipy.integrate

def Tline(x,y,z,t,a,k,L,num,poly):
    """ This function returns the contribution for Tline
    INPUT Parameters:
        x,y,z : coordinate of interest (0,0,0) refers to the midpoint of the canister
        t: time in the simulation in months [months]
        a: thermal diffusivity of repository material [m^2/s]
        L: length of canister [m]
        num: number of assemblies in the canister 
        poly: symbolic polynomial function for the q equation
        NOTE: when poly is updated for q equation of a canister, the num constant can be removed 
    returns 
    T = Tline contribution in K 
    """
    mtos = 60 * 60 * 24 * 30
    t =  t * mtos 
    Tfunc = lambda tp: (num*poly(tp/mtos)/L)/(t-tp)*exp(-(x**2+z**2)/(4*a*(t-tp)))*(erf(0.5*(y+L/2)/sqrt(a*(t-tp))) - erf(0.5*(y-L/2)/sqrt(a*(t-tp))))
    T = 1/(8*np.pi*k)*scipy.integrate.quad(Tfunc,0,t)[0] #K
    return T

def Tpoint(x,y,t,a,k,num,poly):
    mtos = 60 * 60 * 24 * 30
    t =  t * mtos 
    r = np.sqrt(x**2+y**2)
    Tfunc = lambda tp: (num*poly(tp/mtos)/(t-tp)**(3/2))*exp(-r**2/(4*a*(t-tp)))
    T = 1/(8*k*sqrt(a)*np.pi**(3/2))*scipy.integrate.quad(Tfunc,0,t)[0] #K
    return T

def Tinfline(x,z,t,a,k,num,poly):
    mtos = 60 * 60 * 24 * 30
    t =  t * mtos 
    Tfunc = lambda tp: num*poly(tp/mtos)/(t-tp)*exp(-(x**2+z**2)/(4*a*(t-tp)))
    T = 1/(4*np.pi*k)*scipy.integrate.quad(Tfunc,0,t)[0] #K
    return T