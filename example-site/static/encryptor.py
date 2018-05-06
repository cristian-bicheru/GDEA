import os
from gdea import *

directory = input() # ie /car/
basename = input() # ie car.jpg
variName = basename.replace('.', '_x.')

def encrypt_RSA(message):
    key = open('public.pem', "rb").read()
    rsakey = RSA.importKey(key)
    encrypted = rsakey.encrypt(message, 8)
    return encrypted[0]

count = len(os.listdir(directory))

keyPair = genKeyPair()
boi = str(",".join([str(i) for i in keyPair]))
encryptedKeyPair = encrypt_RSA(boi.encode())
with open((str(directory+variName)+".gdea.key"), "a") as keyfile:
    keyfile.write(str(encryptedKeyPair)+"\n")


for iteration in range (1, (count+1)):
    tempName = variName.replace('x', str(iteration))
    rawData = rawToBytearray(directory+tempName)
    cipherText = encrypt(keyPair, rawData)
    with open(str(directory+tempName+".gdea"), "w") as dout:
        for item in cipherText:
            dout.write("%s\n" % item)
