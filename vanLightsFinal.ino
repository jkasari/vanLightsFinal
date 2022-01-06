#include <Adafruit_NeoPixel.h>
#define LED_PIN 6
#define LED_COUNT 50
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);
uint32_t knobbyBoi1, knobbyBoi2, batteryMoniter, volts;
uint8_t button, lightValue, oldLightValue, displayValue, oldDisplayValue;
uint8_t kbt = 2; // kbt stands for knobby boi tolerance. This adjusts how far you have to turn a knob before the program reconizes the movement.
float voltsDec; 
bool displayBattery = false;
bool on = true;
uint8_t globalMode = 0;

void setup() {
  knobbyBoi2 = analogRead(A3);
  lightValue = map(knobbyBoi2, 0, 1023, 20, 255);
  Serial.begin(9600);
  strip.begin();
  strip.setBrightness(lightValue);
  strip.clear();
  pinMode(A1, INPUT);
  pinMode(A3, INPUT);
  pinMode(A5, INPUT_PULLUP);
  pinMode(A7, INPUT);
}



void loop() {  
  if(displayBattery) {
    batteryLife();
  }
  
  on = true;
  switch(globalMode) {
   case 0:
    slidyLight();
    break;
   case 1:
    boringLight();
    break;
   case 2:
    discoParty();
    break;
  }
  
  if(0 > globalMode || globalMode > 2) {
    globalMode = 0;
  }
}



void buttonCheck() {
  uint32_t buttonTimer = 0;
  uint32_t halfSec = 300;
  uint32_t moreSec = 1500;
  if (digitalRead(A5) == LOW) {
    on = false;
    while(digitalRead(A5) == LOW) {
      buttonTimer += 1;
      delay(1);
    }
    if (displayBattery == true) {
      displayBattery = false;
      buttonTimer = 0;
    }
    if (50 < buttonTimer && buttonTimer < halfSec) { globalMode += 1; }
    if (buttonTimer > halfSec && moreSec > buttonTimer) { globalMode -= 1; }
    if (buttonTimer > moreSec) { displayBattery = true; } 
  }
}

//==============================================================BasicFunctions


void brightnessCheck(uint8_t lightValue) {
  if(lightValue - oldLightValue < kbt || oldLightValue - lightValue < kbt) {
    strip.setBrightness(lightValue);
  }
  oldLightValue = lightValue;
}

// kbt stands for "Knobby Boi Tolerance"

void boringDisplaySection() {
  knobbyBoi1 = analogRead(A1);
  displayValue = map(knobbyBoi1, 0, 1023, 0, LED_COUNT * 2);
  if(oldDisplayValue - displayValue < kbt || displayValue - oldDisplayValue < kbt) {
    oldDisplayValue = displayValue;
    if(displayValue < LED_COUNT) {
      for(int i = displayValue + 1; i < LED_COUNT; i++) {
        strip.setPixelColor(i, 0, 0, 0);
      }
    } else {
      for(int i = 0; i < displayValue - LED_COUNT - 1; i++) {
        strip.setPixelColor(i, 0, 0, 0);
      }
    }
  }
}


void slidyDisplaySection() {
  knobbyBoi1 = analogRead(A1);
  displayValue = map(knobbyBoi1, 0, 1023, 5, LED_COUNT - 5);
  if(oldDisplayValue - displayValue < kbt || displayValue - oldDisplayValue < kbt) {
    oldDisplayValue = displayValue;
    uint8_t lightTop = displayValue + 5;
    uint8_t lightBottom = displayValue - 5;
    for(int i = lightTop; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 0, 0, 0);
    } 
    for(int i = 0; i < lightBottom; ++i) {
      strip.setPixelColor(i, 0, 0, 0);
    }
  }
}


//void randomDisplaySection(uint8_t displayValue, uint32_t* randomArr) {
//  for(int i = 0; i < displayValue; ++i) {
//    strip.setPixelColor(randomArr[i], 0, 0, 0);
//  }
//}



void batteryLife(void) {
  batteryMoniter = analogRead(A7);
  volts = map(batteryMoniter, 0, 1023, 0, 1683);
  voltsDec = volts / 100;
  for(int j = 0; j < lightValue* 2 / 3; ++j) {
    delay(20);
    for(int i = 10; i < LED_COUNT; ++i) {
      strip.setPixelColor(i, j, 0, 0);
    }
    strip.setBrightness(j);
    strip.show();
  }
  
  uint32_t cycle = 0;
  Serial.print("---");
  Serial.print(volts);
  while(displayBattery && cycle < 50000) {
    buttonCheck();
    cycle++;
    delay(100);
  }
  
  fadeOffGeneral();
  on = true;
  strip.clear();
}

//=================================================================boringLight

void lightAllWhite(uint8_t lightValue) {
  for(int i = 0; i < LED_COUNT; ++i) {
    strip.setPixelColor(i, lightValue / 2, lightValue, 0);
  }
}

void lightAllRed(uint8_t lightValue) {
  strip.clear();
  for(int i = 0; i < LED_COUNT; ++i) {
    strip.setPixelColor(i, 0, lightValue, 0);
  }
}

void fadeOffGeneral() {
  for(int i = 100; i > 0; --i) {
    knobbyBoi2 = analogRead(A3);
    lightValue = map(knobbyBoi2, 0, 1023, 20, 255);
    uint8_t fadeValue = lightValue * i / 100;
    strip.setBrightness(fadeValue);
    strip.show();
    delay(2);
  }
  strip.setBrightness(lightValue);
}

void fadeOffBoring() {
  for(int i = 100; i > 0; i--) {
    knobbyBoi2 = analogRead(A3);
    lightValue = map(knobbyBoi2, 0, 1023, 20, 255);
    uint8_t fadeValue = lightValue * i / 100;
    if(lightValue > 25) {
      lightAllWhite(fadeValue);
      boringDisplaySection();
      delay(2);
      strip.show();
    } else {
      lightAllRed(fadeValue);
      delay(2);
      strip.show();
    }
  } 
}

void fadeOffSlidy() {
  for(int i = 100; i > 0; --i) {
    knobbyBoi2 = analogRead(A3);
    lightValue = map(knobbyBoi2, 0, 1023, 20, 255);
    uint8_t fadeValue = lightValue * i / 100;
    if(lightValue > 25) {
      lightAllWhite(fadeValue);
      slidyDisplaySection();
      delay(2);
      strip.show();
    } else {
      lightAllRed(fadeValue);
      delay(2);
      strip.show();
    }
  } 
}


void fadeOnBoringLight() {
  for(int i = 0; i < 100; i++) {
    knobbyBoi2 = analogRead(A3);
    lightValue = map(knobbyBoi2, 0, 1023, 20, 255);
    uint8_t fadeValue = lightValue * i / 100;
    if(lightValue > 25) {
      lightAllWhite(fadeValue);
      boringDisplaySection();
      delay(2);
      strip.show();
    } else {
      lightAllRed(fadeValue);
      delay(2);
      strip.show();
    }
  } 
}


void fadeOnSlidyLight() {
  for(int i = 0; i < 100; i++) {
    knobbyBoi2 = analogRead(A3);
    lightValue = map(knobbyBoi2, 0, 1023, 20, 255);
    uint8_t fadeValue = lightValue * i / 100;
    if(lightValue > 25) {
      lightAllWhite(fadeValue);
      slidyDisplaySection();
      strip.show();
      delay(2);
    } else {
      lightAllRed(fadeValue);
      delay(2);
      strip.show();
    }
  } 
}


void boringLight(void) {
  fadeOnBoringLight();
  while(on) {
    knobbyBoi2 = analogRead(A3);
    lightValue = map(knobbyBoi2, 0, 1023, 20, 255);
    if(lightValue > 25) {
      for(int i = 0; i < LED_COUNT; ++i) {
        strip.setPixelColor(i, lightValue / 2, lightValue, 0);
        boringDisplaySection();
      }
    } else {
     for(int i = 0; i < LED_COUNT; ++i) {
        strip.setPixelColor(i, 0, lightValue, 0);
      }
    }
    brightnessCheck(lightValue);
    buttonCheck();
    strip.show();
  }
  fadeOffBoring();
  strip.clear();
}


void slidyLight(void) {
  fadeOnSlidyLight();
  while(on) {
    knobbyBoi2 = analogRead(A3);
    lightValue = map(knobbyBoi2, 0, 1023, 20, 255);
    if(lightValue > 25) {
      for(int i = 0; i < LED_COUNT; ++i) {
        strip.setPixelColor(i, lightValue / 2, lightValue, 0);
        slidyDisplaySection();
      }
    } else {
     for(int i = 0; i < LED_COUNT; ++i) {
        strip.setPixelColor(i, 0, lightValue, 0);
      }
    }
    brightnessCheck(lightValue);
    buttonCheck();
    strip.show();
  }
  fadeOffSlidy();
  strip.clear();
}


//==============================================================randomArrMaker

// swaps two uint32_t's
void swap(uint32_t &swap1, uint32_t &swap2) {
  uint32_t tempSwap = swap1;
  swap1 = swap2;
  swap2 = tempSwap;
}

// fills an array with numbers in numerical order from 0 to |pixelNum|
void fillArr(uint32_t pixelNum, uint32_t *arrToFill) {
  if (!arrToFill) {
    return;
  }
  for (int i = 0; i < pixelNum; ++i) {
    arrToFill[i] = i;
  }
}

// scrambles an array
void randomArrScrambler(uint32_t pixelNum, uint32_t *randomArr) {
  if (!randomArr) {
    return;
  }
  fillArr(pixelNum, randomArr);
  for (int i = 0; i < pixelNum; ++i) {
    int randomIndex = random(pixelNum);
    swap(randomArr[i], randomArr[randomIndex]);
  }
}

//==============================================================discoParty

uint32_t useColor(uint8_t color, uint8_t brightness) {
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
}

class DiscoDrop {

 public:

  // This cycles a DiscoDrop through from dim to bright and back to time. 
void party(void) {
  if(isVisible && startCountDown > delayStart) { 
    if(brightnessDir) {
      increaseBrightness();
    } else {
      decreaseBrightness();
    }
    if(brightness < 1 && !brightnessDir) {
      brightnessDir = true;
      isVisible = false;
      rate = random(1, 3);
      delayStart = random(400);
      changeColor();
      startCountDown = 0;
      strip.setPixelColor(location, 0, 0, 0);
      return;
    }
    strip.setPixelColor(location, useColor(color, brightness));
  }
  startCountDown++;
}

  // This sets the visibilty for a DiscoDrop.
void makeVisible(void) { isVisible = true; }

  // Sets the location for a DiscoDrop.
void setLocation(uint8_t newLocation) { location = newLocation; }

 // Increases brightness value by the rate value.
void increaseBrightness(void) { 
  brightness += rate; 
  if(brightness >= brightnessCap) {
    brightness = brightnessCap;
    brightnessDir = false;
  }
}

 // Decreases brightness value by the rate value.
void decreaseBrightness(void) { 
  brightness -= rate;
  if(brightness < 0) {
    brightness = 0;
  }
}

// Picks a new random color for you.
void changeColor(void) {
  color = random(5);
}

  private:
   bool isVisible = false;
   bool brightnessDir = true;
   uint32_t delayStart = random(200);
   uint32_t startCountDown = 0;
   uint16_t brightness = 0;
   uint8_t brightnessCap = 250;
   uint8_t rate = random(1, 3);
   uint8_t location;
   uint32_t color = 0;
 
};

void discoParty(void) {
  randomSeed(analogRead(A4));
  knobbyBoi1 = analogRead(A1);
  knobbyBoi2 = analogRead(A3);
  lightValue = map(knobbyBoi2, 0, 1023, 20, 255);
  strip.setBrightness(lightValue);
  uint32_t moreDropsTime = map(knobbyBoi1, 0, 1023, 5, 300);
  DiscoDrop drizzle[LED_COUNT];
  uint32_t cycle = 0;
  uint32_t numberOfDrops = 3;
  uint32_t randomArr[LED_COUNT];

  for(int i = 0; i < LED_COUNT; i++) {
    drizzle[i].setLocation(i);
  }

  for(int i = 0; i < numberOfDrops; ++i) {
    drizzle[random(LED_COUNT)].makeVisible();
  }
  
  while(on) {
    knobbyBoi1 = analogRead(A1);
    knobbyBoi2 = analogRead(A3);
    lightValue = map(knobbyBoi2, 0, 1023, 20, 255);
    moreDropsTime = map(knobbyBoi1, 0, 1023, 5, 300);
    for(int i = 0; i < LED_COUNT; ++i) {
      drizzle[i].party();
    }
    if(cycle % moreDropsTime == 0) {
      randomArrScrambler(LED_COUNT, randomArr);
      for(int i = 0; i < numberOfDrops; ++i) {
        drizzle[randomArr[i]].makeVisible();
      }
    }
    brightnessCheck(lightValue);
    buttonCheck();
    strip.show();
    delay(1);
    cycle++;;
  }
  fadeOffGeneral();
  strip.clear();
}


//==============================================================randColorFade

class colorMixer {

 public:
 
 private:
  
};
