#!/bin/python

import sqlite3

import numpy as np
from datetime import datetime
from astropy.time import Time

conn = sqlite3.connect('test.db')
print "Opened database successfully"
#Create new table
conn.execute('DROP TABLE IF EXISTS catalog')
conn.execute('''CREATE TABLE catalog(
hip INT PRIMARY KEY NOT NULL,
mag REAL NOT NULL,
magSource CHAR(1) NOT NULL,
rightAsc REAL NOT NULL,
decl REAL NOT NULL,
motRightAsc REAL NOT NULL,
motDecl REAL NOT NULL,
rightAscNow REAL,
declNow REAL);''')

cursor = conn.cursor()

J2000 = Time(2000.0, format='jyear', scale='tt')
J1991 = Time(1991.25,format='jyear', scale='tt')
JNow = Time(datetime.utcnow().isoformat(sep=' '), scale='utc', format='iso')

delta2000 = J2000-J1991
deltaNow = JNow - J1991
jday2jy= 1/365.25
marcs2arc = 1.0/(1000*3600)
deg2rad = np.pi/180

rawInput = np.loadtxt('hip_main.dat', delimiter='|', dtype=str)
tableData = []
for line in rawInput:
    try:
        hipID = int(line[1])
        mag   = float(line[5])
        magSource = line[7]
        rightAscension = float(line[8])
        declination = float(line[9])
        motionRa  = float(line[12])
        motionDec = float(line[13])
    except ValueError:
        continue
    # calculate position from today
    # motionRa in marc*s/yr -> arc*s/yr
    rightAscNow = rightAscension + motionRa * deltaNow.jd * jday2jy * marcs2arc /np.cos(deg2rad*declination) 
    decNow      = declination + motionDec * deltaNow.jd * jday2jy * marcs2arc
    # calculate position from j2000 parameters
    rightAscension = rightAscension + motionRa * delta2000.jd * jday2jy * marcs2arc/np.cos(deg2rad*declination) 
    declination    = declination + motionDec * delta2000.jd * jday2jy * marcs2arc
 
    # insert star into table
    tableData.append( (hipID, mag, magSource, rightAscension, declination, motionRa, motionDec, rightAscNow, decNow) )
    
    #print tableData


cursor.executemany("INSERT INTO catalog VALUES(?,?,?,?,?,?,?,?,?)", tableData)

conn.commit()
conn.close()