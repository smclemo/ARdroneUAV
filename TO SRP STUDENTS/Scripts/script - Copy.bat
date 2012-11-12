arm-none-linux-gnueabi-gcc -o compiled/v4l2_test5 -O3 v4l2_test/v4l2_test5.c
PAUSE

@ECHO OFF
  :: temp_SendKeys.VBS will contain the "commands"
  ECHO.set handler=WScript.CreateObject("WScript.Shell")>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS

  :: Open Connection To DMZ
  ECHO.handler.SendKeys "open 192.168.1.1~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS

  :: Send Stuff to be done
  ECHO.handler.SendKeys "mv /data/video/usb/v4l2_test5 /data/video~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys "cd /data/video~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS


  :: Run custom code
  ECHO.handler.SendKeys "chmod 777 /data/video/v4l2_test5~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys "./v4l2_test5~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 400000 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys "ls -l *.yuv~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS

  :: Logout
  ECHO.handler.SendKeys "exit~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 250 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys " " >>temp_SendKeys.VBS
  ECHO.WScript.sleep 250 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys "quit~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 250 >>temp_SendKeys.VBS

  :: Open a Telnet Windows
  start telnet.EXE

  :: Run the script
  cscript//nologo temp_SendKeys.VBS

  :: Delete the temporary file 
  DEL temp_SendKeys.VBS

FTP -v -i -s:ftpscript2.txt
