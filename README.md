'How I Met Your Temperature' (HIMYT) es una prueba de concepto de un regulador de aire acondicionado por medio de infrarrojos con seis módulos ESP8266 (ESP-01) y cinco sensores DHT11.

El nombre del proyecto hace referencia en la famosa serie de televisión [How I Met Your Mother](https://es.wikipedia.org/wiki/How_I_Met_Your_Mother) porque se quería tener nombres diferenciados para cada uno de los sensores y cierta persona me recordó esta serie. Además, como se va a usar MQTT para el intercambio de mensajes, viene muy bien tener cinco 'clients' con nombres bien diferenciados (Ted, Marshall, Lily, Robin y Barney) y un 'broker' al que todos mandan información (McLaren's). Este último, se comportará como un punto [único de fallo](https://es.wikipedia.org/wiki/Single_point_of_failure) porque se encargará de muchas labores:
* La gestión de la comunicación inalámbrica puesto que es el AP.
* La gestión de publicación/suscripción de mensajes puesto que es el 'broker' de MQTT.
* Almacenar las últimas seis medidas de temperatura y humedad de cada personaje.
* Cada cierto tiempo, si hay suficientes medidas, procesarlas para unificarlas con una media y suavizar los picos de lecturas erróneas.
* Cada cierto tiempo, si se han podido procesar las medidas de algún personaje, se establecerá la medida de temperatura y humedad oficial de McLaren's utilizando la media de los personajes. Además, también se publicarán en dos 'topic'.
* Cada cierto tiempo, si hay temperatura y humedad oficial, se calculará el [índice de temperatura-humedad](https://es.wikipedia.org/wiki/%C3%8Dndice_de_temperatura-humedad) y se publicará en un 'topic'.
* Cada cierto tiempo, si se ha calculado el índice de temperatura-humedad, se decidirá cuál debe ser el comportamiento del aire acondicionado. En este caso, se calcula el modo a utilizar, la temperatura y la velocidad del ventilador y se publican en tres 'topic'. Se han estableciendo cinco rangos con el objetivo de intentar adaptarse al [INSST](https://www.insst.es/documents/94886/327064/ntp_501.pdf/24b8f22e-7ce7-43c7-b992-f79d969a9d77) (aunque no hay nada oficial):
  * Menor que 67: Modo calentar, temperatura 25 ºC y ventilador a nivel 3.
  * De 67 a 69: Modo calentar, temperatura 23 ºC y ventilador a nivel 2.
  * De 69 a 71: Modo automático, temperatura 23 ºC y ventilador a nivel 1.
  * De 71 a 73: Modo enfriar, temperatura 23 ºC y ventilador a nivel 2.
  * Mayor que 73: Modo enfriar, temperatura 21 ºC y ventilador a nivel 3.
* Cada cierto tiempo, si se ha establecido el comportamiento del acondicionado y varia con respecto del anterior, se enviará la señar por infrarrojos.
* Finalmente, se expondrá la información del estado actual del sistema por medio del puerto serie. En este proyecto también se incluye en la carpeta 'Reader' el código necesario para leer en un equipo esta información y el código necesario para usar un Arduino Uno con una pantalla LiquidCrystal para mostrar dicha información.

Obviamente, al ser una prueba de concepto, se trata de un proyecto que no se piensa continuar y se expone en Github con licencia [MIT](https://github.com/r4mos/HIMYT/blob/master/LICENSE) por si puede servir de ayuda a cualquiera. No obstante, se aceptan colaboraciones.

## Materiales
* Bastantes cables.
* 1 USB a ESP8266 01 serial, que necesitaremos para programar los módulos y para visualizar el estado del Pub con el programa hecho en Python.
* 6 ESP8266 ESP-01, cinco para los personajes y uno para el Pub.
* 5 módulos transceptores DHT11 ESP-01, para los cinco personajes.
* 5 baterías Lipo 3.7V 380 mAh para alimentar los cinco personajes.
* 1 led infrarojo, 1 transistor 2n2222 y 1 resistencia de 10-100 Ohm para conectar al Pub y poder controlar el aire acondicionado.
* (Opcional) 1 Arduino Uno, 1 pantalla LiquidCrystal, 1 resistencia de 220 Ohm, 1 potenciómetro de 10k Ohm y un transformador de 12 V.

## Recordatorios
* Para programar los ESP8266 ESP-01 es necesario puentear el pin 3 (GPIO0) con masa en el momento que se inicia.
* Se debería cambiar la contraseña del Wifi en 'Characters.ino' y 'MacLaren's Pub.ino' y no compartirla. Al fin y al cabo es el único punto de seguridad que tiene este proyecto.
* Para grabar diferentes personajes hay que modificar en 'Characters.ino' la definición MQTT_ID por sus posibles cinco valores: 'MQTT_TED_CHARACTER', 'MQTT_MARSHALL_CHARACTER', 'MQTT_LILY_CHARACTER', 'MQTT_ROBIN_CHARACTER' o 'MQTT_BARNEY_CHARACTER'.
* Antes de flashear 'MacLaren's Pub.ino' hay que modificar la opción 'lwip Variant' tal y como indica la documentaicón de [uMQTTBroker](https://github.com/martin-ger/uMQTTBroker). De 'v2 Lower Memory' a '1.4 High Bandwidth'.
* Para conectar el emisor de infrarrojos hay que seguir el esquema de la [documentación oficial](https://github.com/crankyoldgit/IRremoteESP8266/wiki/ESP01-Send-&-Receive-Circuit) pero usar el pin 2 (GPIO2) en lugar del pin 4 (RXD). En la carpeta Util se puede ver el pinout correcto.
* Para conectar la pantalla LiquidCrystal se han usado los pines del [tutorial](http://www.arduino.cc/en/Tutorial/LiquidCrystalSerialDisplay) (disponibles en la carpeta Util) y después se ha conectado el ESP8266 ESP-01:
  * Del pin 1 (GND) al pin GND analógico.
  * Del pin 4 (RXD) al pin 1 (TX).
  * Del pin 5 (TXD) al pin 0 (RX).
  * Del pin 8 (VCC) al 3.3 V analógico.

## Enlaces
* [ESP8266WiFi](https://github.com/esp8266/Arduino/libraries/ESP8266WiFi/)
* [DHTesp](https://github.com/beegee-tokyo/DHTesp)
* [PubSubClient](https://github.com/knolleary/pubsubclient)
* [uMQTTBroker](https://github.com/martin-ger/uMQTTBroker)
* [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266)
* [LiquidCrystalSerialDisplay](http://www.arduino.cc/en/Tutorial/LiquidCrystalSerialDisplay)