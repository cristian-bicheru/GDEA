import os
import time

GDEA_DIR = '.'
IN_IMG = open('img.png','rb')

os.chdir(GDEA_DIR + '')
from gdea import *
start = time.time()

variName = 'img_x.jpg'

if not os.path.exists(GDEA_DIR + '/temp'):
    os.mkdir('temp')

split(40000, IN_IMG, GDEA_DIR + '/temp')

count = len(os.listdir(GDEA_DIR + '/temp/'))

kp = genKeyPair()

for iteration in range(count):
    tempName = variName.replace('x', str(iteration))
    rawData = rawToBytearray(GDEA_DIR + '/temp/'+tempName)
    cipherText = encrypt(kp, rawData)
    with open(str(GDEA_DIR + '/temp/'+tempName+".gdea"), "w") as dout:
        for item in cipherText:
            dout.write(str(item)+",")   

file = bytearray()
count = (len(os.listdir(GDEA_DIR + '/temp/')))//2

for iteration in range(0, int(count)):
    tempName = variName.replace('x', str(iteration))
    tempCipher = []
    with open(str('C://Users/biche/Desktop/gdea2/temp/'+tempName + ".gdea"), "r") as dout:
        line = dout.readline()
        rawData = line.split(',')
        tempCipher.append(rawData[0])
        tempCipher.append(rawData[1])
        tempCipher.append(rawData[2])
    rawBin = decrypt(kp, tempCipher, 'return')
    file.extend(rawBin)

with open(str('out_img.png'), "wb") as imageFile:
    imageFile.write(file)
    imageFile.close()

print(str(time.time()-start))
