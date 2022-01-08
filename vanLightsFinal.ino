#include <Adafruit_NeoPixel.h>
#define LED_PIN 6
#define LED_COUNT 32
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);
#define WRM_WHT strip.Color(200, 255, 0) 
#define RED strip.Color(0, 255, 0)
#define LOW_BRIGHTNESS 20
#define HIGH_BRIGHTNESS 255



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
        if (pow((value - target), 2) > 9) { value = target;} // If the difference is bigger than 3 change the value. 
        return map(value, 0, 1023, lowRead, highRead);
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
    void setColor(uint32_t newColor) { // Tell the display what color you would like to light up.
      color = newColor;
    }

 
    void slideLightDisplay(int32_t potReading, int32_t brightness) { // The classic slidy light! This lights up 6 leds on the bar.
      head = potReading + 2; // The leds position depends on the reading from the display pot, which is passed to this function,
      tail = potReading - 2;
      for (int i = tail; i <= head; ++i) {
        displayLEDInBounds(i);
      }
      lowLightDisplay(brightness);
    }


    void bigLightDisplay(int32_t potReading, int32_t brightness) {
        head = potReading + (LED_COUNT / 2); // The leds position depends on the reading from the display pot, which is passed to this function,
        tail = potReading - (LED_COUNT / 2);
        // Serial.println(potReading); if I un comment this, the whole thing crashes... why????
        for (int i = tail; i <= head; ++i) {
          displayLEDInBounds(i);
        }
        lowLightDisplay(brightness);
    }

  private:
    uint32_t color;
    int32_t head = 0;
    int32_t tail = 0;
    uint8_t lowLightThreshold = 25;
    void displayLEDInBounds(int32_t loc) { // This takes in a location and only displays it if it is in bounds on the led strip.
      if (loc >= 0 && LED_COUNT >= loc) {
        strip.setPixelColor(loc, color);
      }
    }
    void lowLightDisplay(int32_t brightness) {
      if (brightness < lowLightThreshold) {
        strip.clear();
        strip.fill(RED, 0);
      }

    }
};

class LEDBar { // This is the main class that houses everything!!
  public:

    LEDBar(uint8_t bPort, // Jesus christ look at all those variables!!!
           int32_t bLow,
           int32_t bHigh,
           uint8_t dPort,
           int32_t dLow,
           int32_t dHigh,
           uint8_t buttPort, 
           uint8_t modeLimit) : 
           BrightnessPot(bPort, bLow, bHigh),
           DisplayPot(dPort, dLow, dHigh),
           ButtControl(buttPort, modeLimit) {}

    void isOn() { // This runs the light! Stick this in the main loop on the arduino for best results.
      switch (mode) {
        case 0:
          slideLight();
          break;
        case 1:
          bigLight();
          break;
        case 2:
          dis2();
          break;
        case 3:
          dis3();
          break;
      }
    }

    void calibrateBrightness() { // One time use just to calibrate the brightness before starting the program.
      brightness = BrightnessPot.getValue();
    }

  private:
    PotientometerSmoother BrightnessPot;
    PotientometerSmoother DisplayPot;
    ButtonControl ButtControl;
    LEDDisplay LightDisplay;
    uint8_t mode = 0;
    bool fading = false;
    uint32_t brightness = 0;
    uint8_t lowLightThreshold = 25;

    void brightnessCheck() {
      uint32_t tempBir = BrightnessPot.getValue();
      if (brightness != tempBir) {
        brightness = tempBir;
        strip.setBrightness(brightness);
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
      strip.clear();
    }

    void slideLight() {
      DisplayPot.resetLimits(-2, LED_COUNT + 1);
      LightDisplay.setColor(WRM_WHT);
      LightDisplay.slideLightDisplay(DisplayPot.getValue(), brightness);
      fadeOn();
      while(ButtControl.stayInMode(mode)) {
        strip.clear();
        LightDisplay.slideLightDisplay(DisplayPot.getValue(), brightness);
        brightnessCheck();
        strip.show();
        Serial.println(brightness);
      }
      fadeOff();
    }

    void bigLight() {
      DisplayPot.resetLimits((-LED_COUNT / 2), LED_COUNT + (LED_COUNT / 2) - 1);
      LightDisplay.setColor(WRM_WHT);
      LightDisplay.bigLightDisplay(DisplayPot.getValue(), brightness);
      fadeOn();
      while(ButtControl.stayInMode(mode)) {
        strip.clear();
        LightDisplay.bigLightDisplay(DisplayPot.getValue(), brightness);
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

LEDBar TheLight(A0, LOW_BRIGHTNESS, HIGH_BRIGHTNESS, A1, 0, 0, A5, 3);

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.clear();
  pinMode(A1, INPUT);
  pinMode(A5, INPUT_PULLUP);
  pinMode(A7, INPUT);
  TheLight.calibrateBrightness();
}



void loop() {  
  TheLight.isOn();
}
