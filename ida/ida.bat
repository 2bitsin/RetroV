setlocal
set IDC=%~dp016bit.idc 0x%20
set LOG=%~dp0ida.log
"C:\Devel\IDA 7.6\ida64.exe" -A -L%LOG% -S"%IDC%" -P- -c -pmetapc -b%2 %~dp0..\workspace\ROMs\%1.bin



