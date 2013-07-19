#!/bin/python

import sqlite3

import numpy as np

# Open hip-catalog
cat = sqlite3.connect('featureList2.db')
catCursor = cat.cursor()

# Get sorted list
catCursor.execute('SELECT * FROM featureList ORDER BY theta')

stars = catCursor.fetchall()

outFile = open("kVectorInput.txt", 'w')

# z(i) = m*x + q
# m = (ymax - ymin + 2*esp) / (n-1)
# q = ymin - m - eps

ymin = stars[0][2] # smallest angle in dataset
ymax = stars[-1][2] # biggest angle in dataset
eps = 8e-8

m = (ymax - ymin + 2* eps)/(len(stars)-1 )
q = ymin - m - eps

outFile.write(repr(q) + '\t' + repr(m) + '\n')
k = []
k.append(0)
j = 0
s = repr(j) + '\t' + repr(stars[0][0]) + '\t' + repr(stars[0][1]) + '\t' + repr(stars[0][2]) + '\n'
outFile.write(s)

for i in range(1, len(stars)):
    # calculate k
    # k(i) = j with s(j) <= z(i) < s(j+1)
    z = m * i + q
    while( (stars[j][2] < z) and (stars[j+1][2] <= z) ):
        j = j+1
    # format as k hip1 hip2 theta
    
    s = repr(j) + '\t' + repr(stars[i][0]) + '\t' + repr(stars[i][1]) + '\t' + repr(stars[i][2]) + '\n'
    outFile.write(s)