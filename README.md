                  SISTEMA CONTRA INCENDIOS

Este sistema nos ayudara a detectar a tiempo ya sea incendios, 
altas temperaturas o incluso fugas de gasescomo lo puden ser 
los siguientes:
  -LPG
  -CO
  -HUMO (combinacion de gases)




                    ALARMAS(Disparadores)
 
- alarma sonora (buzzer)
- alarma visual (led y campana en lcd integrado)
- sistema de apoyo a 110V
-----Posible conexion para alarma por email----




                INTRUCCIONES PARA SU USO LOCAL:

1- instalar xampp con la version mas actualizada
2- crear una base de datos con las siguientes tablas:
    -id      [int]     (en autoincremento y como primary key)
    -alarma  [boolean]
    -co      [int]
    -lpg     [int]
    -smoke   [int]
    -temp    [float]
    -date_h  [timestamp] (fecha y hora actual)
2- crear una carpeta dentro de la carpeta htdocs
3- ingresar los siguientes archivos:
    -"connetion.php"
    -"index.html"
    -"index.php"
    -"package-lock.json"
    -"save.php"
4- ingresar a la carpeta creada, la carpeta llamada "node_modules"
5- modificar los siguientes parametros del archivos "Codigo_ESP.ino"
    -SSID (nombre de tu red wifi)
    -PASS (contraseña de tu red wifi)
    -host (IPv4)
6- cargar codigo a arduino uno
7- cargar codigo modificado a NodeMCU ESP8266 LUA