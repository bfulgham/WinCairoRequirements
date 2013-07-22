cl /I../dist/include /D"WIN32" /D"_WIN32_WINNT=0x0501" /D"WINVER=0x0501" date_test.c ../dist/lib/CFLite.lib

xcopy date_test.exe ..\dist\bin /Y

