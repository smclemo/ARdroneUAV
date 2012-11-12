arm-none-linux-gnueabi-gcc -o compiled/video_test -O3 v4l2_test/fast.c v4l2_test/fast_9.c v4l2_test/fast_10.c v4l2_test/fast_11.c v4l2_test/fast_12.c v4l2_test/nonmax.c video_test.c
PAUSE

FTP -v -i -s:ftpscriptX.txt

@ECHO OFF
  :: temp_SendKeys.VBS will contain the "commands"
  ECHO.set handler=WScript.CreateObject("WScript.Shell")>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS

  :: Open Connection To DMZ
  ECHO.handler.SendKeys "open 192.168.1.1~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS

  :: Send Stuff to be done
  ECHO.handler.SendKeys "cd /data/video~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
  :: Kill main process
  ECHO.handler.SendKeys "killall program.elf~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS

  :: Run custom code
  ECHO.handler.SendKeys "chmod 777 /data/video/video_test~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 500 >>temp_SendKeys.VBS
  ECHO.handler.SendKeys "./video_test~" >>temp_SendKeys.VBS
  ECHO.WScript.sleep 5500 >>temp_SendKeys.VBS
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
