# creates `.vscode/environment.json` file if not exists
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

# if `--ixx` argument is passed, compiles `ywstd` module
if ($Args -contains "--ixx") {
  Write-Host "[TASK] Compiling ywstd"
  Remove-Item -Path ywstd/ywstd.ifc -ErrorAction SilentlyContinue
  Invoke-Expression "&`"$($e.cl)`" /std:c++latest /EHsc /nologo /W4 /O2 /Qpar /utf-8 /DYWLIB=1 /c ywstd/ywstd.ixx /Foywstd/ywstd.obj /ifcOutputywstd/ywstd.ifc /I`"$($e.msvc_inc)`" /I`"$($e.ucrt_inc)`" /I`"$($e.um_inc)`" /I`"$($e.shared_inc)`" /I`"$($e.winrt_inc)`" /I`"$($e.cppwinrt_inc)`""
  if (-not (Test-Path ywstd/ywstd.ifc)) {
    Write-Host "[TASK] Failed to compile ywstd"
    exit 1
  } else {
    Write-Host "[TASK] ywstd compiled"
  }
}

# finds cpp file
$cpp = Get-ChildItem -Path . -Filter *.cpp -Recurse | Select-Object -First 1

# if `--cpp` argument is passed, compiles an executable with `ywstd` module
if ($Args -contains "--cpp") {

  # compiles executable
  Write-Host "[TASK] Compiling $($cpp.Name)"
  Remove-Item -Path .\$($cpp.BaseName).exe -ErrorAction SilentlyContinue
  Invoke-Expression "&`"$($e.cl)`" /std:c++latest /EHsc /nologo /W4 /O2 /Qpar /utf-8 /DYWLIB=2 $cpp /reference ywstd=ywstd/ywstd.ifc /link ywstd/ywstd.obj /LIBPATH:`"$($e.msvc_lib)`" /LIBPATH:`"$($e.ucrt_lib)`" /LIBPATH:`"$($e.um_lib)`""
  if (-not (Test-Path .\$($cpp.BaseName).exe)) {
    Write-Host "[TASK] Failed to compile $($cpp.Name)"
    exit 2
  } else {
    Write-Host "[TASK] Compiled $($cpp.Name)"
  }

  # deletes object files
  Remove-Item -Path .\$($cpp.BaseName).obj
}


# if `--run` argument is passed, runs the executable
if ($Args -contains "--run") {
  Write-Host "[TASK] Running $($cpp.Name)"
  Invoke-Expression .\$($cpp.BaseName).exe
}
