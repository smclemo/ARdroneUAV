set handler=WScript.CreateObject("WScript.Shell")
WScript.sleep 500 
handler.SendKeys "open 192.168.1.1~" 
WScript.sleep 500 
handler.SendKeys "cd /data/video~" 
WScript.sleep 500 
handler.SendKeys "chmod 777 /data/video/*~" 
WScript.sleep 500 
handler.SendKeys "./fly~" 
WScript.sleep 80000 
