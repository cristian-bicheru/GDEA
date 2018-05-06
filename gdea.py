import os
import Crypto
from Crypto.PublicKey import RSA
from Crypto import Random
import ast
from decimal import *
import sympy
from fsplit.filesplit import FileSplit

# Graphical Data Encryption Algorithm Version 1.0
# JamHacks 2, 2018 Project
# By Cristian Bicheru
###################################################
# README
#
# This is the raw algorithm, however it should be
# noted that files above 60~70 kb can take around
# 1 second to much longer to decrypt depending on
# how much larger the file is. Hence the file to
# be encrypted should be split into 2-16 kb chunks
# and encrypted seperately. These chunks encrypt
# and decrypt near instantly. This current algo
# is safe for moderate encryption, but has some
# features disabled due to issues. Once these
# issues are fully patched, the algorithm should
# be good for maximum security useage.
#
# Make sure your application sends the key pair
# after being encrypted with non-textbook RSA!!!
###################################################


eqModifiers = ['x**5', 'x**4', 'x**3', 'x**2', 'x']

def split(size, file, outdir):
    fs = FileSplit(file=file, splitsize=size, output_dir=outdir)
    fs.split()

def rawToBytearray(file):
    with open(file, "rb") as rawFile:
      array = bytearray(rawFile.read())
    return array

def genRandom(num):
    return int.from_bytes(os.urandom(num), byteorder='little')

def genKeyPair():
    targetArea = genRandom(15) # Generates target area
    blockUCode = genRandom(1) # Generates unscrambling code
    Offset = 1 #genRandom(7) # Generates offset length, currently disbaled to to being problematic

    
    equationModComplexity = 5 # Minimum of 5 is recommended

    equationModifier = ""
    
    for placeholder in range(1, equationModComplexity):
        modulo = genRandom(1) % 5
        equationModifier = equationModifier + '+' + str(genRandom(1)) + "*" + eqModifiers[modulo]

    keyPair = []
    keyPair.append(targetArea)
    keyPair.append(blockUCode)
    keyPair.append(Offset)
    keyPair.append(equationModifier)

    return keyPair

def genRSA():
    ranGen = Random.new().read
    key = RSA.generate(2048, ranGen)
    return key
    

def encrypt(keyPair, file):
    
    plainInt = int.from_bytes(file, byteorder='big', signed=False) * int(keyPair[1])
    compressor = 65536*4 # splits data into many chunks for lower cipher text size and greater efficiency
    
    splitter = divmod(plainInt, compressor)

    # Currently only programmed to generate a straight line, curves G R E A T L Y increase security

    targetArea = keyPair[0]
    Offset = keyPair[2]
    equationModifier = keyPair[3]
    modulo = splitter[1] ####
    ####################### Encrypts the compressor
    slope = str(Decimal(2*int(targetArea))/Decimal((compressor-int(Offset))**2))
    b = str(Decimal((-1)*int(Offset)*float(slope)))
    compressorEquation = str(slope) + '*x' + ' +'+ str(b)
    compressorEquation.replace('+-', '-')
    compressorEquation = sympy.simplify(str(compressorEquation)+str(equationModifier))

    compressionFactor = splitter[0] # The compression factor from the raw number, can be encrypted too for more security
    #######################

    ####################### Encrypts the remainder
    modulo = splitter[1]
    
    slopeM = str(Decimal(2*int(targetArea))/Decimal((modulo-int(Offset))**2))
    bM = str(Decimal((-1)*int(Offset)*float(slopeM)))

    moduloEquation = str(slopeM)+'*x'+'+'+str(bM)
    moduloEquation = sympy.simplify(str(moduloEquation)+str(equationModifier))
    #######################

    cipherText = []
    cipherText.append(compressorEquation)
    cipherText.append(moduloEquation)
    cipherText.append(str(compressionFactor))
            
    return cipherText

def encryptMessage(keyPair, rawtext): ##### Message Encryptor

    tempArray = bytearray()
    tempArray.extend(map(ord, rawtext))
    plainInt = int.from_bytes(tempArray, byteorder='big', signed=False) * int(keyPair[1])
    compressor = 16384 # splits data into 16384 chunks for lower cipher text size and greater efficiency
    
    splitter = divmod(plainInt, compressor)

    # Currently only programmed to generate a straight line, curves G R E A T L Y increase security

    targetArea = keyPair[0]
    Offset = keyPair[2]
    equationModifier = keyPair[3]
    modulo = splitter[1] ####
    ####################### Encrypts the compressor
    slope = str(Decimal(2*int(targetArea))/Decimal((compressor-int(Offset))**2))
    b = str(Decimal((-1)*int(Offset)*float(slope)))
    compressorEquation = str(slope) + '*x' + ' +'+ str(b)
    compressorEquation.replace('+-', '-')
    compressorEquation = sympy.simplify(str(compressorEquation)+str(equationModifier))

    compressionFactor = splitter[0] # The compression factor from the raw number, can be encrypted too for more security
    #######################

    ####################### Encrypts the remainder
    modulo = splitter[1]
    
    slopeM = str(Decimal(2*int(targetArea))/Decimal((modulo-int(Offset))**2))
    bM = str(Decimal((-1)*int(Offset)*float(slopeM)))

    moduloEquation = str(slopeM)+'*x'+'+'+str(bM)
    moduloEquation = sympy.simplify(str(moduloEquation)+str(equationModifier))
    #######################

    cipherText = []
    cipherText.append(compressorEquation)
    cipherText.append(moduloEquation)
    cipherText.append(str(compressionFactor))
            
    return cipherText

def positiveValues(lst):
    return [x for x in lst if x > 0] or None

def int_to_bytes(x):
    return x.to_bytes((x.bit_length() + 7) // 8, 'big')

def decrypt(keyPair, cipherText, filenameout):

    targetArea = keyPair[0]
    blockUCode = keyPair[1]
    Offset = keyPair[2]
    equationModifier = keyPair[3]

    cipherCompressorEquation = cipherText[0]
    cipherModuloEquation = cipherText[1]
    compressionFactor = cipherText[2]

    ####################### Decrypts compressor
    equationCompressor = sympy.simplify(str(cipherCompressorEquation) + ' - (' + equationModifier + ")")
    x = sympy.Symbol('x')
    equationCompressor = "(" + str(sympy.integrate(equationCompressor, x))+") - ("+str(sympy.integrate(equationCompressor, x).subs(x, Offset))+")" +" - "+str(targetArea)
    compressorsol = sympy.solve(sympy.simplify(str(equationCompressor)), x)
    compressor = positiveValues(compressorsol)[0]
    #######################

    ####################### Decrypts remainder
    equationModulo = sympy.simplify(str(cipherModuloEquation) + ' - (' + equationModifier + ")")
    x = sympy.Symbol('x')
    equationModulo = "(" + str(sympy.integrate(equationModulo, x))+") - ("+str(sympy.integrate(equationModulo, x).subs(x, Offset))+")" +" - "+str(targetArea)
    modulosol = sympy.solve(sympy.simplify(str(equationModulo)), x)
    modulo = positiveValues(modulosol)[0]

    #######################
    if int(compressionFactor) > 0:
        rawFile = int_to_bytes(int(Decimal((int(compressor)*int(compressionFactor)+int(modulo))//int(blockUCode))))
    else:
        rawFile = int_to_bytes(int(Decimal((int(compressor)*(int(compressionFactor)+1)+int(modulo))//int(blockUCode))))

    if filenameout == 'return':
        return rawFile
    else:
        with open(str(filenameout), "wb") as imageFile:
            imageFile.write(rawFile)
        return

def decryptMessage(keyPair, cipherText):

    targetArea = keyPair[0]
    blockUCode = keyPair[1]
    Offset = keyPair[2]
    equationModifier = keyPair[3]

    cipherCompressorEquation = cipherText[0]
    cipherModuloEquation = cipherText[1]
    compressionFactor = cipherText[2]

    ####################### Decrypts compressor
    equationCompressor = sympy.simplify(str(cipherCompressorEquation) + ' - (' + equationModifier + ")")
    x = sympy.Symbol('x')
    equationCompressor = "(" + str(sympy.integrate(equationCompressor, x))+") - ("+str(sympy.integrate(equationCompressor, x).subs(x, Offset))+")" +" - "+str(targetArea)
    compressorsol = sympy.solve(sympy.simplify(str(equationCompressor)), x)
    compressor = positiveValues(compressorsol)[0]
    #######################

    ####################### Decrypts remainder
    equationModulo = sympy.simplify(str(cipherModuloEquation) + ' - (' + equationModifier + ")")
    x = sympy.Symbol('x')
    equationModulo = "(" + str(sympy.integrate(equationModulo, x))+") - ("+str(sympy.integrate(equationModulo, x).subs(x, Offset))+")" +" - "+str(targetArea)
    modulosol = sympy.solve(sympy.simplify(str(equationModulo)), x)
    modulo = positiveValues(modulosol)[0]

    #######################
    if int(compressionFactor) > 0:
        rawFile = int_to_bytes(int(Decimal((int(compressor)*int(compressionFactor)+int(modulo))//int(blockUCode))))
    else:
        rawFile = int_to_bytes(int(Decimal((int(compressor)*(int(compressionFactor)+1)+int(modulo))//int(blockUCode))))

    rawText = rawFile.decode("utf-8")
    
    return rawText
