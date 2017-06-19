del *.sdf
del *.ncb
del /A:H *.suo
del /A:H *.obj
rmdir /Q /S "Debug"
rmdir /Q /S "Release"
rmdir /Q /S "udock\Debug"
rmdir /Q /S "udock\Release"
pause
