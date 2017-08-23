if exist "Plugin64" (
  set X64=true
  7z a -r Libre64.zip Plugin64\
) else (
  set X64=false
  7z a Libre64.zip Plugin\
)

if exist "Bin\Debug\Project64.exe" (
  if "%X64%"=="true" (
    set Cfg=Debug64
  ) else (
    set Cfg=Debug
  )
) else (
  if "%X64%"=="true" (
    set Cfg=Release64
  ) else (
    set Cfg=Release
  )
)
7z a Libre64.zip .\Bin\%Cfg%\Project64.exe
7z a Libre64.zip .\Bin\%Cfg%\Config Config\
7z a Libre64.zip Config\
