@echo off
setlocal EnableExtensions EnableDelayedExpansion

::
:: clean up project directories
::

if "%1" == "" (
    echo Clean directories
	echo  clean-dirs  [show or clean]
    goto done
)

if "%1" == "show" (
    echo --- show directories and files to clean ---
	lr -n -P=.*\\build\\.* -r . 
	lr -n -F=*.class -r . 
	lr -n -F=*.apk -r . 
	lr -n -F=*.aar -r . 
	lr -n -F=*.exe -r . 
	lr -n -F=*.obj -r . 
)
if "%1" == "clean" (
	echo === show directories and files to DELETE ===
	lr -q -P=.*\\build\\.* -r . 
	lr -q -P=.*\\x64\\Debug\\.* -r . 
	lr -q -P=.*\\x64\\Release\\.* -r . 
	lr -q -F=*.aar -r . 
	lr -q -F=*.exe -r . 
	lr -q -F=*.obj -r .
	lr -f -P=.*\\\.vs\\.* -r . 
)

:done
endlocal