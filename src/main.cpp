#include <Wire.h>

#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv;

// Настройки для тонкой регулировки
struct TapticSettings {
  uint8_t feedbackReg;      // Регистр 0x1A - основной контроль
  uint8_t controlReg;       // Регистр 0x1C - контроль формы сигнала
  uint8_t driveReg;         // Регистр 0x18 - уровень драйва
  uint8_t frequency;        // Частота (235 для iPhone 7)
  uint8_t effect;           // Номер эффекта
  uint8_t overdriveReg;     // Регистр 0x16 - контроль перегрузки
  uint8_t compensationReg;  // Регистр 0x17 - компенсация
};

TapticSettings currentSettings = {
    0xFA,  // feedbackReg
    0x5A,  // controlReg
    0x6E,  // driveReg
    235,   // frequency
    14,    // effect
    0x82,  // overdriveReg
    0x26   // compensationReg
};

bool directInputMode = false;
int currentParameter = 0;
String inputBuffer = "";

// --- Объявление функций
void processKeyInput(char cmd);
void processDirectInput(char cmd);
void startDirectInput();
void cancelDirectInput();
void finishDirectInput();
void printParameterName(int param);
void setParameterValue(int param, int value);
void applySettings();
void playEffect();
void printCurrentSettings();
void loadPreset(int preset);
void finishValueInput();

void setup() {
  Serial.begin(115200);
  Serial.println("Taptic Engine Fine Tuning");
  Serial.println("Режим клавиш: q/w g/h j/k d/f a/s l/; </>");
  Serial.println("Режим ввода: } - начать ввод, { - отмена");
  Serial.println("Пресеты: 1-мягкий, 2-средний, 3-сильный");
  Serial.println("Пробел - воспроизвести эффект");

  if (!drv.begin()) {
    Serial.println("DRV2605 not found");
    while (1);
  }

  applySettings();
  printCurrentSettings();
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    if (directInputMode) {
      processDirectInput(cmd);
    } else {
      processKeyInput(cmd);
    }
  }
}

void processDirectInput(char cmd) {
  // Сначала выводим символ для эха
  if (cmd != '\n' && cmd != '\r' && cmd != '{') {
    Serial.print(cmd);
  }

  if (cmd == '\n' || cmd == '\r') {  // Enter
    if (currentParameter == 0) {
      // Выбор параметра
      if (inputBuffer.length() > 0) {
        int param = inputBuffer.toInt();
        if (param >= 1 && param <= 7) {
          currentParameter = param;
          inputBuffer = "";
          Serial.println();
          printParameterName(param);
        } else {
          Serial.println("\nОшибка: неверный номер параметра (1-7)");
          inputBuffer = "";
        }
      }
    } else {
      // Завершение ввода значения - всегда вызываем finishValueInput()
      Serial.println();  // Переводим строку
      finishValueInput();
    }
    return;
  }

  if (cmd == '{') {  // { - отмена ввода и выход
    cancelDirectInput();
  } else if (isdigit(cmd)) {  // Цифры - добавляем в буфер
    inputBuffer += cmd;
  }
}

void printParameterName(int param) {
  Serial.println();
  Serial.println("╔══════════════════════════════════════════════════════════════╗");

  switch (param) {
    case 1:
      Serial.println("║                 РЕДАКТИРОВАНИЕ: FEEDBACK                     ║");
      Serial.print("║ Текущее значение: ");
      Serial.print(currentSettings.feedbackReg);

      if (currentSettings.feedbackReg < 10)
        Serial.print("  ");
      else if (currentSettings.feedbackReg < 100)
        Serial.print(" ");
      else if (currentSettings.feedbackReg > 99)
        Serial.print("");

      Serial.println("                                        ║");
      break;
    case 2:
      Serial.println("║                 РЕДАКТИРОВАНИЕ: OVERDRIVE                    ║");
      Serial.print("║ Текущее значение: ");
      Serial.print(currentSettings.overdriveReg);

      if (currentSettings.overdriveReg < 10)
        Serial.print("  ");
      else if (currentSettings.overdriveReg < 100)
        Serial.print(" ");
      else if (currentSettings.overdriveReg > 99)
        Serial.print("");

      Serial.println("                                        ║");
      break;
    case 3:
      Serial.println("║                 РЕДАКТИРОВАНИЕ: COMPENSATION                 ║");
      Serial.print("║ Текущее значение: ");
      Serial.print(currentSettings.compensationReg);

      if (currentSettings.compensationReg < 10)
        Serial.print("  ");
      else if (currentSettings.compensationReg < 100)
        Serial.print(" ");
      else if (currentSettings.compensationReg > 99)
        Serial.print("");

      Serial.println("                                        ║");
      break;
    case 4:
      Serial.println("║                 РЕДАКТИРОВАНИЕ: DRIVE                        ║");
      Serial.print("║ Текущее значение: ");
      Serial.print(currentSettings.driveReg);

      if (currentSettings.driveReg < 10)
        Serial.print("  ");
      else if (currentSettings.driveReg < 100)
        Serial.print(" ");
      else if (currentSettings.driveReg > 99)
        Serial.print("");

      Serial.println("                                        ║");
      break;
    case 5:
      Serial.println("║                 РЕДАКТИРОВАНИЕ: CONTROL                      ║");
      Serial.print("║ Текущее значение: ");
      Serial.print(currentSettings.controlReg);

      if (currentSettings.controlReg < 10)
        Serial.print("  ");
      else if (currentSettings.controlReg < 100)
        Serial.print(" ");
      else if (currentSettings.controlReg > 99)
        Serial.print("");

      Serial.println("                                        ║");
      break;
    case 6:
      Serial.println("║                 РЕДАКТИРОВАНИЕ: FREQUENCY                    ║");
      Serial.print("║ Текущее значение: ");
      Serial.print(currentSettings.frequency);
      Serial.println(" Hz                                     ║");
      break;
    case 7:
      Serial.println("║                 РЕДАКТИРОВАНИЕ: EFFECT                       ║");
      Serial.print("║ Текущее значение: ");
      Serial.print(currentSettings.effect);

      if (currentSettings.effect < 10)
        Serial.print("  ");
      else if (currentSettings.effect < 100)
        Serial.print(" ");
      else if (currentSettings.effect > 99)
        Serial.print("");

      Serial.println("                                        ║");
      break;
  }

  Serial.println("╚══════════════════════════════════════════════════════════════╝");
  Serial.println("Введите новое значение и нажмите Enter:");
  // Serial.println("╚══════════════════════════════════════════════════════════════╝");
  // Serial.println();
}

void startDirectInput() {
  directInputMode = true;
  inputBuffer = "";
  currentParameter = 0;

  Serial.println();
  Serial.println("╔═══════════════════════════════════════════════════════════════╗");
  Serial.println("║                  РЕЖИМ ПРЯМОГО ВВОДА                          ║");
  Serial.println("╠═══════════════════════════════════════════════════════════════╣");
  Serial.println("║ Выберите параметр для редактирования:                         ║");
  Serial.println("║                                                               ║");

  Serial.print("║  1) Feedback     (0x1A):   ");
  Serial.print(currentSettings.feedbackReg);

  if (currentSettings.feedbackReg < 10)
    Serial.print("       ");
  else if (currentSettings.feedbackReg < 100)
    Serial.print("      ");
  else if (currentSettings.feedbackReg > 99)
    Serial.print("     ");

  Serial.print("Основной контроль мотора");
  // if (currentSettings.feedbackReg < 10) Serial.print(" ");
  Serial.println("   ║");

  Serial.print("║  2) Overdrive    (0x16):   ");
  Serial.print(currentSettings.overdriveReg);
  if (currentSettings.overdriveReg < 10)
    Serial.print("       ");
  else if (currentSettings.overdriveReg < 100)
    Serial.print("      ");
  else if (currentSettings.overdriveReg > 99)
    Serial.print("     ");
  Serial.print("Защита от перегрузки");
  // if (currentSettings.overdriveReg < 10) Serial.print(" ");
  Serial.println("       ║");

  Serial.print("║  3) Compensation (0x17):   ");
  Serial.print(currentSettings.compensationReg);

  if (currentSettings.compensationReg < 10)
    Serial.print("       ");
  else if (currentSettings.compensationReg < 100)
    Serial.print("      ");
  else if (currentSettings.compensationReg > 99)
    Serial.print("     ");

  Serial.print("Компенсация обратной связи");
  Serial.println(" ║");

  Serial.print("║  4) Drive        (0x18):   ");
  Serial.print(currentSettings.driveReg);

  if (currentSettings.driveReg < 10)
    Serial.print("       ");
  else if (currentSettings.driveReg < 100)
    Serial.print("      ");
  else if (currentSettings.driveReg > 99)
    Serial.print("     ");

  Serial.print("Сила вибрации (0-255)");
  // if (currentSettings.driveReg < 10) Serial.print(" ");
  Serial.println("      ║");

  Serial.print("║  5) Control      (0x1C):   ");
  Serial.print(currentSettings.controlReg);

  if (currentSettings.controlReg < 10)
    Serial.print("       ");
  else if (currentSettings.controlReg < 100)
    Serial.print("      ");
  else if (currentSettings.controlReg > 99)
    Serial.print("     ");

  Serial.print("Форма сигнала");
  // if (currentSettings.controlReg < 10) Serial.print(" ");
  Serial.println("              ║");

  Serial.print("║  6) Frequency          :   ");
  Serial.print(currentSettings.frequency);
  Serial.print(" Hz  Резонансная частота");
  Serial.println("        ║");

  Serial.print("║  7) Effect             :   ");
  Serial.print(currentSettings.effect);

  if (currentSettings.effect < 10)
    Serial.print("       ");
  else if (currentSettings.effect < 100)
    Serial.print("      ");
  else if (currentSettings.effect > 99)
    Serial.print("     ");

  Serial.print("Тип эффекта 1-117");
  if (currentSettings.effect < 10) Serial.print("");
  Serial.println("          ║");

  Serial.println("╠══════════════════════════════════════════════╦════════════════╣");
  Serial.println("║ Введите номер параметра (1-7) и нажмите      ║      Enter     ║");
  Serial.println("╟----------------------------------------------╫----------------╢");
  Serial.println("║ Для выхода в обычный режим нажмите           ║        {       ║");
  Serial.println("╚══════════════════════════════════════════════╩════════════════╝");
  Serial.println();
}

void cancelDirectInput() {
  directInputMode = false;
  Serial.println("\nВыход в обычный режим.");
  printCurrentSettings();
}

void finishValueInput() {
  if (inputBuffer.length() == 0) {
    // Не выводим сообщение об ошибке - просто игнорируем
    return;
  }

  int value = inputBuffer.toInt();
  setParameterValue(currentParameter, value);

  Serial.println("Значение применено!");
  applySettings();
  playEffect();

  // Сбрасываем для выбора нового параметра
  currentParameter = 0;
  inputBuffer = "";

  // Возвращаем в меню выбора параметра
  startDirectInput();
}

void processKeyInput(char cmd) {
  switch (cmd) {
    // Основные регулировки
    case 'w':
      currentSettings.feedbackReg++;
      break;  // Регистр 0x1A +
    case 'q':
      currentSettings.feedbackReg--;
      break;  // Регистр 0x1A -
    case 's':
      currentSettings.controlReg++;
      break;  // Регистр 0x1C +
    case 'a':
      currentSettings.controlReg--;
      break;  // Регистр 0x1C -
    case 'f':
      currentSettings.driveReg++;
      break;  // Регистр 0x18 +
    case 'd':
      currentSettings.driveReg--;
      break;  // Регистр 0x18 -

    // Дополнительные регулировки
    case 'h':
      currentSettings.overdriveReg++;
      break;  // Регистр 0x16 +
    case 'g':
      currentSettings.overdriveReg--;
      break;  // Регистр 0x16 -
    case 'k':
      currentSettings.compensationReg++;
      break;  // Регистр 0x17 +
    case 'j':
      currentSettings.compensationReg--;
      break;  // Регистр 0x17 -

    // Частота и эффекты
    case ';':
      currentSettings.frequency++;
      break;  // Частота +
    case 'l':
      currentSettings.frequency--;
      break;  // Частота -
    case '.':
      currentSettings.effect++;
      if (currentSettings.effect > 117) currentSettings.effect = 117;
      break;  // Эффект +
    case ',':
      currentSettings.effect--;
      if (currentSettings.effect < 1) currentSettings.effect = 1;
      break;  // Эффект -

    // Режим прямого ввода
    case '}':
      startDirectInput();
      return;  // Начать ввод значения
    case '{':
      cancelDirectInput();
      return;  // Отмена ввода

    // Быстрые пресеты
    case '1':
      loadPreset(1);
      break;  // Мягкий
    case '2':
      loadPreset(2);
      break;  // Средний
    case '3':
      loadPreset(3);
      break;  // Сильный

    // Воспроизведение
    case ' ':
      playEffect();
      break;

    default:
      return;  // Игнорируем другие символы
  }

  applySettings();
  printCurrentSettings();
  playEffect();
}

void finishDirectInput() {
  if (currentParameter == 0) {
    // Выбор параметра
    int param = inputBuffer.toInt();
    if (param >= 1 && param <= 7) {
      currentParameter = param;
      inputBuffer = "";
      Serial.println();
      printParameterName(param);
      Serial.println("Введите новое значение:");
    } else {
      Serial.println("\nОшибка: неверный номер параметра (1-7)");
      cancelDirectInput();
    }
  } else {
    // Ввод значения
    int value = inputBuffer.toInt();
    setParameterValue(currentParameter, value);
    directInputMode = false;
    Serial.println();
    applySettings();
    printCurrentSettings();
    playEffect();
  }
}

void setParameterValue(int param, int value) {
  switch (param) {
    case 1:
      currentSettings.feedbackReg = constrain(value, 0, 255);
      break;
    case 2:
      currentSettings.overdriveReg = constrain(value, 0, 255);
      break;
    case 3:
      currentSettings.compensationReg = constrain(value, 0, 255);
      break;
    case 4:
      currentSettings.driveReg = constrain(value, 0, 255);
      break;
    case 5:
      currentSettings.controlReg = constrain(value, 0, 255);
      break;
    case 6:
      currentSettings.frequency = constrain(value, 100, 300);
      break;
    case 7:
      currentSettings.effect = constrain(value, 1, 127);
      break;
  }
}

void applySettings() {
  drv.setMode(DRV2605_MODE_INTTRIG);
  drv.selectLibrary(1);

  drv.writeRegister8(0x1A, currentSettings.feedbackReg);
  drv.writeRegister8(0x16, currentSettings.overdriveReg);
  drv.writeRegister8(0x17, currentSettings.compensationReg);
  drv.writeRegister8(0x18, currentSettings.driveReg);
  drv.writeRegister8(0x1C, currentSettings.controlReg);
  drv.writeRegister8(0x20, currentSettings.frequency);
}

void playEffect() {
  drv.setWaveform(0, currentSettings.effect);
  drv.setWaveform(1, 0);
  drv.go();
  Serial.println("Playing effect...");
}

void printCurrentSettings() {
  Serial.println();
  Serial.println("╔══════════════════════════════════════════════════════════════════════════════════════════════════════╗");
  Serial.println("║                                           ТЕКУЩИЕ НАСТРОЙКИ                                          ║");
  Serial.println("╠══════════════════════════════════════════════════════════════════════════════════════════════════════╣");

  Serial.print("║ 1) Feedback      (0x1A)  |  q/w  |  ");
  Serial.print(currentSettings.feedbackReg);
  if (currentSettings.feedbackReg < 100) Serial.print(" ");
  if (currentSettings.feedbackReg < 10) Serial.print(" ");
  Serial.print(" (0x");
  if (currentSettings.feedbackReg < 16) Serial.print("0");
  Serial.print(currentSettings.feedbackReg, HEX);
  Serial.print(")");
  Serial.print("  |  Основной контроль: тип мотора и режим работы");
  Serial.println("      ║");

  Serial.print("║ 2) Overdrive     (0x16)  |  g/h  |  ");
  Serial.print(currentSettings.overdriveReg);
  if (currentSettings.overdriveReg < 100) Serial.print(" ");
  if (currentSettings.overdriveReg < 10) Serial.print(" ");
  Serial.print(" (0x");
  if (currentSettings.overdriveReg < 16) Serial.print("0");
  Serial.print(currentSettings.overdriveReg, HEX);
  Serial.print(")");
  Serial.print("  |  Защита от перегрузки: чем выше, тем безопаснее");
  Serial.println("    ║");

  Serial.print("║ 3) Compensation  (0x17)  |  j/k  |  ");
  Serial.print(currentSettings.compensationReg);
  if (currentSettings.compensationReg < 100) Serial.print(" ");
  if (currentSettings.compensationReg < 10) Serial.print(" ");
  Serial.print(" (0x");
  if (currentSettings.compensationReg < 16) Serial.print("0");
  Serial.print(currentSettings.compensationReg, HEX);
  Serial.print(")");
  Serial.print("  |  Компенсация: для стабильной работы мотора");
  Serial.println("         ║");

  Serial.print("║ 4) Drive         (0x18)  |  d/f  |  ");
  Serial.print(currentSettings.driveReg);
  if (currentSettings.driveReg < 100) Serial.print(" ");
  if (currentSettings.driveReg < 10) Serial.print(" ");
  Serial.print(" (0x");
  if (currentSettings.driveReg < 16) Serial.print("0");
  Serial.print(currentSettings.driveReg, HEX);
  Serial.print(")");
  Serial.print("  |  Усиление: 0-255, прямо влияет на силу вибрации");
  Serial.println("    ║");

  Serial.print("║ 5) Control       (0x1C)  |  a/s  |  ");
  Serial.print(currentSettings.controlReg);
  if (currentSettings.controlReg < 100) Serial.print(" ");
  if (currentSettings.controlReg < 10) Serial.print(" ");
  Serial.print(" (0x");
  if (currentSettings.controlReg < 16) Serial.print("0");
  Serial.print(currentSettings.controlReg, HEX);
  Serial.print(")");
  Serial.print("  |  Форма сигнала: влияет на резкость и отклик");
  Serial.println("        ║");

  Serial.print("║ 6) Frequency             |  l/;  |  ");
  Serial.print(currentSettings.frequency);
  Serial.print(" Hz      |  Резонансная частота: ДОЛЖНА совпадать с мотором!");
   Serial.println("  ║");

  Serial.print("║ 7) Effect                |  </>  |  ");
  Serial.print(currentSettings.effect);
  Serial.print("          |  Тип эффект");
   Serial.println("                                        ║");

  Serial.println("╠══════════════════╦══════════════╦════════════════════════════════════════════════════════════════════╣");
  Serial.println("║ прямой ввод - }  ║  отмена - {  ║  воспроизвести -  ПРОБЕЛ                                           ║");
  Serial.println("╚══════════════════╩══════════════╩════════════════════════════════════════════════════════════════════╝");
  Serial.println();
  Serial.println("┌──────────────────────────────────────────────────────┐");
  Serial.println("|                 Советы по настройке:                 |");
  Serial.println("├──────────────────────────────────────────────────────┤");
  Serial.println("| 1) Увеличить силу: повысить Drive и Feedback         |");
  Serial.println("| 2) Сделать мягче: уменьшить Drive, эффект 12 или 14  |");
  Serial.println("| 3) Быстрее отклик: увеличить Control                 |");
  Serial.println("| 4) Стабильнее: настроить Compensation                |");
  Serial.println("| 5) Пресеты: 1=мягкий, 2=средний, 3=сильный           |");
  Serial.println("└──────────────────────────────────────────────────────┘");
  Serial.println();
}

void loadPreset(int preset) {
  switch (preset) {
    case 1:  // Мягкий пресет
      currentSettings.feedbackReg = 0x86;
      currentSettings.controlReg = 0x10;
      currentSettings.driveReg = 0x04;
      currentSettings.effect = 12;
      currentSettings.overdriveReg = 0x80;
      currentSettings.compensationReg = 0x15;
      break;

    case 2:  // Средний пресет
      currentSettings.feedbackReg = 0x93;
      currentSettings.controlReg = 0x20;
      currentSettings.driveReg = 0x06;
      currentSettings.effect = 14;
      currentSettings.overdriveReg = 0x83;
      currentSettings.compensationReg = 0x1A;
      break;

    case 3:  // Сильный пресет
      currentSettings.feedbackReg = 0xA0;
      currentSettings.controlReg = 0x30;
      currentSettings.driveReg = 0x08;
      currentSettings.effect = 37;
      currentSettings.overdriveReg = 0x83;
      currentSettings.compensationReg = 0x1A;
      break;
  }
}