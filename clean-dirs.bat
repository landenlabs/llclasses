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
    p -C=green -p="--- show directories and files to clean ---\n"
	lr -n -P=.*\\build\\.* -P=.*\\x64\\Debug\\.* -P=.*\\x64\\Release\\.* -r . 
    lr -n -P=.*\\.vs\\.* -r . 
	lr -n -F=*.class -r . 
	lr -n -F=*.apk -F=*.aar -F=*.exe -F=*.obj -r . 
)
if "%1" == "clean" (
	p -C=red -p="=== show directories and files to DELETE ===\n"
	lr -q -P=.*\\build\\.* -r . 
	lr -q -P=.*\\x64\\Debug\\.* -P=.*\\x64\\Release\\.* -r . 
	lr -f -P=.*\\.vs\\.* -r . 
	lr -q -F=*.aar -F=*.exe -F=*.obj -r .
)

:done
endlocal