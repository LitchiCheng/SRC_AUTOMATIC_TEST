import os,sys
os.chdir(sys.path[0])
nowname = '../SRC_TEST_Project/output/Project.sc2.bin'
import configparser
cp = configparser.ConfigParser()
print(os.getcwd())

cfgfile = open("list.cfg",'r')
cp.read('./list.cfg')
index = cp.get('Init','START')
index_int = int(index)
ip = '192.168.192.' + str(index_int-1)
new_name = './'+ip+'/App_'+ip+'.sc2.bin'
os.rename(nowname, new_name)
cfgfile.close()