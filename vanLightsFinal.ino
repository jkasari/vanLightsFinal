#include <Adafruit_NeoPixel.h>
#define LED_PIN 6
#define LED_COUNT 50
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);
#define WRM_WHT strip.Color(127, 255, 0) 
#define RED strip.Color(0, 255, 0)
#define GRN strip.Color(255, 0, 0)
#define LOW_BRIGHTNESS 5
#define HIGH_BRIGHTNESS 254
#define POT1 A1
#define POT2 A3
#define BUTTON A5
#define MODE_NUM 2



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
        return map(value, 7, 1016, lowRead, highRead);
      }

      void resetLimits(int32_t low, int32_t high) { // Resets the limits on the pot reader.
        highRead = high;
        lowRead = low;
      }

      void calibrateSensor() {
        value = analogRead(port);
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

    bool stayInMode(int8_t& mode) { // Returns true if the mode has not changed, and false if the mode changes.
      while(digitalRead(port) == LOW) {
        count++; // Keep track of how long the button has been held down for. 
        delay(1);
        if (count > moreSec) { // What tf is this?? Well this is a fake battery life display if you hold the button down long enough.
          strip.clear();
          strip.fill(GRN, 0, LED_COUNT);
          strip.show();
        }
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
    const int16_t moreSec = 3000;
};

class LEDDisplay{ // This class is incharge of just lighting leds up on the bar.

  public:
    void setColor(uint32_t newColor) { // Tell the display what color you would like to light up.
      color = newColor;
      for (int i = 0; i < LED_COUNT; ++i) {
        ColorDots[i].setLocation(i);
      }
    }
 
    void slideLightDisplay(int32_t potReading, int32_t brightness) { // The classic slidy light! This lights up 6 leds on the bar.
      head = potReading + 3; // The leds position depends on the reading from the display pot, which is passed to this function,
      tail = potReading - 3;
      for (int i = tail; i <= head; ++i) {
        displayLEDInBounds(i);
      }
      lowLightDisplay(brightness);
    }


    void bigLightDisplay(int32_t potReading, int32_t brightness) {
        head = potReading + (LED_COUNT / 2) + 6; // The leds position depends on the reading from the display pot, which is passed to this function,
        tail = potReading - (LED_COUNT / 2) - 6; // The plus 6 here is so that the light pauses longer when fully displayed. 
        for (int i = tail; i <= head; ++i) {
          displayLEDInBounds(i);
        }
        lowLightDisplay(brightness);
    }

    // Needs to be used in a loop. This will display random leds, fading them on a and off. Once off, they stay off untill turned on again.
    // The pot reading here acts as a delay stopping the function from turning leds on and off.
    // So everytime the count lines up with the pot reading, the function lights up another led.
    // The greater the pot reading the longer between new leds getting lit up. 
    void discoLights(int32_t potReading) {
      count++;
      if (count % potReading == 0) { // Time to light up another dot. 
        ColorDots[random(LED_COUNT)].turnOn();
      }
      for (int i = 0; i < LED_COUNT; ++i) {
        ColorDots[i].displayDot();
      }
    }

  private:
    // Helper class for the disco light function
    class ColoredDots{

      public:
        void displayDot() { // If the dot is on, move it up or down in brightness.
          if (ON) {
            if (brightDir) {
              increaseBrightness();
            } else {
              decreaseBrightnes();
            }
            displaySelf();
          }
        }

        void turnOn() {
          ON = true;
        }

        void setLocation(int32_t newLoc) {
          location = newLoc;
        }

      private:
        uint8_t colorIndex = random(5);
        int32_t location = 0;
        uint8_t brightValue = 0;
        size_t count = 0;
        bool brightDir = true;
        bool ON = false;
        int8_t changeRate = random(5);

        void increaseBrightness() {
          brightValue += changeRate;
          if (brightValue >= 250) {
            brightDir = false;
          }
        }

        void decreaseBrightnes() {
          brightValue -= changeRate;
          if (brightValue <= 5) {
            brightDir = true;
            ON = false;
            brightValue = 0;
            colorIndex = random(5);
          }
        }

        void setRandomColor() { // This picks a new color index for the pick color function
          colorIndex = random(5);
        }

        // This allows you to animate a color for any level of brightness free of strip brightness.
        uint32_t pickColor(uint8_t color, uint8_t brightness) {
          switch(color) {
           case 0:
           //White
            return strip.Color(brightness, brightness, brightness);
           case 1:
            //Magenta
            return strip.Color(0, brightness, brightness);
           case 2:
            //Violet
            return strip.Color(0, brightness / 2, brightness);
           case 3:
            // Green ish??
            return strip.Color(brightness, brightness / 2, 0);
           case 4:
            // Cyan
            return strip.Color(brightness, 0, brightness );
          }
          return strip.Color(0, 0, 0);
        }

         void displaySelf() {
           strip.setPixelColor(location, pickColor(colorIndex, brightValue));
         }
    };

    ColoredDots ColorDots[LED_COUNT];
    size_t count = 0;
    uint32_t color = 0;
    int32_t head = 0;
    int32_t tail = 0;
    uint8_t lowLightThreshold = LOW_BRIGHTNESS + 1;
    void displayLEDInBounds(int32_t loc) { // This takes in a location and only displays it if it is in bounds on the led strip.
      if (loc >= 0 && LED_COUNT >= loc) {
        strip.setPixelColor(loc, color);
      }
    }
    void lowLightDisplay(int32_t brightness) { // Turns the strip red when the light is turned down low enough. 
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
          discoParty();
          break;
      }
    }

    void calibrateSensors() {
      BrightnessPot.calibrateSensor();
      brightness = BrightnessPot.getValue();
      DisplayPot.calibrateSensor();
    }

  private:
    PotientometerSmoother BrightnessPot;
    PotientometerSmoother DisplayPot;
    ButtonControl ButtControl;
    LEDDisplay LightDisplay;
    int8_t mode = 0;
    bool fading = false;
    uint32_t brightness = 0;
    uint8_t lowLightThreshold = 25;

    void brightnessCheck() { // Check the reading on the brightness potentiometer. If it has changed update the overall strip brightness.
      uint32_t tempBir = BrightnessPot.getValue();
      if (brightness != tempBir) {
        brightness = tempBir;
        strip.setBrightness(brightness);
      }
    }

    void fadeOn() { // Take the brightness from zero to whatever the current level is.
      for(int i = 1; i < 101; ++i) {
        brightnessCheck();
        uint32_t fadeValue = brightness * i / 100;
        strip.setBrightness(fadeValue);
        strip.show();
        delay(2);
      }
      strip.setBrightness(brightness);
    }

    void fadeOff() { // Take the brightness from the current level down to zero.
      for(int i = 100; i > 0; --i) {
        brightnessCheck();
        uint32_t fadeValue = brightness * i / 100;
        strip.setBrightness(fadeValue);
        strip.show();
        delay(2);
      }
      strip.setBrightness(brightness);
      strip.clear();
    }

    void slideLight() { // Displays a 5 led light pod and slides it around for localized light
      DisplayPot.resetLimits(-3, LED_COUNT + 2);
      LightDisplay.setColor(WRM_WHT);
      LightDisplay.slideLightDisplay(DisplayPot.getValue(), brightness);
      fadeOn();
      while(ButtControl.stayInMode(mode)) {
        strip.clear();
        LightDisplay.slideLightDisplay(DisplayPot.getValue(), brightness);
        brightnessCheck();
        strip.show();
      }
      fadeOff();
    }

    void bigLight() { // This time the light pod is the size of the led strip. Sliding it one way or the other essentially shifts the total light on the strip.
      DisplayPot.resetLimits((-LED_COUNT / 2) - 6, LED_COUNT + (LED_COUNT / 2) + 5);
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

    void discoParty() { // Randomly lights up colored leds for a party! how many leds are currently lit is decided by the display pot. 
      DisplayPot.resetLimits(1, 50);
      fadeOn();
      while(ButtControl.stayInMode(mode)) {
        LightDisplay.discoLights(DisplayPot.getValue());
        brightnessCheck();
        delay(1);
        strip.show();
      }
      fadeOff();
    }
};

LEDBar TheLight(POT2, LOW_BRIGHTNESS, HIGH_BRIGHTNESS, POT1, 0, 0, BUTTON, MODE_NUM);

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.clear();
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A5, INPUT_PULLUP);
  pinMode(A7, INPUT);
  TheLight.calibrateSensors();
}



void loop() {  
  TheLight.isOn();
}
