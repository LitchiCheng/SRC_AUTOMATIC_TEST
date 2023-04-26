import os,sys
os.chdir(sys.path[0])
nowname = '../udpBootLoader/src2000Proj/Output/Project.sc2b.bin'
import configparser
cp = configparser.ConfigParser()
cfgfile = open("list.cfg",'r')

cp.read('./list.cfg')
print("sss " + cp.get('Init','START'))
index_int = int(cp.get('Init','START'))
ip = '192.168.192.' + str(index_int-1)
new_name = './'+ip+'/BL_'+ip+'.sc2b.bin'
os.rename(nowname, new_name)
cfgfile.close()