#!/bin/python

import sqlite3

import numpy as np
from  numpy.linalg import norm
# calculate carthesion coordinates from spherical coordinates
def sph2cart(az, elev, r):
    DEG_TO_RAD = np.pi / 180.0
    xyz = np.zeros( (len(az), 3) )
    xyz[:,2] = r * np.sin(elev*DEG_TO_RAD)
    rcoselev = r * np.cos(elev*DEG_TO_RAD)
    xyz[:,0] = rcoselev * np.cos(az*DEG_TO_RAD)
    xyz[:,1] = rcoselev * np.sin(az*DEG_TO_RAD)
    return xyz

MAX_MAGNITUDE = 3.5
FOV = 70.0

# Open hip-catalog
cat = sqlite3.connect('../hip-catalog/test.db')
catCursor = cat.cursor()

catCursor.execute('SELECT hip, rightAscNow, declNow FROM catalog WHERE mag<=?', (MAX_MAGNITUDE,) )
stars = np.array(catCursor.fetchall() )
xyz = sph2cart(stars[:,1], stars[:,2], 1)

data=[]
RAD_TO_DEG = 180.0 / np.pi
for i in  range(0,len(xyz)-1) :
    for j in range(i+1, len(xyz)-1):
        angle = np.arccos(np.dot(xyz[i], xyz[j])/(norm(xyz[i])*norm(xyz[j])) )*RAD_TO_DEG
        if angle <= FOV:
            data.append([int(stars[i,0]), int(stars[j,0]), angle])
    

featureList = sqlite3.connect('featureList2.db')
featCursor = featureList.cursor()

featCursor.execute('DROP TABLE IF EXISTS featureList')
featCursor.execute('''CREATE TABLE featureList(
                        hip1 INT NOT NULL,
                        hip2 INT NOT NULL,
                        theta REAL NOT NULL,
                        PRIMARY KEY(hip1,hip2) 
                                               )'''
                    )

featCursor.executemany("INSERT INTO featureList VALUES(?,?,?)", data)                    

featureList.commit()
featureList.close()
cat.close()
