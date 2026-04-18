int buttonPin = 2;//対象ボタンピン
int ledPin = 13;//対象LED出力ピン
int sensorPin = A0;//対象センサーピン
int lastState = HIGH;//直前のボタンの状態(未確定)
int stableState = HIGH;//現在のボタンの状態(確定)
int lastStableState = HIGH;//直前のボタンの状態(確定)
int sensorValue = 0;//光センサー値

enum ButtonState {
  BUTTON_RELEASED,
  BUTTON_PRESSED,
  BUTTON_LONG_PRESSED
};

enum ButtonEvent {
  BUTTON_EVENT_NONE,
  BUTTON_EVENT_SHORT_PRESS,
  BUTTON_EVENT_LONG_PRESS
};

enum Mode {
  MODE_STOP,
  MODE_PATTERN1,
  MODE_PATTERN2
};

enum Led {
  LED_ON,
  LED_OFF
};
enum ButtonEvent buttonEvent = BUTTON_EVENT_NONE;
enum ButtonState buttonState = BUTTON_RELEASED;
enum Mode mode = MODE_STOP;
enum Led ledState = LED_OFF;

unsigned long startTime = 0;
unsigned long changeTime = 0;//チャタリング確認用
unsigned long pressStartTime = 0;//長押し確認用
unsigned long lastPrintTime = 0;//ログ表示頻度調整用

const unsigned long debounceTime = 20;
const unsigned long longPressTime = 1000;

// 点滅周期パラメータ
unsigned long onTime = 1000;
unsigned long offTime = 3000;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(115200);
}

void loop() {
  unsigned long currentTime = millis();

  updateButton(currentTime);
  handleMode(currentTime);
  updateSensor(currentTime);
  updateLed(currentTime);
}

void updateButton(unsigned long currentTime) {
  int currentState = digitalRead(buttonPin);

  if (currentState != lastState) {
    changeTime = currentTime;
    lastState = currentState;
  }

  if (currentTime - changeTime >= debounceTime) {
    if (stableState != currentState) {
      lastStableState = stableState;
      stableState = currentState;

      // HIGH → LOW：押し始め
      if (lastStableState == HIGH && stableState == LOW) {
        pressStartTime = currentTime;
        buttonState = BUTTON_PRESSED;
      }

      // LOW → HIGH：離した瞬間
      if (lastStableState == LOW && stableState == HIGH) {
        if (buttonState == BUTTON_PRESSED) {
          buttonEvent = BUTTON_EVENT_SHORT_PRESS;
        }
        buttonState = BUTTON_RELEASED;
      }
    }
  }
  // 押され続けて1秒以上経過したら長押し
  if (buttonState == BUTTON_PRESSED && currentTime - pressStartTime >= longPressTime) {
    buttonEvent = BUTTON_EVENT_LONG_PRESS;
    buttonState = BUTTON_LONG_PRESSED;
  }
}

void handleMode(unsigned long currentTime) {
  if (buttonEvent == BUTTON_EVENT_LONG_PRESS) {
    mode = MODE_STOP;
    ledState = LED_OFF;
    digitalWrite(ledPin, LOW);
    Serial.println("STOP mode0");
    buttonEvent = BUTTON_EVENT_NONE;
  }
  else if (buttonEvent == BUTTON_EVENT_SHORT_PRESS) {
    if (mode == MODE_STOP) {
      mode = MODE_PATTERN1;
      ledState = LED_OFF;
      startTime = currentTime;
      digitalWrite(ledPin, LOW);
      Serial.println("START mode1");
      offTime = 3000;
      onTime  = 1000;
    }
    else if (mode == MODE_PATTERN1) {
      mode = MODE_PATTERN2;
      ledState = LED_OFF;
      startTime = currentTime;
      digitalWrite(ledPin, LOW);
      Serial.println("START mode2");
      offTime = 1000;
      onTime  = 1000;
    }
    else if (mode == MODE_PATTERN2) {
      mode = MODE_PATTERN1;
      ledState = LED_OFF;
      startTime = currentTime;
      digitalWrite(ledPin, LOW);
      Serial.println("START mode1");
      offTime = 3000;
      onTime  = 1000;
    }

    buttonEvent = BUTTON_EVENT_NONE;
  }
}

void updateSensor(unsigned long currentTime) {
  sensorValue = analogRead(sensorPin);

  if (mode == MODE_PATTERN2) {
    int adjustedSensorValue = constrain(sensorValue, 0, 150);
    offTime = map(adjustedSensorValue, 0, 150, 2000, 300);
    onTime = map(adjustedSensorValue, 0, 150, 2000, 300);

    if (currentTime - lastPrintTime >= 500) {
      lastPrintTime = currentTime;
      Serial.print(sensorValue);
      Serial.print("\t");
      Serial.print(offTime);
      Serial.print("\t");
      Serial.println(onTime);
    }
  }
}

void updateLed(unsigned long currentTime) {
  if (mode == MODE_STOP) {
    return;
  }

  if (ledState == LED_OFF && currentTime - startTime >= offTime) {
    digitalWrite(ledPin, HIGH);
    startTime = currentTime;
    ledState = LED_ON;
  }
  else if (ledState == LED_ON && currentTime - startTime >= onTime) {
    digitalWrite(ledPin, LOW);
    startTime = currentTime;
    ledState = LED_OFF;
  }
}
