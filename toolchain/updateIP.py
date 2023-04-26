import os,sys
os.chdir(sys.path[0])
lwip_comm_c_addr = "../src/LWIP/lwip_app/lwip_comm/lwip_comm.c"

import configparser
cp = configparser.ConfigParser()
cp.read('list.cfg')
cfgfile = open("list.cfg",'w')
index = cp.get('Init','START')
lwip_write_str = '192.168.192.' + index
print("set ip " + lwip_write_str)
cp.set('Init','START',str(int(index)+1))
cp.write(cfgfile)
cfgfile.close()

os.mkdir("./"+lwip_write_str)

f = open(lwip_comm_c_addr,'r')
rewrite_content = []
for i in f.readlines():
	if "lwipx->ip[3]=" in i:
		i = "\tlwipx->ip[3]=" + index + ";\n" 
		print(i)
	rewrite_content.append(i) 
f.close()
f2 = open(lwip_comm_c_addr,"w")
f2.writelines(rewrite_content)
f2.close()

down_firmware_addr = "../iapTool/F4Kernel/loadf4kernel.py"
f3 = open(down_firmware_addr, 'r',encoding='utf-8')
py_rewrite_content = []
for i in f3.readlines():
	if "F4K_ip = '192.168.192." in i:
		i = "\t\tF4K_ip = '192.168.192." + index + "'\n"
	py_rewrite_content.append(i)
f3.close()
f4 = open(down_firmware_addr,"w")
f4.writelines(py_rewrite_content)
f4.close()
