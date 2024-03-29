// ПРИМЕР ПОЛУЧЕНИЯ ДАННЫХ ПО ШИНЕ I2C:           // * Строки со звёздочкой являются необязательными.
                                                  //
#include <Wire.h>                                 // * Подключаем библиотеку для работы с аппаратной шиной I2C.
#include <iarduino_I2C_TDS.h>                     //   Подключаем библиотеку для работы с TDS/EC-метром I2C-flash.
iarduino_I2C_TDS tds(0x09);                       //   Объявляем объект tds  для работы с функциями и методами библиотеки iarduino_I2C_TDS, указывая адрес модуля на шине I2C.
                                                  //   Если объявить объект без указания адреса (iarduino_I2C_TDS tds;), то адрес будет найден автоматически.
void setup(){                                     //
     delay(500);                                  // * Ждём завершение переходных процессов связанных с подачей питания.
     Serial.begin(9600);                          //   Инициируем работу с шиной UART для передачи данных в монитор последовательного порта на скорости 9600 бит/сек.
     tds.begin();                                 //   Инициируем работу с TDS/EC-метром I2C-flash.
}                                                 //
                                                  //
void loop(){                                      //
     tds.set_t(25.00f);                           //   Указываем текущую температуру жидкости.
     Serial.print("Ro="          );               //
     Serial.print(tds.getRo()    );               //   Выводим общее измеренное сопротивление.
     Serial.print("Ом, S="       );               //
     Serial.print(tds.get_S()    );               //   Выводим измеренную удельную электропроводность жидкости.
     Serial.print("мСм/см, EC="  );               //
     Serial.print(tds.getEC()    );               //   Выводим удельную электропроводность жидкости приведённую к опорной температуре.
     Serial.print("мСм/см, TDS=" );               //
     Serial.print(tds.getTDS()   );               //   Выводим количество растворённых твёрдых веществ в жидкости.
     Serial.print(" мг/л\r\n"    );               //
     delay(1000);                                 //
}                                                 //