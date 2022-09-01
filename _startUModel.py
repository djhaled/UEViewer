import subprocess
import os


VAL_KEY = '0x4BE71AF2459CF83899EC9DC2CB60E22AC4B3047E0211034BBABE9D174C069DD6'
VAL_PAKS_PATH = 'E:\\Riot Games\\VALORANT\\live\\ShooterGame\\Content\\Paks'
VAL_UMODEL_EXE = 'umodel.exe'
newpath = "D:\\GameVal\\"
#-path=E:\\Riot Games\\VALORANT\\live\\ShooterGame\\Content\\Paks, -export, TP_Core_NewMale_Skelmesh
def runUmodel():
	calltomake = [VAL_UMODEL_EXE,f"-out={newpath}",f"-path={VAL_PAKS_PATH}",f"-game=VALORANT","-filelist=C:\\UEViewer\\aesfile.txt",f"-aes={VAL_KEY}"]
	print(calltomake)
	subprocess.call(calltomake)


runUmodel()