import os,sys,time

for i in range(0,70):
    os.system("C:/Keil_v5/UV4/UV4.exe -b ../udpBootLoader/src2000Proj/Project.uvprojx")
    time.sleep(2)
    os.system("C:/Keil_v5/UV4/UV4.exe -b ../SRC_TEST_Project/Project.uvprojx")
    time.sleep(2)
