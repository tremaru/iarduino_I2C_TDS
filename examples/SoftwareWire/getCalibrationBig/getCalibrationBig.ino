// ПРИМЕР ИНФОРМИРУЕТ О ПРОЦЕССЕ КАЛИБРОВКИ КНОПКОЙ:                    //
                                                                        //
// TDS/EC-метр с щупом (Trema-модуль) Flash-I2C:                        //   https://iarduino.ru/shop/Sensory-Datchiki/bamper-s-9-datchikami-liniy-s-shagom-7mm-flash-i2c.html
// Информация о подключении модулей к шине I2C:                         //   https://wiki.iarduino.ru/page/i2c_connection/
// Информация о модуле и описание библиотеки:                           //   https://wiki.iarduino.ru/page/TDS-EC-i2c/
                                                                        //
// Функция getCalibration() возвращает стадию калибровки:               //
// 0-нет, 1-стадия, 2-стадия, 3-калибровка завершена с ошибкой.         //
// Если функцию вызвать с параметром (как в данном примере), то         //
// функция будет отправлять в параметр дополнительную информацию:       //
// 0-нет действий, 1-смена частоты, 2-накопление данных, 3-ожидание.    //
                                                                        //
#include <iarduino_I2C_Software.h>                                      // Подключаем библиотеку для работы с программной шиной I2C, до подключения библиотеки iarduino_I2C_TDS.
SoftTwoWire sWire(3,4);                                                 // Создаём объект программной шины I2C указав выводы которым будет назначена роль линий: SDA, SCL.
                                                                        //
#include <iarduino_I2C_TDS.h>                                           // Подключаем библиотеку для работы с TDS/EC-метром I2C-flash.
iarduino_I2C_TDS tds(0x09);                                             // Создаём объект tds  для работы с функциями и методами библиотеки iarduino_I2C_TDS, указывая адрес модуля на шине I2C.
                                                                        // Если объявить объект без указания адреса (iarduino_I2C_TDS tds;), то адрес будет найден автоматически.
uint8_t  now_cal, pre_cal;                                              // Стадия калибровки (now-текущая, pre-предыдущая).
uint8_t  now_sta, pre_sta;                                              // Статус калибровки (now-текущий, pre-предыдущий).
uint16_t now_frq, pre_frq;                                              // Частота           (now-текущая, pre-предыдущая).
                                                                        //
void setup(){                                                           //
     delay(500);                                                        // Ждём завершения переходных процессов.
     Serial.begin(9600);                                                // Инициируем передачу данных в монитор последовательного порта на скорости 9600 бит/сек.
//   Инициируем работу с модулем указав шину I2C:                       //
     tds.begin(&sWire);                                                 // Инициируем работу с TDS/EC-метром, указав ссылку на объект для работы с шиной I2C на которой находится модуль (по умолчанию &Wire).
//   Получаем текущую стадию калибровки и рабочую частоту:              //
     now_cal = tds.getCalibration(&now_sta);                            //
     now_frq = tds.getFrequency();                                      //
//   Выводим текущие настройки модуля:                                  //
     Serial.println( (String) "F    = "+tds.getFrequency()+"Гц."    );  // Частота смены полярности на выводах щупа.
     Serial.println( (String) "TDS1 = "+tds.getKnownTDS(1)+"ppm."   );  // TDS первой калибровочной жидкости.
     Serial.println( (String) "TDS2 = "+tds.getKnownTDS(2)+"ppm."   );  // TDS второй калибровочной жидкости.
     Serial.println( (String) "Kt   = "+String(tds.getKt(),4)       );  // Температурный коэффициент жидкости.
     Serial.println( (String) "Kp   = "+String(tds.getKp(),2)       );  // Коэффициент пересчёта.
     Serial.println( (String) "T    = "+String(tds.get_T(),2)+"°С." );  // Опорная температура жидкости.
     Serial.println( (String) "Ka   = "+String(tds.getKa(),1)       );  // Множитель степенной функции.
     Serial.println( (String) "Kb   = "+String(tds.getKb(),3)       );  // Степень степенной функции.
}                                                                       //
                                                                        //
void loop(){
     delay(1000);
//   Указываем реальную температуру жидкости:
     tds.set_t(25.00f);
//   Сохраняем текущие значения как предыдущие:
     pre_cal = now_cal;
     pre_sta = now_sta;
     pre_frq = now_frq;
//   Получаем текущую стадию калибровки и рабочую частоту:
     now_cal = tds.getCalibration(&now_sta);
     now_frq = tds.getFrequency();
//   Информируем о изменении стадии калибровки:
     if( pre_cal!=1 && now_cal==1 ){ Serial.println("Начинается 1 стадия калибровки"); }
     if( pre_cal!=2 && now_cal==2 ){ Serial.println("Начинается 2 стадия калибровки"); }
     if( pre_cal==1 && now_cal==0 ){ Serial.println("Завершена 1 стадия калибровки"); }
     if( pre_cal==2 && now_cal==0 ){ Serial.println("Завершена 2 стадия калибровки"); }
     if( pre_cal==1 && now_cal==3 ){ Serial.println("Первая стадия калибровки завершена с ошибкой"); }
     if( pre_cal==2 && now_cal==3 ){ Serial.println("Вторая стадия калибровки завершена с ошибкой"); }
//   Информируем о изменении статуса калибровки:
     if( pre_sta!=0 && now_sta==0 ){ Serial.println("- Нет действий"); }
     if( pre_sta!=1 && now_sta==1 ){ Serial.println("- Смена частоты"); }
     if( pre_sta!=2 && now_sta==2 ){ Serial.println("- Накопление данных..."); }
     if( pre_sta!=3 && now_sta==3 ){ Serial.println("- Ожидание 2 стадии..."); }
//   Информируем о изменении рабочей частоты:
     if( pre_frq    != now_frq    ){ Serial.println( (String) "- Частота изменена на "+now_frq+"Гц." ); }
//   Информируем о рассчитанных коэффициентах после успешного завершения калибровки:
     if( pre_cal==2 && now_cal==0 ){ Serial.println( (String) "Ka = " + String(tds.getKa(),1) + ", Kb = " + String(tds.getKb(),3) ); }
//   Выводим полученные значения в обычном режиме:
     if( now_cal==0 || now_cal==3 ){ Serial.println( (String) "F="+now_frq+"Гц,\tTDS="+tds.getTDS()+"мг/л,\tEC="+tds.getEC()+"мСм/см.\tVout="+String(tds.getVout(),4)+"В" ); }
}