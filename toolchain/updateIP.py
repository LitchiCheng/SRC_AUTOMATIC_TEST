import json, os, stat, sys

os.chdir(sys.path[0])

outFilename = '..\\src\\utility\\network_conf.h'
ipconfig_json = '..\\src\\User\\ipconfig.json'
binFile = '..\\Output\\Project.bin'

try:
	if(os.stat(ipconfig_json)[stat.ST_MTIME] > os.stat(outFilename)[stat.ST_MTIME]):
		print(ipconfig_json + 'modified. Updating' + outFilename + '...')
	else:
		print('nothing to be done for all')
		quit()
except Exception as e:
	print(e)
	print('time compare exception')

with open(ipconfig_json,'r') as f:
    data = json.load(f)
    f.close()

ipv4_addr = '#define IPV4_ADDR\t\t\t\t{' + data['inet addr'].replace('.', ', ') + '}\n'
ipv4_gateway = '#define IPV4_GATEWAY\t\t{' + data['Gateway'].replace('.', ', ') + '}\n'
ipv4_broadcast = '#define IPV4_BROADCAST\t{' + data['Bcast'].replace('.', ', ') + '}\n'
ipv4_subnetmask = '#define IPV4_SUBNETMASK\t{' + data['Mask'].replace('.', ', ') + '}\n'
mac_addr = '#define MAC_ADDR\t\t\t\t{0x' + data['HWaddr fc'].replace(':', ', 0x') + '}\n'
# print (mac_addr)

try:
	os.chmod(outFilename, 755)
except:
	pass
	
output = open(outFilename, 'w')

output.write('//Attention: This file is created by Python script.\n')
output.write('//Keep this file READ-ONLY.\n//If you want to change the ipconfig, edit on /User/ipconfig.json\n\n')
output.write('#ifndef NETWORKD_CONF_H\n#define NETWORKD_CONF_H\n\n')
output.write(ipv4_addr)
output.write(ipv4_gateway)
output.write(ipv4_broadcast)
output.write(ipv4_subnetmask)
output.write(mac_addr)
output.write('\n#endif\n')

output.close()
os.chmod(outFilename, 111)
print("IP address updated.")
