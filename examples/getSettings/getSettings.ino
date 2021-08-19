// ПРИМЕР ПОЛУЧЕНИЯ НАСТРОЕК МОДУЛЯ:              // * Строки со звёздочкой являются необязательными.
                                                  //
#include <Wire.h>                                 // * Подключаем библиотеку для работы с аппаратной шиной I2C.
#include <iarduino_I2C_TDS.h>                     //   Подключаем библиотеку для работы с TDS/EC-метром I2C-flash.
iarduino_I2C_TDS tds(0x09);                       //   Объявляем объект tds  для работы с функциями и методами библиотеки iarduino_I2C_TDS, указывая адрес модуля на шине I2C.
                                                  //   Если объявить объект без указания адреса (iarduino_I2C_TDS tds;), то адрес будет найден автоматически.
void setup(){                                     //
     delay(500);                                  // * Ждём завершение переходных процессов связанных с подачей питания.
     Serial.begin(9600);                          //   Инициируем работу с шиной UART для передачи данных в монитор последовательного порта на скорости 9600 бит/сек.
     tds.begin();                                 //   Инициируем работу с TDS/EC-метром I2C-flash.
     Serial.print("F    = "         );            //
     Serial.print(tds.getFrequency());            //   Выводим частоту переменного тока используемую для измерений от 50 до 500 Гц.
     Serial.print("Гц.\r\nTDS1 = "  );            //
     Serial.print(tds.getKnownTDS(1));            //   Выводим требуемую концентрацию 1 раствора для калибровки модуля кнопкой от 0 до 10'000 ppm.
     Serial.print("ppm.\r\nTDS2 = " );            //
     Serial.print(tds.getKnownTDS(2));            //   Выводим требуемую концентрацию 2 раствора для калибровки модуля кнопкой от 0 до 10'000 ppm.
     Serial.print("ppm.\r\nKt   = " );            //
     Serial.print(tds.getKt()     ,4);            //   Выводим температурный коэффициент жидкости от 0,0000 до 6,5535.
     Serial.print("\r\nKp   = "     );            //
     Serial.print(tds.getKp()     ,2);            //   Выводим коэффициент пересчёта от 0,01 до 2,55.
     Serial.print("\r\nT    = "     );            //
     Serial.print(tds.get_T()     ,2);            //   Выводим опорную температуру жидкости от 0,00 до 63,75 °С.
     Serial.print("°С.\r\nRd   = "  );            //
     Serial.print(tds.getRd()     ,3);            //   Выводим общее сопротивление датчика от 0,000 до 16'777,215 Ом.
     Serial.print("Ом.\r\nKd   = "  );            //
     Serial.print(tds.getKd()     ,3);            //   Выводим постоянную датчика от 0,000 до 65,535 см^-1.
     Serial.print("см^-1.\r\n"      );            //
}                                                 //
                                                  //
void loop(){}                                     //