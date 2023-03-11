import subprocess
import os

filist = "D:\\ExportFortUE\\maps\\athena_poi_durrrburger_001\\AllAssets.txt"
VAL_KEY = '0xF5936C8253EE162EF7C3654274ED829A147932D3303158D5FB5574B9A8192117'
VAL_PAKS_PATH = 'E:\\Apex Legends Mobile\\Apex Legends Mobile\\AClient\\Content\\Paks'
VAL_UMODEL_EXE = "umodel.exe"
newpath = "D:\\RunTown\\"#/Game/Athena/Environments/Landscape/Foliage/SM_Athena_Flowers_Group_01.uasset
#-path=E:\\Riot Games\\VALORANT\\live\\ShooterGame\\Content\\Paks, -export, TP_Core_NewMale_Skelmesh
def runUmodel():
	calltomake = [VAL_UMODEL_EXE,f"-out={newpath}",f"-path={VAL_PAKS_PATH}",f"-game=ue4.23",f"-aes={VAL_KEY}","-export","SM_I_ExtendedEnergyMag"]
	subprocess.call(calltomake)

runUmodel()