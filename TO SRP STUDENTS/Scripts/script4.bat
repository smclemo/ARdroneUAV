cd ardrone4\motorboard
call make.bat

cd ..\navboard
call make.bat

cd ..\attitude
call make.bat

cd ..\vbat
call make.bat

cd ..\gpio
call make.bat

cd ..\video
call make.bat

cd ..\horizontal_velocities
call make.bat

cd ..\object_detect
call make.bat

cd ..\fly
call make.bat

cd ..\..
PAUSE

FTP -v -i -s:ftpscript5.txt

@ECHO OFF
  :: temp_SendKeys.VBS will contain the "commands"
  ECHO.set handler=WScript.CreateObject("WScript.Shell")>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS

  :: Open Connection To DMZ
  ECHO.handler.SendKeys "open 192.168.1.1~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS

  :: Run custom code
  ECHO.handler.SendKeys "cd /data/video~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
  :: Kill old process
  ECHO.handler.SendKeys "killall fly~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
  ::ECHO.handler.SendKeys "mv /data/video/usb/* /data/video~" >>temp_SendKeys.VBS
  ::ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
    :: Kill main process
  ECHO.handler.SendKeys "killall program.elf~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys "chmod 777 /data/video/*~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys "./horizontal_velocities~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 80000 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys "ls -l *.csv~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS

  :: Logout
  ECHO.handler.SendKeys "exit~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys " " >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys "quit~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 1000 >>temp_SendKeys.VBS

  :: Open a Telnet Windows
  start telnet.EXE

  :: Run the script
  cscript//nologo temp_SendKeys.VBS

  :: Delete the temporary file 
  DEL temp_SendKeys.VBS

FTP -v -i -s:ftpscript4.txt
