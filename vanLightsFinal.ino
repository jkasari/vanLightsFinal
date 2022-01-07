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
        target = analogRead(port);
        value += floor((target - value) / 8);
        return map(value, 7, 893, lowRead, highRead);
      }

      void resetLimits(int32_t low, int32_t high) { // Resets the limits on the pot reader.
        highRead = high;
        lowRead = low;
      }

    private:
      uint8_t port = 0;
      int32_t target = 0;
      int32_t value = 0;
      int32_t highRead = 0;
      int32_t lowRead = 0;
};

class ButtonControl { //This class keeps an eye on the button and mode managment

  public:
    ButtonControl(uint8_t portNum, uint8_t modeLim) {
      port = portNum;
      modeLimit = modeLim;
    }

    bool stayInMode(uint8_t& mode) { // Returns true if the mode has not changed, and false if the mode changes.
      while(digitalRead(port) == LOW) {
        count++; // Keep track of how long the button has been held down for. 
        delay(1);
      }
      if (count > 1) { // Only check in here if the button has been held down at all.
        if (halfSec > count) {mode++;}
        if (count > halfSec && moreSec > count) {mode--;}
        count = 0;
        mode = modeCheck(mode);
        return false;
      }
      return true;
    }

  private:
    uint8_t modeCheck(int8_t mode) { // Returns an adjusted mode value that stays in bounds
      if (mode > modeLimit) {
        return 0;
      } 
      if (0 > mode) {
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

class LEDDisplay{ // This class is incharge of just lighting leds up on the bar.

  public:
    void setColor(uint8_t red, uint8_t green, uint8_t blue) { // Tell the display what color you would like to light up.
      color = strip.Color(green, red, blue);
    }

    void slideLight(size_t potReading) { // The classic slidy light! This lights up 6 leds on the bar.
        head = potReading + 3; // The leds position depends on the reading from the display pot, which is passed to this function,
        tail = potReading - 3;
        for (int i = tail; i < head; ++i) {
          displayLEDInBounds(i);
        }
    }

  private:
    uint32_t color;
    int32_t head = 0;
    int32_t tail = 0;
    void displayLEDInBounds(int32_t loc) { // This takes in a location and only displays it if it is in bounds on the led strip.
      if (loc >= 0 && LED_COUNT >= loc) {
        strip.setPixelColor(loc, color);
      }
    }

};

class LEDBar { // This is the main class that houses everything!!
  public:

    LEDBar(uint8_t bPort, // Jesus christ look at all those variables!!!
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

    void isOn() { // This runs the light! Stick this in the main loop on the arduino for best results.
      brightness = BrightnessPot.getValue(); // Make sure we are 
      switch (mode) {
        case 0:
          slideLight();
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
    uint8_t mode = 0;
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

    void slideLight() {
      DisplayPot.resetLimits(-2, LED_COUNT + 2);
      LightDisplay.setColor(125, 255, 0);
      LightDisplay.slideLight(DisplayPot.getValue());
      fadeOn();
      while(ButtControl.stayInMode(mode)) {
        strip.clear();
        LightDisplay.slideLight(DisplayPot.getValue());
        Serial.println(DisplayPot.getValue());
        brightnessCheck();
        strip.show();
      }
      fadeOff();
    }

    void dis1() {
      strip.setPixelColor(10, 0, 255, 0);
      strip.show();
    }

    void dis2() {
      strip.setPixelColor(10, 0, 255, 0);
      strip.show();
    }

    void dis3() {
      strip.setPixelColor(10, 0, 255, 0);
      strip.show();
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
