file = r'C:\projects\obs-studio\UI\window-basic-settings.cpp'

'''
	bool enableAutoUpdates = config_get_bool(GetGlobalConfig(), "General",
						 "EnableAutoUpdates");
config_get_bool(GetGlobalConfig(), "General",
						 "EnableAutoUpdates");
'''
f = open(file,'r',encoding='utf-8')
f2 = open(file + '.new','w',encoding='utf-8')
step = 0
for line in f.readlines():
    if step == 0 and line.find('bool enableAutoUpdates =') != -1:
        step += 1
        f2.write('bool enableAutoUpdates = false;\n')
    elif step == 1:
        step += 1
    else:
        f2.write(line)
f.close()
f2.close()
