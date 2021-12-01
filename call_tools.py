import os

# os.system("gen_six.exe skybox ./assets/textures/hdr/Arches_E_PineTree_3k.hdr ./assets/textures/skybox/ 2048")
# os.system("gen_six.exe irr ./assets/textures/hdr/Arches_E_PineTree_3k.hdr ./assets/textures/skybox/ 256")
os.system("gen_one.exe skybox ./assets/textures/hdr/Arches_E_PineTree_3k.hdr ./assets/textures/skybox/")
os.system("gen_one.exe irr ./assets/textures/hdr/Arches_E_PineTree_3k.hdr ./assets/textures/skybox/")
os.system("gen_one.exe pref ./assets/textures/hdr/Arches_E_PineTree_3k.hdr ./assets/textures/skybox/")
