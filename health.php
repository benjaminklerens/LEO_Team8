<!DOCTYPE html>
<html>
<head>
<title>LEO1</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0">
<meta name="apple-mobile-web-app-capable" content="yes">
</head><body><p style="font-family: "Lucida Grande", Verdana, Geneva; font-size: 12px;">
<b>Raspberry Helath Logging</b><br/>
<?php
echo ('Last Health Logs:<br/>');
for($i=1; $i <11; $i++){
    $timestampstring = shell_exec("sed '$i!d;q' /home/pi/logdata/health.txt | awk -F, '{print$1}'");
    #$timestamp =  intval($timestampstring ,10);
    $date = shell_exec("date -d @$imestampstring");
    $templog = shell_exec("sed '$i!d;q' /home/pi/logdata/health.txt | awk -F, '{print $2}'");
    $storlog = shell_exec("sed '$i!d;q' /home/pi/logdata/health.txt | awk -F, '{print $3}'");
    echo ($date." ".$templog." ".$storlog.'<br/>');
}
?>
</p>
</body>
</html>