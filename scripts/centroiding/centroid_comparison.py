#!/bin/python

# import list
from numpy import \
        array, sqrt
        
import matplotlib.pyplot as pl
        
from matplotlib.pyplot import \
        plot, show, figure, hist, legend, xlabel, ylabel

fileNameMatlab = 'centroidMatlab.txt'
fileNameStarCam = 'new-test.txt'
#fileNameCC = 'ccStarCam.txt'
fileNameCC = 'cc-new.txt'

# margin (in pixel) to determine similar points
epsX = 0.7
epsY = 0.7

# import data from Matlab
fMatlab = open(fileNameMatlab)

listMatlab = dict()
currentFile = ''
data = []
# process the file
for line in fMatlab:
    # empty line means end of data set for of a file 
    if(line == '\n'):
        arr = array(data)
        # sort data by x-coordinate
        listMatlab[currentFile] = arr[arr[:,0].argsort()]
        data = []        
    else:
        fields = line.split()        
        # line indicating new file        
        if(fields[0] == 'File:'):
            currentFile = fields[1]
        else:
            # read data of the centroid x y area
            row_data = [float(fields[0])-1.0, float(fields[1]), float(fields[2])]
            data.append(row_data)
    
fMatlab.close()

# import data from starcamera
fStarCam = open(fileNameStarCam)

listStarCam = dict()
currentFile = ''
data = []
# process the file
for line in fStarCam:
    # empty line means end of data set for of a file 
    if(line == '\n'):
        arr = array(data)
        # sort data by x-coordinate
        listStarCam[currentFile] = arr[arr[:,0].argsort()]
        data = []        
    else:
        fields = line.split()        
        # line indicating new file        
        if(fields[0] == 'File:'):
            currentFile = fields[1].split('.')[0]
        else:
            # read data of the centroid x y area
            row_data = [float(x) for x in fields]	
            data.append(row_data)

fStarCam.close()


##################################################################
# import data from CC
fCC = open(fileNameCC)

listCC = dict()
currentFile = ''
data = []
for line in fCC:
    # empty line means end of data set for of a file 
    if(line == '\n'):
        arr = array(data)
        # sort data by x-coordinate
        listCC[currentFile] = arr[arr[:,0].argsort()]
        data = []        
    else:
        fields = line.split()        
        # line indicating new file        
        if(fields[0] == 'File:'):
            currentFile = fields[1]
        else:
            # read data of the centroid x y area
            row_data = [float(x) for x in fields]
            data.append(row_data)
    
fCC.close()
###################################################################


############################################################################
#find similar points
matchedListCC = dict()
statsListCC = dict()
for key in listCC.keys():
   temp = []
   dist = []
   count = 0
   matlab = listMatlab[key]
   star = listCC[key]
   for row in matlab:
       # check for similar point in x-direction
#       row[1] -= 1
       maskX = (star[:,0] < (row[0] + epsX)) & (star[:,0] > (row[0] - epsX))
       if(maskX.sum() > 0):
       #  check if possible points are also similar in y-direction
           maskY = (star[maskX,1] < (row[1] + epsY)) & (star[maskX,1] > (row[1] - epsY))
           # if maskY has one true element then a similar point has been found
           if(maskY.sum() == 1):
               result = star[maskX,:][maskY].flatten()
               # calculate the distance between the found points
               dx1 = result[0] - row[0]
               dx2 = result[3] - row[0]
               dy1 = result[1] - row[1]       
               dy2 = result[4] - row[1]        
               dist.append([sqrt(dx1**2 + dy1**2), sqrt(dx2**2 + dy2**2)])
               temp.append(result)
               
               count += 1
           elif (maskY.sum() > 1):
               print 'Ambiguous matching in file'
               print key
   matchedListCC[key] = array(temp)
   statsListCC[key] = [len(matlab), len(star), count, array(dist)]
   
#################################################################################   

circleCentroid = []
weightedCentroid1 = []
weightedCentroid2 = []
for key in statsList.keys():
    circleCentroid.extend(statsList[key][3][:,0])
    weightedCentroid1.extend(statsList[key][3][:,1])
    weightedCentroid2.extend(statsList[key][3][:,2])    
    
circleCentroid = array(circleCentroid)
weightedCentroid1 = array(weightedCentroid1)
weightedCentroid2 = array(weightedCentroid2)
    

figure()
hist(weightedCentroid1, 40, (0,4.), label = 'Weighted centroiding', color='red')
hist(circleCentroid, 40, (0,4.), label = 'Circular centroiding', alpha=0.75)
xlabel("Displacement (px)")
ylabel("Number of matched spots")
pl.ylim((0,200))
legend()

figure()
hist(weightedCentroid1, 40, (0,4.), label = 'Weighted centroiding', color='red')
hist(weightedCentroid2, 40, (0,4.), label = 'Bounding rect. centroiding', alpha=0.75)
xlabel("Displacement (px)")
ylabel("Number of matched spots")
pl.ylim((0,200))
legend()

################################################################################
#y = []   
#for key in statsListCC.keys():
#    y.append(statsListCC[key][0:3])
#    
#y = array(y)
#x = range(len(statsListCC))
#
#figure()
#plot(x, y[:,0], x, y[:,1], x, y[:,2])
#
#ccCentroid = []
#ccCentroid2 = []
#for key in statsListCC.keys():
#    ccCentroid.extend(statsListCC[key][3][:,0])
#    ccCentroid2.extend(statsListCC[key][3][:,1])    
#    
#ccCentroid = array(ccCentroid)
#ccCentroid2 = array(ccCentroid2)   
#
#figure()
#hist(weightedCentroid1, 40, (0,4.), label = 'Weighted centroiding (contours)', color='red')
#hist(ccCentroid, 40, (0,4.), label = 'Geometric centroiding (conn. comp.)', alpha=0.75)
#xlabel("Displacement (px)")
#ylabel("Number of matched spots")
#pl.ylim((0,1400))
#legend()
#
#figure()
#hist(weightedCentroid1, 40, (0,4.), label = 'Weighted centroiding (contours)', color='red')
#hist(ccCentroid2, 40, (0,4.), label = 'Weighted centroiding (conn. comp.)', alpha=0.75)
#xlabel("Displacement (px)")
#ylabel("Number of matched spots")
#pl.ylim((0,1400))
#legend()
#
#
#################################################################################

show()
   
    
                
