# creates `.vscode/environment.json` file
if (-not (Test-Path .vscode/environment.json)) {
  Write-Host "[TASK] Creating .vscode/environment.json"
  $cl = (Get-ChildItem cl.exe -Path "C:\Program Files*\Microsoft Visual Studio\*\*\VC\Tools\MSVC\*\bin\Hostx64\x64" -Recurse | Select-Object -First 1).FullName
  $ucrt_lib = (Get-ChildItem ucrt -Path "C:\Program Files*\Windows Kits\10\Lib\*\" -Directory -Recurse | Select-Object -Last 1).FullName
  $ucrt_inc = $ucrt_lib -replace "Lib", "Include"
  $e = @{
    cl = $cl
    msvc_lib = $cl -replace "\\bin\\Hostx64\\x64\\cl.exe", "\lib\x64"
    msvc_inc = $cl -replace "\\bin\\Hostx64\\x64\\cl.exe", "\include"
    ucrt_lib = $ucrt_lib + "\x64"
    um_lib = ($ucrt_lib -replace "ucrt", "um") + "\x64"
    ucrt_inc = $ucrt_inc
    um_inc = $ucrt_inc -replace "ucrt", "um"
    shared_inc = $ucrt_inc -replace "ucrt", "shared"
    winrt_inc = $ucrt_inc -replace "ucrt", "winrt"
    cppwinrt_inc = $ucrt_inc -replace "ucrt", "cppwinrt"
  }
  $e | ConvertTo-Json | Set-Content .vscode/environment.json
}

# reads `.vscode/environment.json` file
$e = Get-Content .vscode/environment.json | ConvertFrom-Json

# compiles ywlib c++ module
if (($Args -contains "--rebuild") -or -not (Test-Path ywlib/ywlib.ifc)) {
  Write-Host "[TASK] Compiling ywlib"
  Remove-Item -Path ywlib/ywlib.ifc -ErrorAction SilentlyContinue
  Invoke-Expression "&`"$($e.cl)`" /std:c++latest /EHsc /nologo /W4 /O2 /Qpar /utf-8 /DYWLIB=1 /c ywlib/ywlib.ixx /Foywlib/ywlib.obj /ifcOutputywlib/ywlib.ifc /I`"$($e.msvc_inc)`" /I`"$($e.ucrt_inc)`" /I`"$($e.um_inc)`" /I`"$($e.shared_inc)`" /I`"$($e.winrt_inc)`" /I`"$($e.cppwinrt_inc)`""
  if (-not (Test-Path ywlib/ywlib.ifc)) {
    Write-Host "[TASK] Failed to compile ywlib"
    exit 1
  } else {
    Write-Host "[TASK] Compiled ywlib"
  }
}
if (($Args -contains "--moduleonly")) {
  exit 0
}

# finds cpp file
$cpp = Get-ChildItem -Path . -Filter *.cpp -Recurse | Select-Object -First 1

# compiles executable
Write-Host "[TASK] Compiling $($cpp.Name)"
Remove-Item -Path .\$($cpp.BaseName).exe -ErrorAction SilentlyContinue
Invoke-Expression "&`"$($e.cl)`" /std:c++latest /EHsc /nologo /W4 /O2 /Qpar /utf-8 /DYWLIB=2 $cpp /reference ywlib=ywlib/ywlib.ifc /link ywlib/ywlib.obj /LIBPATH:`"$($e.msvc_lib)`" /LIBPATH:`"$($e.ucrt_lib)`" /LIBPATH:`"$($e.um_lib)`""
if (-not (Test-Path .\$($cpp.BaseName).exe)) {
  Write-Host "[TASK] Failed to compile $($cpp.Name)"
  exit 2
} else {
  Write-Host "[TASK] Compiled $($cpp.Name)"
}

# deletes object files
Remove-Item -Path .\$($cpp.BaseName).obj

# runs executable if `--run` argument is passed
if ($Args -contains "--run") {
  Write-Host "[TASK] Running $($cpp.Name)"
  Invoke-Expression .\$($cpp.BaseName).exe
}
