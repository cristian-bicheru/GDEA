import os
import time

os.chdir('C://Users/biche/Desktop/GDEA')
from gdea import *
start = time.time()

variName = 'img_x.jpg'

split(20000, 'img.jpg', 'C://Users/biche/Desktop/GDEA/temp/')

count = len(os.listdir('C://Users/biche/Desktop/GDEA/temp/'))

kp = genKeyPair()

for iteration in range (1, int(count+1)):
    tempName = variName.replace('x', str(iteration))
    rawData = rawToBytearray('C://Users/biche/Desktop/GDEA/temp/'+tempName)
    cipherText = encrypt(kp, rawData)
    with open(str('C://Users/biche/Desktop/GDEA/temp/'+tempName+".gdea"), "w") as dout:
        for item in cipherText:
            dout.write(str(item)+",")   

file = bytearray()
count = (len(os.listdir('C://Users/biche/Desktop/GDEA/temp/')))/2

for iteration in range(1, int(count+1)):
    tempName = variName.replace('x', str(iteration))
    tempCipher = []
    with open(str('C://Users/biche/Desktop/GDEA/temp/'+tempName + ".gdea"), "r") as dout:
        line = dout.readline()
        rawData = line.split(',')
        tempCipher.append(rawData[0])
        tempCipher.append(rawData[1])
        tempCipher.append(rawData[2])
    rawBin = decrypt(kp, tempCipher, 'return')
    file.extend(rawBin)

with open(str('out_img.jpg'), "wb") as imageFile:
    imageFile.write(file)

print(str(time.time()-start))
