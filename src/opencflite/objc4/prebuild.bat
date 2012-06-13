@echo off

echo prebuild: installing headers
xcopy /Y "%ProjectDir%runtime\objc.h" "%SolutionDir%include\objc\"
xcopy /Y "%ProjectDir%runtime\objc-api.h" "%SolutionDir%include\objc\"
xcopy /Y "%ProjectDir%runtime\objc-auto.h" "%SolutionDir%include\objc\"
xcopy /Y "%ProjectDir%runtime\objc-class.h" "%SolutionDir%include\objc\"
xcopy /Y "%ProjectDir%runtime\objc-exception.h" "%SolutionDir%include\objc\"
xcopy /Y "%ProjectDir%runtime\message.h" "%SolutionDir%include\objc\"
xcopy /Y "%ProjectDir%runtime\runtime.h" "%SolutionDir%include\objc\"
xcopy /Y "%ProjectDir%runtime\hashtable.h" "%SolutionDir%include\objc\"
xcopy /Y "%ProjectDir%runtime\hashtable2.h" "%SolutionDir%include\objc\"
xcopy /Y "%ProjectDir%runtime\maptable.h" "%SolutionDir%include\objc\"

echo prebuild: setting version
version
