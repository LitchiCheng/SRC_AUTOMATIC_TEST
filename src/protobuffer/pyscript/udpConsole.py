import socket
import struct
import signal
import threading
from time import sleep
from datetime import datetime
import sys
from prompt_toolkit import prompt
from prompt_toolkit.history import FileHistory
from prompt_toolkit.completion import WordCompleter
from F4kCommandHandler import *
import logging
import os
import shutil
from logging.handlers import RotatingFileHandler


global is_exit
global printFlag
global logFlag
global is_log_param_set

is_exit = False
printFlag = True
logFlag = False
is_log_param_set = False

input_mutex = threading.Lock()

consoleVersion = '1.0.1'


def exit_handler():
    global is_exit
    is_exit = True
    sys.exit()


commanddict = {
    '': empty_commandhandler,
    "exit": exit_handler,
    "reset": reset_commandhandler,
    'debuginfo': debuginfo_commandhandler,
    'ntpcali': ntpcali_commandhandler,
    'gyrounlock': gyrounlock_commandhandler,
    'gyroversion': gyroversion_commandhandler,
    'gyroreset': gyroreset_commandhandler,
    'gyrorepower': gyrorepower_commandhandler,
    'gyro_goto_emergency': gyro_goto_emergency_commandhandler,
    'ifconfig': ifconfig_commandhandler,
    'uname': uname_commandhandler,
    'version': uname_commandhandler,
    'ododebug': ododebug_commandhandler,
    'odoreset': odoreset_commandhandler,
    'clear': clear_commandhandler,
    'errorinfo': errorinfo_commandhandler,
    'uartconsole': uartconsole_commandhandler,
    'taskmanager': taskmanager_commandhandler,
    'odojumpdebug': odojumpdebug_commandhandler
}


def help_handler():
    print('All the available command is list as follow:')
    for key in commanddict:
        print(key)
    sys.stdout.flush()

def log_start_handler():
    global logFlag
    logFlag = True
    print('log start........')

def log_stop_handler():
    global logFlag
    logFlag = False
    print('log stop........')

def make_folder(path):
    path=path.strip()
    path=path.rstrip("\\")
    isExists=os.path.exists(path)
    if not isExists:
        os.makedirs(path) 
        return True
    else:
        return False

def del_file(path):
    path=path.strip()
    path=path.rstrip("\\")
    isExists=os.path.exists(path)
    if not isExists:
        return True
    else:
        shutil.rmtree(path)
        return False   

commanddict['help'] = help_handler
commanddict['?'] = help_handler
commanddict['log_start'] = log_start_handler
commanddict['log_stop'] = log_stop_handler

cmdList = []
for key in commanddict:
    cmdList.append(key)

cmdCompleter = WordCompleter(cmdList)

def listenThreadFunc():
    global is_exit
    global is_log_param_set
    timestampflag = True
    local_addr = ('', 4999)
    so = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    so.settimeout(0.05)
    try:
        so.bind(local_addr)
    except OSError:
        print('There is already an udpConsole running, press enter to exit...')
        # input('Press enter to exit...')
        exit_handler()

    print('*** SeerDIOBoard udpConsole: V' + consoleVersion + ' ***\r\n')
    print('Local> Listening to local port %d' % local_addr[1])

    mkpath="\\UdpConsoleLog\\"
    pwd = os.getcwd()
    pwd = pwd.replace('\\','\\\\')
    out_address = pwd + mkpath
    del_file(out_address)

    while True:
        try:
            (stmlog, addr) = so.recvfrom(1000)
        except socket.timeout:
            continue
        except NameError:
            quit()
        
        if logFlag is True:
            if is_log_param_set is False:
                make_folder(out_address)
                logger = logging.getLogger(__name__)
                logger.setLevel(level = logging.INFO)
                rHandler = RotatingFileHandler((out_address+"log.txt"), maxBytes = 20*1024*1024,backupCount = 10)
                rHandler.setLevel(logging.INFO)
                formatter = logging.Formatter('[%(asctime)s] - %(levelname)s - %(message)s')
                rHandler.setFormatter(formatter)
                logger.addHandler(rHandler)
                is_log_param_set = True         
            try:
                str_to_log = stmlog.decode()
            except UnicodeDecodeError:
                print(stmlog)
            logger.info(str_to_log)

        if printFlag is True:
            if timestampflag is True:
                sys.stdout.write('[' + datetime.strftime(datetime.now(), '%H:%M:%S.%f')[0:-3] + '] ')
                timestampflag = False

            try:
                logstr = stmlog.decode()
            except UnicodeDecodeError:
                print(stmlog)
            else:
                if logstr.find('\r\n') >= 0:
                    timestampflag = True

                sys.stdout.write(logstr)
                sys.stdout.flush()

        if(is_exit):
            so.close()
            print('\n\bLocal> Listen port close')
            quit()


listenThread = threading.Thread(target=listenThreadFunc)
listenThread.setDaemon(True)
listenThread.start()


while True:
    try:
        user_input = prompt('Local> ',
                            history=FileHistory('history.txt'),
                            completer=cmdCompleter)
    except KeyboardInterrupt:
        exit_handler()
    sys.stdout.flush()
    try:
        commanddict[user_input]()
    except KeyError:
        print('Unknow command, see \'help\'')
        sys.stdout.flush()

    if('' != user_input):
        printFlag = True
    else:
        printFlag = not printFlag

    if is_exit:
        exit_handler()
