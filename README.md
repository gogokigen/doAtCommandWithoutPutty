# zaAtCmdTool  

**If you want to use this tool:**  
echo -e "AT+CFUN=?\r\n" > /dev/smd11  
  
AT+CFUN=?   
+CFUN: (0-1,4-7),(0-1)   
OK  
  
  
**More AT commands**

Get modem status: at+cfun?  
  
Set modem online: at+cfun=1  
  
Get CS registration: AT+CREG?  
  
Get PS registration: AT+CGREG?  
  
PS attach or detach: AT+CGATT  

Dial: ATD09xxxxxxxx;  

Send SMS:  
AT+CMGF=1  
AT+CMGS=”09xxxxxxxx”  
  
Get APN: at+cgdcont?  
  
Add APN: at+cgdcont=1,"IP","INTERNET"  
  
PDP status: AT+CGACT?  
  
Activate/deactive PDP: AT+CGACT=1,1  
  
Get IccID: at+crsm=176,12258,0,0,10  
  
Check Pin: AT+CPIN?  
  
Input PIN: AT+CPIN="****"  
  
Network Selection: at+cops?  
