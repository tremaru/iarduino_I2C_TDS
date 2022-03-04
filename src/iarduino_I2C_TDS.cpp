#include "iarduino_I2C_TDS.h"																									//
																																//
//		Инициализация модуля:																									//	Возвращаемое значение: результат инициализации.
bool	iarduino_I2C_TDS::begin				(void){																				//	Параметр: отсутствует
		//	Инициируем работу с шиной I2C:																						//
			objI2C->begin(100);																									//	Инициируем передачу данных по шине I2C на скорости 100 кГц.
		//	Если адрес не указан, то ищим модуль на шине I2C:																	//
			if(valAddrTemp==0){																									//
				for(int i=1; i<127; i++){																						//	Проходим по всем адресам на шине I2C
					if( objI2C->checkAddress(i)											){	valAddr=i; delay(2);				//	Если на шине I2C есть устройство с адресом i, то используем этот адрес для проверки найденного модуля...
					if(_readBytes(REG_MODEL,4)											){										//	Читаем 4 байта начиная с регистра «REG_MODEL» в массив «data».
					if( data[0]     == DEF_MODEL_TDS									){										//	Если у модуля с адресом i в регистре «MODEL»   (data[0]) хранится значение DEF_MODEL_TDS, то ...
					if((data[2]>>1) == i                 || data[2] == 0xFF				){										//	Если у модуля с адресом i в регистре «ADDRESS» (data[2]) хранится значение i (адрес+младший бит) или 0xFF (адрес не задавался), то ...
					if( data[3]     == DEF_CHIP_ID_FLASH || data[3] == DEF_CHIP_ID_METRO){										//	Если у модуля с адресом i в регистре «CHIP_ID» (data[3]) хранится значение DEF_CHIP_ID_FLASH (идентификатор модулей Flash), или DEF_CHIP_ID_METRO (идентификатор модулей Metro), то ...
						valAddrTemp=i; i=128;																					//	Считаем что модуль обнаружен, сохраняем значение i как найденный адрес и выходим из цикла.
					}}}}}																										//
				}																												//
			}																													//
		//	Если модуль не найден, то возвращаем ошибку инициализации:															//
			if( valAddrTemp == 0														){	valAddr=0; return false;}			//
		//	Проверяем наличие модуля на шине I2C:																				//
			if( objI2C->checkAddress(valAddrTemp) == false								){	valAddr=0; return false;}			//	Если на шине I2C нет устройств с адресом valAddrTemp, то возвращаем ошибку инициализации
			valAddr=valAddrTemp;																								//	Сохраняем адрес модуля на шине I2C.
			delay(5);																											//
		//	Проверяем значения регистров модуля:																				//
			if(_readBytes(REG_MODEL,4)==false											){	valAddr=0; return false;}			//	Если не удалось прочитать 4 байта в массив «data» из модуля начиная с регистра «REG_MODEL», то возвращаем ошибку инициализации.
			if( data[0]     != DEF_MODEL_TDS											){	valAddr=0; return false;}			//	Если значение  регистра «MODEL»   (data[0]) не совпадает со значением DEF_MODEL_TDS, то возвращаем ошибку инициализации.
			if((data[2]>>1) != valAddrTemp       && data[2] !=0xFF						){	valAddr=0; return false;}			//	Если значение  регистра «ADDRESS» (data[2]) не совпадает с адресом модуля и не совпадает со значением 0xFF, то возвращаем ошибку инициализации.
			if( data[3]     != DEF_CHIP_ID_FLASH && data[3] != DEF_CHIP_ID_METRO		){	valAddr=0; return false;}			//	Если значение  регистра «CHIP_ID» (data[3]) не совпадает со значением DEF_CHIP_ID_FLASH и DEF_CHIP_ID_METRO, то возвращаем ошибку инициализации.
			valVers=data[1];																									//	Сохраняем байт регистра «VERSION» (data[1]) в переменую «valVers».
		//	Перезагружаем модуль устанавливая его регистры в значение по умолчанию:												//
			reset();																											//	Выполняем программную перезагрузку.
			delay(5);																											//
			return true;																										//	Возвращаем флаг успешной инициализаии.
}																																//
																																//
//		Перезагрузка модуля:																									//	Возвращаемое значение:	результат перезагрузки.
bool	iarduino_I2C_TDS::reset				(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Устанавливаем бит перезагрузки:																					//
				if(_readBytes(REG_BITS_0,1)==false){return false;}																//	Читаем 1 байт регистра «BITS_0» в массив «data».
				data[0] |= 0b10000000;																							//	Устанавливаем бит «SET_RESET»
				if(_writeBytes(REG_BITS_0,1)==false){return false;}																//	Записываем 1 байт в регистр «BITS_0» из массива «data».
			//	Ждём установки флага завершения перезагрузки:																	//
				do{ if(_readBytes(REG_FLAGS_0,1)==false){return false;} }														//	Читаем 1 байт регистра «REG_FLAGS_0» в массив «data».
				while( (data[0]&0b10000000) == 0);																				//	Повторяем чтение пока не установится флаг «FLG_RESET».
				return true;																									//
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Смена адреса модуля:																									//	Возвращаемое значение:	резульат смены адреса.
bool	iarduino_I2C_TDS::changeAddress		(uint8_t newAddr){																	//	Параметр:				newAddr - новый адрес модуля (0x07 < адрес < 0x7F).
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Проверяем новый адрес:																							//
				if(newAddr>0x7F){newAddr>>=1;}																					//	Корректируем адрес, если он указан с учётом бита RW.
				if(newAddr==0x00 || newAddr==0x7F){return false;}																//	Запрещаем устанавливать адрес 0x00 и 0x7F.
			//	Записываем новый адрес:																							//
				if(_readBytes(REG_BITS_0,1)==false){return false;}																//	Читаем 1 байт регистра «BITS_0» в массив «data».
				data[0] |= 0b00000010;																							//	Устанавливаем бит «SAVE_ADR_EN»
				if(_writeBytes(REG_BITS_0,1)==false){return false;}																//	Записываем 1 байт в регистр «BITS_0» из массива «data».
				data[0] = (newAddr<<1)|0x01;																					//	Готовим новый адрес к записи в модуль, установив бит «SAVE_FLASH».
				if(_writeBytes(REG_ADDRESS,1)==false){return false;}															//	Записываем 1 байт в регистр «ADDRESS» из массива «data».
				delay(200);																										//	Даём более чем достаточное время для применения модулем нового адреса.
			//	Проверяем наличие модуля с новым адресом на шине I2C:															//
				if(objI2C->checkAddress(newAddr)==false){return false;}															//	Если на шине I2C нет модуля с адресом newAddr, то возвращаем ошибку.
				valAddr     = newAddr;																							//	Сохраняем новый адрес как текущий.
				valAddrTemp = newAddr;																							//	Сохраняем новый адрес как указанный.
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Получение флага наличия подтяжки линий шины I2C:																		//	Возвращаемое значение:	флаг наличия подтяжки линий шины I2C.
bool	iarduino_I2C_TDS::getPullI2C		(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Считываем первые два регистра: регистр флагов и регистр битов:													//
				if(_readBytes(REG_FLAGS_0,2)==false ){ return false; }															//	Читаем 2 байта начиная с регистра «REG_FLAGS_0» в массив «data».
			//	Проверяем поддерживает ли модуль управление подтяжкой линий шины I2C:											//
				if( (data[0] & 0b00000100) == false ){ return false; }															//	Если флаг «FLG_I2C_UP» регистра «REG_FLAGS_0» сброшен, значит модуль не поддерживает управление подтяжкой линий шины I2C.
			//	Проверяем установлена ли подтяжка линий шины I2C:																//
				if( (data[1] & 0b00000100) == false ){ return false; }															//	Если бит  «SET_I2C_UP» регистра «REG_BITS_0»  сброшен, значит подтяжка линий шины I2C не установлена.
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Установка подтяжки линий шины I2C:																						//	Возвращаемое значение:	результат установки подтяжки линий шины I2C.
bool	iarduino_I2C_TDS::setPullI2C		(bool f){																			//	Параметр:				флаг подтяжки линий шины I2C.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Считываем первые два регистра: регистр флагов и регистр битов:													//
				if(_readBytes(REG_FLAGS_0,2)==false ){ return false; }															//	Читаем 2 байта начиная с регистра «REG_FLAGS_0» в массив «data».
			//	Проверяем поддерживает ли модуль управление подтяжкой линий шины I2C:											//
				if( (data[0] & 0b00000100) == false ){ return false; }															//	Если флаг «FLG_I2C_UP» регистра «REG_FLAGS_0» сброшен, значит модуль не поддерживает управление подтяжкой линий шины I2C.
			//	Устанавливаем или сбрасываем бит включения подтяжки линий шины I2C:												//
				if(f){ data[0] = (data[1] |  0b00000100); }																		//	Если флаг «f» установлен, то копируем значение из 1 в 0 элемент массива «data» установив бит «SET_I2C_UP».
				else { data[0] = (data[1] & ~0b00000100); }																		//	Если флаг «f» сброшен   , то копируем значение из 1 в 0 элемент массива «data» сбросив   бит «SET_I2C_UP».
			//	Сохраняем получившееся значение в регистр «REG_BITS_0»:															//
				if(_writeBytes(REG_BITS_0,1)==false ){ return false; }															//	Записываем 1 байт в регистр «REG_BITS_0» из массива «data».
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Получение частоты переменного тока используемого для измерения электрического сопротивления жидкости:					//	Возвращаемое значение:	частота переменного тока от 50 до 5000 Гц.
uint16_t iarduino_I2C_TDS::getFrequency		(void				){																//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_FREQUENCY_L, 2);																				//	Читаем 2 байта из модуля начиная регистра «REG_TDS_FREQUENCY_L» в массив «data».
			//	Возвращаем результат:																							//
				return ((uint16_t)data[1]<<8) | data[0];																		//	Собираем результат из массива «data» и возвращаем полученное значение в Гц.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Установка частоты переменного тока используемого для измерения электрического сопротивления жидкости:					//	Возвращаемое значение:	результат установки частоты.
bool	iarduino_I2C_TDS::setFrequency		(uint16_t frequency){																//	Параметр:				частота от 50 до 5000 Гц.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Проверяем полученные данные:																					//
				if( (frequency<50) || (frequency>5000) ){ return false; }														//	Значение параметра должно находиться в диапазоне от 50 до 5000 Гц.
			//	Готовим данные для записи:																						//
				data[0] =  frequency     & 0x00FF;																				//	Устанавливаем младший байт значения «frequency» для регистра «REG_TDS_FREQUENCY_L».
				data[1] = (frequency>>8) & 0x00FF;																				//	Устанавливаем старший байт значения «frequency» для регистра «REG_TDS_FREQUENCY_H».
			//	Отправляем подготовленные данные в модуль:																		//
				if(_writeBytes(REG_TDS_FREQUENCY_L, 2)==false ){ return false; }												//	Записываем 2 байта из массива «data» в модуль, начиная с регистра «REG_TDS_FREQUENCY_L».
				delay(50);																										//	Даём время для сохранения данных в энергонезависимую память модуля.
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Запуск калибровки модуля:																								//	Возвращаемое значение:	результат запуска калибровки.
bool	iarduino_I2C_TDS::setCalibration	(uint8_t num, uint16_t tds){														//	Параметры:				num - стадия калибровки (1 или 2), tds - концентрация калибровочной жидкости от 0 до 10000 ppm.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Проверяем полученные данные:																					//
				if( (num<1) || (num>2) ){ return false; }																		//	Параметр «num» определяет стадию калибровки и может принимать значения 1 или 2.
				if(  tds>10000         ){ return false; }																		//	Параметр «tds» не должен превышать 10'000 ppm.
			//	Готовим данные для записи:																						//
				data[0] =  tds     & 0x00FF;																					//	Устанавливаем младший байт значения «tds» для регистра «REG_TDS_KNOWN_TDS».
				data[1] = (tds>>8) & 0x00FF;																					//	Устанавливаем старший байт значения «tds» для регистра «REG_TDS_KNOWN_TDS».
			//	Отправляем подготовленные данные в модуль:																		//
				if(_writeBytes(REG_TDS_KNOWN_TDS, 2)==false ){ return false; }													//	Записываем 2 байта из массива «data» в регистр «REG_TDS_KNOWN_TDS».
			//	Готовим данные для записи:																						//
				data[0] = num==1? TDS_BIT_CALC_1:TDS_BIT_CALC_2;																//	Устанавливаем бит запуска 1 или 2 стадии калибровки.
			//	Отправляем подготовленные данные в модуль:																		//
				if(_writeBytes(REG_TDS_CALIBRATION, 1)==false ){ return false; }												//	Записываем 1 байт из массива «data» в регистр «REG_TDS_CALIBRATION».
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Получение текущей стадии калибровки модуля:																				//	Возвращаемое значение:	стадия калибровки (0-нет, 1, 2)
uint8_t	iarduino_I2C_TDS::getCalibration	(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_CALIBRATION, 1);																				//	Читаем 1 байт из регистра «REG_TDS_CALIBRATION» в массив «data».
			//	Возвращаем результат:																							//
				if(data[0]&TDS_FLG_STATUS_1){return 1;}																			//	Выполняется 1 стадия калибровки.
				if(data[0]&TDS_FLG_STATUS_2){return 2;}																			//	Выполняется 2 стадия калибровки.
			}																													//	
			return 0;																											//	Калибровка не выполняется.
}																																//
																																//
//		Получение концентрации раствора для калибровки модуля с кнопки:															//	Возвращаемое значение:	количество растворённых твёрдых веществ в калибровочной жидкости.
uint16_t iarduino_I2C_TDS::getKnownTDS		(uint8_t num){																		//	Параметр:				стадия калибровки (1 или 2).
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Проверяем полученные данные:																					//
				if( (num<1) || (num>2) ){ return 0; }																			//	Параметр «num» определяет стадию калибровки и может принимать значения 1 или 2.
			//	Читаем данные из модуля:																						//
				_readBytes(num==1? REG_TDS_KNOWN_TDS_1:REG_TDS_KNOWN_TDS_2, 2);													//	Читаем 2 байта из регистров «REG_TDS_KNOWN_TDS_1» или «REG_TDS_KNOWN_TDS_2» в массив «data».
			//	Возвращаем результат:																							//
				uint16_t result = ((uint16_t)data[1]<<8) | data[0];																//	Собираем результат из массива «data» в переменную «result».
				return result;																									//	Возвращаем результат.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Установка концентрации раствора для калибровки модуля с кнопки:															//	Возвращаемое значение:	результат установки.
bool	iarduino_I2C_TDS::setKnownTDS		(uint8_t num, uint16_t tds){														//	Параметры:				num - стадия калибровки (1 или 2), tds - концентрация калибровочной жидкости от 0 до 10000 ppm.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Проверяем полученные данные:																					//
				if( (num<1) || (num>2) ){ return false; }																		//	Параметр «num» определяет стадию калибровки и может принимать значения 1 или 2.
				if(  tds>10000         ){ return false; }																		//	Параметр «tds» не должен превышать 10'000 ppm.
			//	Готовим данные для записи:																						//
				data[0] =  tds     & 0x00FF;																					//	Устанавливаем младший байт значения «tds».
				data[1] = (tds>>8) & 0x00FF;																					//	Устанавливаем старший байт значения «tds».
			//	Отправляем подготовленные данные в модуль:																		//
				if(_writeBytes(num==1? REG_TDS_KNOWN_TDS_1:REG_TDS_KNOWN_TDS_2, 2)==false ){ return false; }					//	Записываем 2 байта из массива «data» в регистр «REG_TDS_KNOWN_TDS_1» или «REG_TDS_KNOWN_TDS_2».
				delay(50);																										//	Даём время для сохранения данных в энергонезависимую память модуля.
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Получение множителя степенной функции:																					//	Возвращаемое значение:	множитель от 0,01 до 167'772,15.
float	iarduino_I2C_TDS::getKa				(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_Ka, 3);																						//	Читаем 3 байта из регистра «REG_TDS_Ka» в массив «data».
			//	Возвращаем результат:																							//
				int32_t result = ((uint32_t)(data[2]&0x7F)<<16) | ((uint32_t)data[1]<<8) | data[0];								//	Собираем результат из массива «data» в переменную «result».
				return ((float)result)/100.0f;																					//	Возвращаем результат, преобразовав значение из сотых долей.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Установка множителя степенной функции:																					//	Возвращаемое значение:	результат установки.
bool	iarduino_I2C_TDS::setKa				(float val_Ka){																		//	Параметр:				значение множителя от 0,01 до 167'772,15.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Проверяем полученные данные:																					//
				if( (val_Ka<0.01f) || (val_Ka>167772.15f) ){ return false; }													//
			//	Готовим данные для записи:																						//
				uint32_t result = (uint32_t)(val_Ka*100.0f);																	//	Преобразуем результат в сотые доли.
				data[0] =  result      & 0x000000FF;																			//	Устанавливаем младший байт значения «result».
				data[1] = (result>> 8) & 0x000000FF;																			//	Устанавливаем средний байт значения «result».
				data[2] = (result>>16) & 0x000000FF;																			//	Устанавливаем старший байт значения «result».
			//	Отправляем подготовленные данные в модуль:																		//
				if(_writeBytes(REG_TDS_Ka, 3)==false ){ return false; }															//	Записываем 3 байта из массива «data» в регистр «REG_TDS_Ka».
				delay(50);																										//	Даём время для сохранения данных в энергонезависимую память модуля.
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Получение степени степенной функции:																					//	Возвращаемое значение:	степень степенной функции от -0,01 до -65,535.
float	iarduino_I2C_TDS::getKb				(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_Kb, 2);																						//	Читаем 2 байта из регистра «REG_TDS_Kb» в массив «data».
			//	Возвращаем результат:																							//
				uint16_t result = ((uint16_t)data[1]<<8) | data[0];																//	Собираем результат из массива «data» в переменную «result».
				return ((float)result)/-1000.0f;																				//	Возвращаем результат, преобразовав значение из тысячных долей и добавив отрицательный знак.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Установка степени степенной функции:																					//	Возвращаемое значение:	результат установки.
bool	iarduino_I2C_TDS::setKb				(float val_Kb){																		//	Параметр:				степень степенной функции от -0,01 до -65,535.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Проверяем полученные данные:																					//
				if(  val_Kb<0 ){ val_Kb*=-1; }																					//	Избавляемся от знака, так как записывается модуль.
				if( (val_Kb<0.01f) || (val_Kb>65.535f) ){ return false; }														//
			//	Готовим данные для записи:																						//
				uint16_t result = (uint16_t)(val_Kb*1000.0f);																	//	Преобразуем результат в тысячные доли.
				data[0] =  result     & 0x00FF;																					//	Устанавливаем младший байт значения «result».
				data[1] = (result>>8) & 0x00FF;																					//	Устанавливаем старший байт значения «result».
			//	Отправляем подготовленные данные в модуль:																		//
				if(_writeBytes(REG_TDS_Kb, 2)==false ){ return false; }															//	Записываем 2 байта из массива «data» в регистр «REG_TDS_Kb».
				delay(50);																										//	Даём время для сохранения данных в энергонезависимую память модуля.
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Получение температурного коэффициента жидкости:																			//	Возвращаемое значение:	температурный коэффициент жидкости от 0 до 6,5535.
float	iarduino_I2C_TDS::getKt				(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_Kt, 2);																						//	Читаем 2 байта из регистра «REG_TDS_Kt» в массив «data».
			//	Возвращаем результат:																							//
				uint16_t result = ((uint16_t)data[1]<<8) | data[0];																//	Собираем результат из массива «data» в переменную «result».
				return ((float)result)/10000.0f;																				//	Возвращаем результат, преобразовав значение из десятитысячных долей.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Установка температурного коэффициента жидкости:																			//	Возвращаемое значение:	результат установки.
bool	iarduino_I2C_TDS::setKt				(float val_Kt){																		//	Параметр:				температурный коэффициент жидкости от 0 до 6,5535.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Проверяем полученные данные:																					//
				if( (val_Kt<0) || (val_Kt>6.5535f) ){ return false; }															//
			//	Готовим данные для записи:																						//
				uint16_t result = (uint16_t)(val_Kt*10000.0f);																	//	Преобразуем результат в десятитысячные доли.
				data[0] =  result     & 0x00FF;																					//	Устанавливаем младший байт значения «result».
				data[1] = (result>>8) & 0x00FF;																					//	Устанавливаем старший байт значения «result».
			//	Отправляем подготовленные данные в модуль:																		//
				if(_writeBytes(REG_TDS_Kt, 2)==false ){ return false; }															//	Записываем 2 байта из массива «data» в регистр «REG_TDS_Kt».
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Получение коэффициента пересчёта:																						//	Возвращаемое значение:	коэффициент пересчёта от 0,01 до 2,55.
float	iarduino_I2C_TDS::getKp				(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_Kp, 1);																						//	Читаем 1 байт из регистра «REG_TDS_Kp» в массив «data».
			//	Возвращаем результат:																							//
				return ((float)data[0])/100.0f;																					//	Возвращаем результат, преобразовав значение из сотых долей.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Установка коэффициента пересчёта:																						//	Возвращаемое значение:	результат установки.
bool	iarduino_I2C_TDS::setKp				(float val_Kp){																		//	Параметр:				коэффициент пересчёта от 0,01 до 2,55.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Проверяем полученные данные:																					//
				if( (val_Kp<0.01) || (val_Kp>2.55f) ){ return false; }															//
			//	Готовим данные для записи:																						//
				data[0] = (uint8_t)(val_Kp*100.0f);																				//	Преобразуем результат в сотые доли.
			//	Отправляем подготовленные данные в модуль:																		//
				if(_writeBytes(REG_TDS_Kp, 1)==false ){ return false; }															//	Записываем 1 байт из массива «data» в регистр «REG_TDS_Kp».
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Установка реальной температуры жидкости:																				//	Возвращаемое значение:	результат установки.
bool	iarduino_I2C_TDS::set_t				(float val_t){																		//	Параметр:				реальная температура жидкости от 0 до 63,75 °С.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Проверяем полученные данные:																					//
				if( (val_t<0) || (val_t>63.75f) ){ return false; }																//
			//	Готовим данные для записи:																						//
				data[0] = (uint8_t)(val_t*4.0f);																				//	Преобразуем результат в целое число с эквивалентным шагом 0.25°С.
			//	Отправляем подготовленные данные в модуль:																		//
				if(_writeBytes(REG_TDS_t, 1)==false ){ return false; }															//	Записываем 1 байт из массива «data» в регистр «REG_TDS_t».
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Получение опорной температуры жидкости:																					//	Возвращаемое значение:	опорная температура от 0 до 63,75 °С.
float	iarduino_I2C_TDS::get_T				(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_T, 1);																						//	Читаем 1 байт из регистра «REG_TDS_T» в массив «data».
			//	Возвращаем результат:																							//
				return ((float)data[0])/4.0f;																					//	Возвращаем результат, преобразовав значение из целого числа с эквивалентным шагом 0.25°С.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Установка опорной температуры жидкости:																					//	Возвращаемое значение:	результат установки.
bool	iarduino_I2C_TDS::set_T				(float val_T){																		//	Параметр:				опорная температура от 0 до 63,75 °С.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Проверяем полученные данные:																					//
				if( (val_T<0) || (val_T>63.75f) ){ return false; }																//
			//	Готовим данные для записи:																						//
				data[0] = (uint8_t)(val_T*4.0f);																				//	Преобразуем результат в целое число с эквивалентным шагом 0.25°С.
			//	Отправляем подготовленные данные в модуль:																		//
				if(_writeBytes(REG_TDS_T, 1)==false ){ return false; }															//	Записываем 1 байт из массива «data» в регистр «REG_TDS_T».
				return true;																									//	Возвращаем флаг успеха.
			}else{																												//	Иначе, если модуль не инициализирован, то ...
				return false;																									//	Возвращаем ошибку.
			}																													//
}																																//
																																//
//		Получение общего измеренного сопротивления:																				//	Возвращаемое значение:	сопротивление от 0 до 10'000'000 Ом.
uint32_t iarduino_I2C_TDS::getRo			(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_Ro, 3);																						//	Читаем 3 байта из регистра «REG_TDS_Ro» в массив «data».
			//	Возвращаем результат:																							//
				return ((uint32_t)data[2]<<16) | ((uint32_t)data[1]<<8) | data[0];												//	Собираем результат из массива «data» и возвращаем полученное значение в Ом.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Получение напряжения на выходе первого ОУ:																				//	Возвращаемое значение:	напряжение на выходе первого ОУ от 0 до 3.3000 В.
float	iarduino_I2C_TDS::getVout			(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_Vout, 2);																					//	Читаем 2 байта из регистра «REG_TDS_Vout» в массив «data».
			//	Возвращаем результат:																							//
				return (float)(((uint16_t)data[1]<<8) | data[0])/10000.0f;														//	Собираем результат из массива «data» и возвращаем полученное значение в В.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Получение измеренной удельной электрической проводимости:																//	Возвращаемое значение:	электропроводность от 0 до 65'535 мкСм/см.
uint16_t iarduino_I2C_TDS::get_S			(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_S, 2);																						//	Читаем 2 байта из регистра «REG_TDS_S» в массив «data».
			//	Возвращаем результат:																							//
				return ((uint16_t)data[1]<<8) | data[0];																		//	Собираем результат из массива «data» и возвращаем полученное значение в мкСм/см.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Получение приведённой удельной электрической проводимости:																//	Возвращаемое значение:	электропроводность от 0 до 65'535 мкСм/см.
uint16_t iarduino_I2C_TDS::getEC			(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_EC, 2);																						//	Читаем 2 байта из регистра «REG_TDS_EC» в массив «data».
			//	Возвращаем результат:																							//
				return ((uint16_t)data[1]<<8) | data[0];																		//	Собираем результат из массива «data» и возвращаем полученное значение в мкСм/см.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Получение количества растворённых твёрдых веществ в жидкости:															//	Возвращаемое значение:	концентрация раствора от 0 до 65'535 мг/л воды (ppm).
uint16_t iarduino_I2C_TDS::getTDS			(void){																				//	Параметр:				отсутствует.
			if(valAddr){																										//	Если модуль был инициализирован, то ...
			//	Читаем данные из модуля:																						//
				_readBytes(REG_TDS_TDS, 2);																						//	Читаем 2 байта из регистра «REG_TDS_TDS» в массив «data».
			//	Возвращаем результат:																							//
				return ((uint16_t)data[1]<<8) | data[0];																		//	Собираем результат из массива «data» и возвращаем полученное значение в мкСм/см.
			}																													//	
			return 0;																											//	Возвращаем ошибку.
}																																//
																																//
//		Чтение данных из регистров в массив data:																				//	Возвращаемое значение:	результат чтения (true/false).
bool	iarduino_I2C_TDS::_readBytes		(uint8_t reg, uint8_t sum){															//	Параметры:				reg - номер первого регистра, sum - количество читаемых байт.
			bool	result=false;																								//	Определяем флаг       для хранения результата чтения.
			uint8_t	sumtry=10;																									//	Определяем переменную для подсчёта количества оставшихся попыток чтения.
			do{	result = objI2C->readBytes(valAddr, reg, data, sum);															//	Считываем из модуля valAddr, начиная с регистра reg, в массив data, sum байт.
				sumtry--;	if(!result){delay(1);}																				//	Уменьшаем количество попыток чтения и устанавливаем задержку при неудаче.
			}	while		(!result && sumtry>0);																				//	Повторяем чтение если оно завершилось неудачей, но не более sumtry попыток.
			delayMicroseconds(500);																								//	Между пакетами необходимо выдерживать паузу.
			return result;																										//	Возвращаем результат чтения (true/false).
}																																//
																																//
//		Запись данных в регистры из массива data:																				//	Возвращаемое значение:	результат записи (true/false).
bool	iarduino_I2C_TDS::_writeBytes		(uint8_t reg, uint8_t sum, uint8_t num){											//	Параметры:				reg - номер первого регистра, sum - количество записываемых байт, num - номер первого элемента массива data.
			bool	result=false;																								//	Определяем флаг       для хранения результата записи.
			uint8_t	sumtry=10;																									//	Определяем переменную для подсчёта количества оставшихся попыток записи.
			do{	result = objI2C->writeBytes(valAddr, reg, &data[num], sum);														//	Записываем в модуль valAddr начиная с регистра reg, sum байи из массива data начиная с элемента num.
				sumtry--;	if(!result){delay(1);}																				//	Уменьшаем количество попыток записи и устанавливаем задержку при неудаче.
			}	while		(!result && sumtry>0);																				//	Повторяем запись если она завершилась неудачей, но не более sumtry попыток.
			delay(10);																											//	Ждём применения модулем записанных данных.
			return result;																										//	Возвращаем результат записи (true/false).
}																																//
																																//