@echo off
SET compiler=-nologo -Zi -Od -W4 -wd4005 -wd4100 -wd4201 -wd4244 -wd4305 -wd4311 -wd4312 -wd4366 -wd4457 -wd4459 -wd4505 -wd4530 -wd4838
SET linker=/IGNORE:4098 /IGNORE:4099 /IGNORE:4286 /OUT:L-IMGUI" "(windows).exe
SET definitions=/D _MBCS /D _CRT_SECURE_NO_WARNINGS
SET libraries=user32.lib shcore.lib gdi32.lib winmm.lib opengl32.lib xinput.lib

cd ..
IF NOT EXIST p:/L-IMGUI/build mkdir build
cd build
cl %compiler% %definitions% p:/L-IMGUI/source/l_imgui_windows.cpp %libraries% /I p:/L-IMGUI/source /link %linker% 
cd ..
cd source


