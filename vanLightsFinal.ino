#include <Adafruit_NeoPixel.h>
#define LED_PIN 6
#define LED_COUNT 32
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);


class PotientometerSmoother{ 
  
    public: // Give it the number of the port, along the high and low limits of the values you want back. 
      PotientometerSmoother(uint8_t portNum, int32_t low, int32_t high) {
        port = portNum;
        highRead = high;
        lowRead = low;
      }

      // returns the smoothed out value of the potentiometer.
      int getValue() {
        target = map(analogRead(port), 0, 1023, lowRead - 7, highRead - 7);
        value += floor((target - value) / 8);
        return value;
      }

    private:
      uint8_t port = 0;
      int32_t target = 0;
      int32_t value = 0;
      int32_t highRead = 0;
      int32_t lowRead = 0;
};

class ButtonControl {

  public:
    ButtonControl(uint8_t portNum, uint8_t modeLim) {
      port = portNum;
      modeLimit = modeLim;
    }

    bool stayInMode(uint8_t& mode) {
      while(digitalRead(port) == LOW) {
        count++;
        delay(1);
      }
      if (count > 1) {
        if (halfSec > count) {mode++;}
        if (count > halfSec && moreSec > count) {mode--;}
        //if (count > moreSec) {mode = 0;}
        count = 0;
        mode = modeCheck(mode);
        return false;
      }
      return true;
    }

  private:
    uint8_t modeCheck(int8_t mode) {
      if (mode > modeLimit) {
        return 1;
      } 
      if (1 > mode) {
        return modeLimit;
      }
      return mode;
    }
    size_t count = 0;
    int8_t modeLimit = 0;
    uint8_t port = 0;
    const int16_t halfSec = 300; // These are how long a button needs to be held for to switch modes.
    const int16_t moreSec = 5000;
};

class LEDDisplay{ 

  public:
    void slideLight(size_t) {
        uint32_t color = strip.Color(0, 255, 0);
        strip.fill(color, 0, LED_COUNT);
        strip.show();
    }

  private:

};

class LEDBar {
  public:

    LEDBar(uint8_t bPort, 
           size_t bLow,
           size_t bHigh,
           uint8_t dPort,
           size_t dLow,
           size_t dHigh,
           uint8_t buttPort, 
           uint8_t modeLimit) : 
           BrightnessPot(bPort, bLow, bHigh),
           DisplayPot(dPort, dLow, dHigh),
           ButtControl(buttPort, modeLimit) {}

    void isOn() {
      brightness = BrightnessPot.getValue();
      switch (mode) {
        case 0:
          batteryDisplay();
          break;
        case 1:
          dis1();
          break;
        case 2:
          dis2();
          break;
        case 3:
          dis3();
          break;
      }
    }

  private:
    PotientometerSmoother BrightnessPot;
    PotientometerSmoother DisplayPot;
    ButtonControl ButtControl;
    LEDDisplay LightDisplay;
    uint8_t mode = 1;
    bool fading = false;
    uint32_t brightness = 0;

    void brightnessCheck() {
      if (brightness != BrightnessPot.getValue()) {
        strip.setBrightness(BrightnessPot.getValue());
      }
    }

    void fadeOn() {
      for(int i = 1; i < 101; ++i) {
        uint32_t fadeValue = brightness * i / 100;
        strip.setBrightness(fadeValue);
        strip.show();
        delay(2);
      }
      strip.setBrightness(brightness);
    }

    void fadeOff() {
      for(int i = 100; i > 0; --i) {
        uint32_t fadeValue = brightness * i / 100;
        strip.setBrightness(fadeValue);
        strip.show();
        delay(2);
      }
      strip.setBrightness(brightness);
    }

    void batteryDisplay() {
      LightDisplay.slideLight(DisplayPot.getValue());
      fadeOn();
      while(ButtControl.stayInMode(mode)) {
        LightDisplay.slideLight(DisplayPot.getValue());
        Serial.println(BrightnessPot.getValue());
        brightnessCheck();
      }
      fadeOff();
    }

    void dis1() {
      LightDisplay.slideLight(DisplayPot.getValue());
      fadeOn();
      while(ButtControl.stayInMode(mode)) {
        LightDisplay.slideLight(DisplayPot.getValue());
        Serial.println(BrightnessPot.getValue());
        brightnessCheck();
      }
      fadeOff();
    }

    void dis2() {
      LightDisplay.slideLight(DisplayPot.getValue());
      fadeOn();
      while(ButtControl.stayInMode(mode)) {
        LightDisplay.slideLight(DisplayPot.getValue());
        Serial.println(BrightnessPot.getValue());
        brightnessCheck();
      }
      fadeOff();
    }

    void dis3() {
      LightDisplay.slideLight(DisplayPot.getValue());
      fadeOn();
      while(ButtControl.stayInMode(mode)) {
        LightDisplay.slideLight(DisplayPot.getValue());
        Serial.println(BrightnessPot.getValue());
        brightnessCheck();
      }
      fadeOff();
    }

};

LEDBar TheLight(A0, 20, 255, A1, 0, 255, A5, 3);

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.clear();
  pinMode(A1, INPUT);
  pinMode(A5, INPUT_PULLUP);
  pinMode(A7, INPUT);
}



void loop() {  
  TheLight.isOn();
}
