// ПРИМЕР КАЛИБРОВКИ МОДУЛЯ БЕЗ НАЖАТИЯ НА КНОПКУ:                      // * Строки со звёздочкой являются необязательными.
                                                                        //
//   Функция getCalibration() возвращает стадию калибровки:             //
//   0-нет, 1-стадия, 2-стадия, 3-калибровка завершена с ошибкой.       //
//   В примере setCalibrationMini функция вызывается без параметра.     //
//   Если функцию вызвать с параметром (как в данном примере), то       //
//   функция будет отправлять в параметр дополнительную информацию:     //
//   0-нет действий, 1-поиск частоты, 2-накопление данных, 3-ожидание.  //
                                                                        //
#include <Wire.h>                                                       // * Подключаем библиотеку для работы с аппаратной шиной I2C.
#include <iarduino_I2C_TDS.h>                                           //   Подключаем библиотеку для работы с TDS/EC-метром I2C-flash.
iarduino_I2C_TDS tds(0x09);                                             //   Объявляем объект tds  для работы с функциями и методами библиотеки iarduino_I2C_TDS, указывая адрес модуля на шине I2C.
                                                                        //   Если объявить объект без указания адреса (iarduino_I2C_TDS tds;), то адрес будет найден автоматически.
float val_t=25.0;                                                       //   Температура калибровочных жидкостей.
uint8_t i=0, j=0;                                                       //
                                                                        //
void setup(){                                                           //
     delay(500);                                                        // * Ждём завершение переходных процессов связанных с подачей питания.
     Serial.begin(9600);                                                //   Инициируем работу с шиной UART для передачи данных в монитор последовательного порта на скорости 9600 бит/сек.
     tds.begin();                                                       //   Инициируем работу с TDS/EC-метром I2C-flash.
     tds.set_t(val_t);                                                  //   Указываем температуру жидкости в °С.
//   Информируем о начале калибровки:                                   //
     Serial.println("----------------------------------------");        //
     Serial.println("ОПУСТИТЕ ДАТЧИК В 1 РАСТВОР (500 ppm).");          //
     Serial.println("Калибровка начнётся через 1 минуту..." );          // *
     delay(60000);                                                      //   Ждём 1 минуту.
//   Запускаем 1 стадию калибровки:                                     //
     tds.setCalibration(1, 500 );                                       //   Выполняем 1 стадию калибровки указав известную концентрацию 1 раствора (в примере 500ppm ).
     Serial.println("ВЫПОЛНЯЕТСЯ 1 СТАДИЯ КАЛИБРОВКИ..." );             //
     Serial.println("Процесс займёт примерно 1 минуту...");             // *
//   Ждём завершение 1 стадии калибровки:                               //
     i=0, j=0;                                                          //
     while( tds.getCalibration(&j)==1 ){                                //   Если выполняется 1 стадия калибровки ...
         if( i!=j ){ i=j;                                               // * Это условие позволяет выводить строку только 1 раз, при изменении значения j.
             if( j==0 ){ Serial.println("- Нет действий..."     ); }    // * Эта строка не может появится в процессе выполнения 1 или 2 стадии калибровки.
             if( j==1 ){ Serial.println("- Поиск частоты..."    ); }    // * Эта строка появится только у модулей версии 6 и выше.
             if( j==2 ){ Serial.println("- Накопление данных..."); }    // * Эта строка появится только у модулей версии 6 и выше.
             if( j==3 ){ Serial.println("- Ожидание 2 стадии..."); }    // * Эта строка не может появится в процессе выполнения 1 или 2 стадии калибровки.
         }                                                              // * 
         delay(500);                                                    //   Слишком частые запросы к модулю в процессе калибровки могут ухудшить результат, по этому добавляем задержку.
     }                                                                  //
     Serial.println("1 СТАДИЯ КАЛИБРОВКИ ЗАВЕРШЕНА!" );                 //
     Serial.println("----------------------------------------");        //
     if( tds.getCalibration(&j)==0 ){                                   // * Если калибровка не выполняется...
         if( j==0 ){ Serial.println("- Нет действий..."     ); }        // * Эта строка появится только у модулей версии 6 и выше.
         if( j==1 ){ Serial.println("- Поиск частоты..."    ); }        // * Эта строка не может появится если калибровка не выполняется.
         if( j==2 ){ Serial.println("- Накопление данных..."); }        // * Эта строка не может появится если калибровка не выполняется.
         if( j==3 ){ Serial.println("- Ожидание 2 стадии..."); }        // * Эта строка появится только у модулей версии 6 и выше.
     }                                                                  // *
//   Информируем о необходимости смены калибровочных жидкостей:         //
     Serial.println("СПОЛОСНИТЕ ДАТЧИК.");                              //
     Serial.println("У Вас есть 30 сек...");                            // *
     delay(30000);                                                      //   Ждём 30 секунд.
     Serial.println("----------------------------------------");        //
     Serial.println("ОПУСТИТЕ ДАТЧИК ВО 2 РАСТВОР (1500 ppm).");        //
     Serial.println("Калибровка продолжится через 1 минуту...");        // *
     delay(60000);                                                      //   Ждём 1 минуту.
//   Запускаем 2 стадию калибровки:                                     //
     tds.setCalibration(2, 1500 );                                      //   Выполняем 2 стадию калибровки указав известную концентрацию 2 раствора (в примере 1500ppm ).
     Serial.println("ВЫПОЛНЯЕТСЯ 2 СТАДИЯ КАЛИБРОВКИ..."  );            //
     Serial.println("Процесс займёт примерно 30 секунд...");            // *
//   Ждём завершение 2 стадии калибровки:                               //
     i=0, j=0;                                                          //
     while( tds.getCalibration(&j)==2 ){                                //   Если выполняется 2 стадия калибровки ...
         if( i!=j ){ i=j;                                               // * Это условие позволяет выводить строку только 1 раз, при изменении значения j.
             if( j==0 ){ Serial.println("- Нет действий..."     ); }    // * Эта строка не может появится в процессе выполнения 1 или 2 стадии калибровки.
             if( j==1 ){ Serial.println("- Поиск частоты..."    ); }    // * Эта строка не может появится в процессе выполнения 2 стадии калибровки.
             if( j==2 ){ Serial.println("- Накопление данных..."); }    // * Эта строка появится только у модулей версии 6 и выше.
             if( j==3 ){ Serial.println("- Ожидание 2 стадии..."); }    // * Эта строка не может появится в процессе выполнения 1 или 2 стадии калибровки.
         }                                                              // * 
         delay(500);                                                    //   Слишком частые запросы к модулю в процессе калибровки могут ухудшить результат, по этому добавляем задержку.
     }                                                                  //
     Serial.println("2 СТАДИЯ КАЛИБРОВКИ ЗАВЕРШЕНА!" );                 //
     Serial.println("----------------------------------------");        //
//   Информируем о результате калибровки:                               //
     i = tds.getCalibration(&j);                                        // * Получаем стадию калибровки i и дополнительную информацию по адресу &j.
     if( i==0 ){ Serial.println("КАЛИБРОВКА ВЫПОЛНЕНА УСПЕШНО!"); }     // *
     if( i==1 ){ Serial.println("ВЫПОЛНЯЕТСЯ 1 СТАДИЯ ..."     ); }     // * Эта строка не может появится после завершения 2 стадии калибровки.
     if( i==2 ){ Serial.println("ВЫПОЛНЯЕТСЯ 2 СТАДИЯ ..."     ); }     // * Эта строка не может появится после завершения 2 стадии калибровки.
     if( i==3 ){ Serial.println("ОШИБКА КАЛИБРОВКИ!"           ); }     // * Эта строка появится только у модулей версии 6 и выше.
     if( j==0 ){ Serial.println("- Нет действий..."            ); }     // * Эта строка появится только у модулей версии 6 и выше.
     if( j==1 ){ Serial.println("- Поиск частоты..."           ); }     // * Эта строка не может появится после завершения 2 стадии калибровки.
     if( j==2 ){ Serial.println("- Накопление данных..."       ); }     // * Эта строка не может появится после завершения 2 стадии калибровки.
     if( j==3 ){ Serial.println("- Ожидание 2 стадии..."       ); }     // * Эта строка не может появится после завершения 2 стадии калибровки.
     Serial.println("----------------------------------------");        //
}                                                                       //
                                                                        //
void loop(){                                                            //
     Serial.print((String) "TDS="+tds.getTDS()+"мг/л, ");               //   Выводим количество растворённых твёрдых веществ в жидкости.
     Serial.print((String) "EC=" +tds.getEC() +"мСм/см.\r\n");          //   Выводим удельную электропроводность приведённую к опорной температуре.
     delay(1000);                                                       //
}                                                                       //