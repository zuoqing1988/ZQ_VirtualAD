
setlocal enabledelayedexpansion 
for /l %%i in (270 1 340) do (
siftWin32 <data3\gray\%%i.pgm >tmp1.key
copy tmp1.key data3\key\%%i.key
)
pause