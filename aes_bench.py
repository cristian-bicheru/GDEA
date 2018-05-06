import base64
from Crypto.Cipher import AES
from Crypto.Hash import SHA256
from Crypto import Random
import os
import time
from gdea import *

os.chdir('C://Users/biche/Desktop/GDEA')
start = time.time()
random = 'mysecretpassword'.encode()

file = rawToBytearray('img.jpg')

def encrypt(key, source, encode=True):
    key = SHA256.new(key).digest()  # use SHA-256 over our key to get a proper-sized AES key
    IV = Random.new().read(AES.block_size)  # generate IV
    encryptor = AES.new(key, AES.MODE_CBC, IV)
    padding = AES.block_size - len(source) % AES.block_size  # calculate needed padding
    source += bytes([padding]) * padding  # Python 2.x: source += chr(padding) * padding
    data = IV + encryptor.encrypt(source)  # store the IV at the beginning and encrypt
    return base64.b64encode(data).decode("latin-1") if encode else data

def decrypt(key, source, decode=True):
    if decode:
        source = base64.b64decode(source.encode("latin-1"))
    key = SHA256.new(key).digest()  # use SHA-256 over our key to get a proper-sized AES key
    IV = source[:AES.block_size]  # extract the IV from the beginning
    decryptor = AES.new(key, AES.MODE_CBC, IV)
    data = decryptor.decrypt(source[AES.block_size:])  # decrypt
    padding = data[-1]  # pick the padding value from the end; Python 2.x: ord(data[-1])
    if data[-padding:] != bytes([padding]) * padding:  # Python 2.x: chr(padding) * padding
        raise ValueError("Invalid padding...")
    return data[:-padding]  # remove the padding

def int_to_bytes(x):
    return x.to_bytes((x.bit_length() + 7) // 8, 'big')

intp = str(int.from_bytes(file, byteorder='big', signed=False)).encode()

code = encrypt(random, intp)

back = decrypt(random, code)

outp = int_to_bytes(int(back.decode()))

with open(str('aesout_img.jpg'), "wb") as imageFile:
    imageFile.write(outp)

print(str(time.time()-start))
