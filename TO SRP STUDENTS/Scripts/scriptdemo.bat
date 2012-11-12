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
  ECHO.handler.SendKeys "chmod 777 /data/video/*~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys "./fly~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 70000 >>temp_SendKeys.VBS

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