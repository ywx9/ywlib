# prepares the directory for modules and executables
if (-not (Test-Path "out")) { New-Item -ItemType Directory -Name "out" | Out-Null }

# finds the path to the `cl.exe`
$temp = (Get-ChildItem cl.exe -Path "C:\Program Files*\Microsoft Visual Studio\*\*\VC\Tools\MSVC\*\bin\Hostx64\x64" -Recurse | Select-Object -First 1).FullName

$env:PATH += ";$(Split-Path $temp -Parent)"

$std_ixx = $temp -replace "\\bin\\Hostx64\\x64\\cl.exe", "\modules\std.ixx"
$env:YW_MSVC_LIB = $temp -replace "\\bin\\Hostx64\\x64\\cl.exe", "\lib\x64"
$env:YW_MSVC_INC = $temp -replace "\\bin\\Hostx64\\x64\\cl.exe", "\include"

# finds the path to the Windows Kits
$temp = (Get-ChildItem ucrt -Path "C:\Program Files*\Windows Kits\10\Lib\*\" -Directory -Recurse | Select-Object -Last 1).FullName

$env:YW_WTK_LIB_UCRT = "$temp\x64"
$env:YW_WTK_LIB_UM = $env:YW_WTK_LIB_UCRT -replace "ucrt", "um"
$env:YW_WTK_INC_UCRT = $temp -replace "Lib", "Include"
$env:YW_WTK_INC_UM = $env:YW_WTK_INC_UCRT -replace "ucrt", "um"
$env:YW_WTK_INC_SHARED = $env:YW_WTK_INC_UCRT -replace "ucrt", "shared"
$env:YW_WTK_INC_WINRT = $env:YW_WTK_INC_UCRT -replace "ucrt", "winrt"
$env:YW_WTK_INC_CPPWINRT = $env:YW_WTK_INC_UCRT -replace "ucrt", "cppwinrt"

# compiles `std.ixx`
Invoke-Expression "cl /std:c++latest /EHsc /nologo /W4 /utf-8 /c `"$std_ixx`" /Foout\std.obj /ifcOutputout\std.ifc /I`"$env:YW_MSVC_INC`" /I`"$env:YW_WTK_INC_UCRT`" /I`"$env:YW_WTK_INC_UM`" /I`"$env:YW_WTK_INC_SHARED`" /I`"$env:YW_WTK_INC_WINRT`" /I`"$env:YW_WTK_INC_CPPWINRT`""
