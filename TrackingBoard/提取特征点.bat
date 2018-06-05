
setlocal enabledelayedexpansion 
for /l %%i in (1 1 230) do (
siftWin32 <data1\gray\%%i.pgm >tmp.key
copy tmp.key data1\key\%%i.key
)
pause