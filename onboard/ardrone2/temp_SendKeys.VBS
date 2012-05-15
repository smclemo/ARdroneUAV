set handler=WScript.CreateObject("WScript.Shell")
WScript.sleep 500 
handler.SendKeys "open 192.168.1.1~" 
WScript.sleep 500 
handler.SendKeys "chmod 777 /data/video/bin/*~" 
WScript.sleep 500 
handler.SendKeys "/data/video/bin/fly~" 
WScript.sleep 12500 
handler.SendKeys "exit~" 
WScript.sleep 250 
handler.SendKeys " " 
WScript.sleep 250 
handler.SendKeys "quit~" 
WScript.sleep 250 
