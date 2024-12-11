# ident size: 2
# encoding: utf-8

# options
# --std -> compiles ywstd.ixx
# --ixx -> compiles ywlib.ixx
# --cpp -> compiles [first found .cpp file]
# --run -> runs executable if compiled successfully

import os
import sys
import glob
import json
import subprocess

env_json = {} # environments

# creates .vscode/environment.json if not exists
if not os.path.exists('.vscode/environment.json'):
  print("creating .vscode/environment.json")
  # search MSVC
  cl_exe_path = R"C:\Program Files*\Microsoft Visual Studio\*\*\VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe"
  for path in glob.glob(cl_exe_path):
    env_json["cl_exe"] = path
    break
  if "cl_exe" not in env_json:
    print("MSVC not found")
    sys.exit(1)
  env_json["msvc_inc"] = env_json["cl_exe"].replace(R"bin\Hostx64\x64\cl.exe", "include")
  env_json["msvc_lib"] = env_json["msvc_inc"].replace("include", R"\lib\x64")
  # search Windows Kits
  ucrt_inc_path = R"C:\Program Files*\Windows Kits\*\Include\*\ucrt"
  for path in glob.glob(ucrt_inc_path):
    env_json["ucrt_inc"] = path
    break
  if "ucrt_inc" not in env_json:
    print("Windows Kits not found")
    sys.exit(1)
  env_json["ucrt_lib"] = env_json["ucrt_inc"].replace("Include", "Lib") + R"\x64"
  env_json["um_inc"] = env_json["ucrt_inc"].replace("ucrt", "um")
  env_json["um_lib"] = env_json["ucrt_lib"].replace("ucrt", "um")
  env_json["shared_inc"] = env_json["ucrt_inc"].replace("ucrt", "shared")
  env_json["winrt_inc"] = env_json["ucrt_inc"].replace("ucrt", "winrt")
  env_json["cppwinrt_inc"] = env_json["ucrt_inc"].replace("ucrt", "cppwinrt")
  with open('.vscode/environment.json', 'w', encoding='utf-8') as f:
    json.dump(env_json, f, ensure_ascii=False, indent=2)

# reads .vscode/environment.json
with open('.vscode/environment.json', 'r', encoding='utf-8') as f:
  env_json = json.load(f)
cl_exe = env_json["cl_exe"]
msvc_inc = env_json["msvc_inc"]
msvc_lib = env_json["msvc_lib"]
ucrt_inc = env_json["ucrt_inc"]
ucrt_lib = env_json["ucrt_lib"]
um_inc = env_json["um_inc"]
um_lib = env_json["um_lib"]
shared_inc = env_json["shared_inc"]
winrt_inc = env_json["winrt_inc"]
cppwinrt_inc = env_json["cppwinrt_inc"]

# compiles ywstd as module if not exists
if not os.path.exists('ywstd.ifc') or "--std" in sys.argv:
  args = [cl_exe, "/c", "ywstd.ixx", ]
  args += ["/std:c++latest", "/EHsc", "/nologo", "/W4", "/O2", "/Qpar", "/utf-8", ]
  args += ["/DYWSTD_COMPILE=true", "/Foywstd.obj", "/ifcOutputywstd.ifc", ]
  args += [f"/I{msvc_inc}", f"/I{ucrt_inc}", f"/I{um_inc}", f"/I{shared_inc}", f"/I{winrt_inc}", f"/I{cppwinrt_inc}", ]
  print("compiling ywstd")
  subprocess.run(args, check=True)
  if os.path.exists('ywstd.ifc'):
    print("ywstd successfully compiled")
  else:
    print("failed to compile ywstd")
    sys.exit(1)

# compiles ywlib as module if not exists
if not os.path.exists('ywlib.ifc') or "--ixx" in sys.argv:
  args = [cl_exe, "/c", "ywlib.ixx", ]
  args += ["/std:c++latest", "/EHsc", "/nologo", "/W4", "/O2", "/Qpar", "/utf-8", ]
  args += ["/DYWLIB_COMPILE=true", "/DYWSTD_IMPORT=true", "/Foywlib.obj", "/ifcOutputywlib.ifc", ]
  args += [f"/I{msvc_inc}", f"/I{ucrt_inc}", f"/I{um_inc}", f"/I{shared_inc}", f"/I{winrt_inc}", f"/I{cppwinrt_inc}", ]
  args += [f"/reference ywstd=ywstd.ifc", ]
  print("compiling ywlib")
  subprocess.run(args, check=True)
  if os.path.exists('ywlib.ifc'):
    print("ywlib successfully compiled")
  else:
    print("failed to compile ywlib")
    sys.exit(1)

# compiles [first found .cpp file] as executable
if "--cpp" in sys.argv:
  cpp_file = None
  for path in glob.glob('*.cpp'):
    cpp_file = path
    break
  if not cpp_file:
    print("no .cpp file found")
    sys.exit(1)
  args = [cl_exe, cpp_file, ]
  args += ["/std:c++latest", "/EHsc", "/nologo", "/W4", "/O2", "/Qpar", "/utf-8", ]
  args += ["/DYWSTD_IMPORT=true", "/DYWLIB_IMPORT=true", "/reference ywstd=ywstd.ifc", "/reference ywlib=ywlib.ifc", ]
  args += ["/link", "ywstd.obj", "ywlib.obj",  f"/LIBPATH:{msvc_lib}", f"/LIBPATH:{ucrt_lib}", f"/LIBPATH:{um_lib}", ]
  exe_file = cpp_file.replace(".cpp", ".exe")
  obj_file = cpp_file.replace(".cpp", ".obj")
  if os.path.exists(exe_file): os.remove(exe_file)
  subprocess.run(args)
  if os.path.exists(obj_file): os.remove(obj_file)
  if os.path.exists(exe_file): print(f"{cpp_file} successfully compiled")
  else: print(f"failed to compile {cpp_file}")
  if "--run" in sys.argv and os.path.exists(exe_file):
    print(f"running {exe_file}")
    subprocess.run(exe_file)
    sys.exit(0)

# runs executable
if "--run" in sys.argv:
  exe_file = None
  for path in glob.glob('*.exe'):
    exe_file = path
    break
  if not exe_file:
    print("no .exe file found")
    sys.exit(1)
  print(f"running {exe_file}")
  subprocess.run(exe_file)
