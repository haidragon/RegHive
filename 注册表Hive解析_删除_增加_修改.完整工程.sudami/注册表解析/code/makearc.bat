@if exist regedt33.rar fvermove.exe regedt33.rar bak\
@if errorlevel 1 goto bad
winrar -av a regedt33.rar *.cpp *.c *.h *.hpp *.dsp *.dsw *.bat *.rc *.bmp *.ico *.txt *.exe
@goto end
:bad
@pause
:end