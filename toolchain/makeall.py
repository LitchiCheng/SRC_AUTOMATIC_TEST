import os
import sys

pathfile = './keilpath.ini'
try:
    f = open(pathfile,'r')
    keilpath = f.readline()
    f.close()
except FileNotFoundError:
    print('Cannot find path of keil.exe, For example: C:\\engineering\\keil\\UV4\\UV4.exe')
    keilpath = input('please paste the path of keil.exe here\n')

if(os.path.exists(keilpath) is False):
    input('keil path error: ' + keilpath + ', file not exist')
    sys.exit(1)

print('Find keil at: '+keilpath)

if(os.path.exists(pathfile) is False):
    f = open(pathfile, 'w')
    f.write(keilpath)

projpath = [ 
'..\\appProject\\Project.uvprojx',
'..\\SRC2000_Project\\Project.uvprojx',
'..\\udpBootLoader\\src2000Proj\\Project.uvprojx',
'..\\udpBootLoader\\Project\\Project.uvprojx',
]

ret = os.system(keilpath + ' -b -j0 ' + projpath[0] + ' -o .\\build_log.txt')
# os.system('cd >> 123.txt')
# ret = os.system(keilpath + ' -b -j0 ' + '..\\appProject\\Project.uvprojx')
print(ret)




