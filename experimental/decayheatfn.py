import pandas as pd
from pyne.material import Material as m
from pyne import nucname
import numpy as np
from matplotlib import pyplot as plt
from scipy.optimize import curve_fit
from sympy import *
from scipy import special as s 
import scipy.integrate

def get_dataframe(dat):
    names = [
            'assembly_id',
            'reactor_id',
            'reactor_type',
            'initial_uranium_kg',
            'initial_enrichment',
            'discharge_burnup',
            'discharge_date',
            'discharge_time',
            'total_assembly_decay_heat_kw',
            'name',
            'evaluation_date',
            'total_mass_g',
            'total_radioactivity_curies']
    df = pd.read_csv(dat,
                    sep='\s+',
                    index_col=False,
                    names=names)
    return df


def decay_assembly(assembly_id,df): 
    """This function gives the decayheat of an assembly when
    given its assembly_id """
    new_m = initial_material_assembly(assembly_id,df)
    decayheat_total = decay_heat(new_m)
    return decayheat_total 

def initial_material_assembly(assembly_id,df): 
    """This function gives the initial pyne material of an 
    assembly in the dat file when given its assembly_id """
    
    assem = df.loc[df['assembly_id'] == assembly_id]
    compdict = {}
    for index, row in assem.iterrows():
        if row['assembly_id'] == assembly_id: 
            nuc = row['name']
            mass = row['total_mass_g']
            if nuc == 'ag-108m' or nuc == 'am-242m' or nuc == 'ag-110m': # meta stable nucs dont work?
                compdict = compdict 
            else:
                compdict[nucname.id(nuc)] = mass
    new_m = m(compdict,mass = -1)
    return new_m
    
def decay_heat(material): 
    """ This function gives the total decay heat of a 
    PyNE material
    """
    decayheat = material.decay_heat() 
    decayheat_total = 0
    for key,val in decayheat.items(): 
        decayheat_total += val*10**6 # returns decay heat total in Watts 
        
    return decayheat_total 

def get_q(assembly_id,years,df):
    """This function gives decay heat and time values 
    for an assembly for a specified number of years 
    INPUT: 
    assembly_id: int, assembly id 
    years: int, no. of years 
    OUTPUT: 
    Q: np array, decay heat [W]
    t: np array, time [s]
    """
    
    n = years # number of years 
    t = np.zeros(n*12) # time steps in months 
    Q = np.zeros(n*12)

    # get initial Pyne Material 
    new_m = initial_material_assembly(assembly_id,df)
    decayed_m = new_m 

    Q[0] = decay_heat(new_m)

    for i in range(1,n*12):
        decayed_m = decayed_m.decay(30*24*60*60)
        Q[i] = decay_heat(decayed_m)
        t[i] = i
    return t,Q

def q_poly(order,t,Q):
    """ This function gives the polynomial fit of
    the time and Q data 
    INPUT: 
    order: int, order of polynomial 
    t: np array, time values 
    Q: np array, Q values 
    OUTPUT: 
    Q_new: np array, Q values fitted to order polynomial 
    t_new: np.array, 1000 time steps  
    Q_coeffs: np array, coefficient values for poly fit 
    """
    Q_coeffs = np.polyfit(t, Q, order)
    f = np.poly1d(Q_coeffs)
    t_new = np.linspace(t[0], t[-1], 1000)
    Q_new = f(t_new)
    return t_new, Q_new, Q_coeffs 

def plot_q(t,Q,t_new,Q_new,order): 
    """ This function gives a plot compring the original 
    t,Q plot with the polynomial fit plot 
    INPUT: 
    order: int, order of polynomial 
    t: np array, time values 
    Q: np array, Q values      
    t_new = np.array, 1000 time steps  
    Q_new = np array, Q values fitted to order polynomial 
    """
    plt.plot(t,Q,label = 'original')
    plt.ylabel('Decay Heat (W)')
    plt.xlabel('Time (months)')
    plt.plot(t_new,Q_new,label = ('fit order',order))
    plt.legend()
    return 