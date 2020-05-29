#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 87

#define LED_CENTRE_BOTTOM 41
#define LED_CENTRE_LEFT 62
#define LED_TOP_LEFT 78
#define LED_CENTRE_RIGHT 19
#define LED_TOP_RIGHT 6
#define LED_STRIP_END 86

// signal from op amp
#define PICKUP_PIN A0
// control knobs
#define MODE_ROTARY_PIN A3
#define ADJ_POT_PIN A4
#define BRIGHT_POT_PIN A5

#define MAX_BRIGHTNESS 64
#define MAX_BRIGHTNESS_AUDIO 96

#define BRIGHTNESS_MODE_POT 0
#define BRIGHTNESS_MODE_AUDIO_PULSE 1
#define BRIGHTNESS_MODE_MAX 2

#define PATTERN_SOLID_COLOR 0
#define PATTERN_RAINBOW 1
#define PATTERN_RAINBOW_CYCLE 2
#define PATTERN_COLOR_WAVE 3
#define PATTERN_COLOR_WAVE_RAINBOW 4
#define PATTERN_COLOR_PULSE 5
#define PATTERN_GRADIENT_1 6
#define PATTERN_GRADIENT_2 7
#define PATTERN_RANDOM_PULSE 8

#define PATTERN_SOUND_PULSE_SOLID 9
#define PATTERN_SOUND_AMPLITUDE 10

#define RAINBOW_MIN_SAT 128

#define NUM_RAND_PULSE_LEDS 10

#define NUM_GRADIENT_COLORS 4
#define GRADIENT_GREEN_PURPLE 0
#define GRADIENT_AQUA_PURPLE 1
#define GRADIENT_PURPLE_PINK 2
#define GRADIENT_BLUE_AQUA 3

#define CLOCKWISE 0
#define ANTICLOCKWISE 1

#define ENERGY_HISTORY_SIZE 10
#define NUM_SAMPLES 256
#define BEAT_DETECTION_CONSTANT 13

#define PICKUP_NOISE_THRESHOLD 15
#define PICKUP_LOCAL_MAX_MILLIS 10

#define PICKUP_NUM_SAMPLES 20


extern const uint8_t gamma[];

CRGB leds[NUM_LEDS];

uint8_t activePattern;
uint32_t interval;
uint32_t lastUpdate;
uint8_t patternBrightnessMode = BRIGHTNESS_MODE_POT;

uint16_t totalSteps;
uint16_t curStep;

CHSV colorHSV1;
uint8_t randLeds[NUM_RAND_PULSE_LEDS];

uint8_t lastHue;

uint8_t gradHue1;
uint8_t gradHue2;

uint8_t curMode = 255;


uint16_t pickupBaseline = 1023;


void setup() {


    
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

    // this reduces flicker at low brightness levels
    FastLED.setDither(0);

    // random seed
    random16_add_entropy(analogRead(A1));

    FastLED.show();

    // calibratePickup();
}

void loop() {
    updateLedBrightness();

    updateMode();

    updatePattern();

    FastLED.show();
}

// reverses direction of pots
// use this instead of analogRead() for control knobs
uint16_t readPot(uint8_t pin) {
    return 1023 - analogRead(pin);
}

uint16_t readPotScaled(uint8_t pin, uint16_t max) {
    return map(readPot(pin), 0, 1023, 0, max);
}

uint16_t readPotScaled(uint8_t pin, uint16_t min, uint16_t max) {
    return map(readPot(pin), 0, 1023, min, max);
}

// returns mode from 0-11
uint8_t readMode() {
    uint16_t reading = readPot(MODE_ROTARY_PIN);
    return getModeFromReading(reading);
}

// get pos of rotary switch for mode
uint8_t getModeFromReading(uint16_t reading) {
    if (reading < 47) return 0;
    else if (reading < 140) return 1;
    else if (reading < 233) return 2;
    else if (reading < 326) return 3;
    else if (reading < 419) return 4;
    else if (reading < 512) return 5;
    else if (reading < 605) return 6;
    else if (reading < 698) return 7;
    else if (reading < 791) return 8;
    else if (reading < 884) return 9;
    else if (reading < 977) return 10;
    else return 11;
}

void updateMode() {
    uint8_t mode = readMode();
    if (mode != curMode) {
        // reset pickup baseline on mode change
        pickupBaseline = 1023;
        curMode = mode;
        // reset leds to off so they're not left on from the effect before
        fill_solid(&(leds[0]), NUM_LEDS, CRGB(0, 0, 0));
        switch (curMode) {
            case 0:
                randomPulseInit();
                break;
            case 1:
                colorWaveInit();
                break;
            case 2:
                solidColorInit(255);
                break;
            case 3:
                solidColorInit(178);
                break;
            case 4:
                soundPulseSolidInit();
                break;
            case 5:
                rainbowCycleInit(false);
                break;
            case 6:
                rainbowInit(true);
                break;
            case 7:
                rainbowCycleInit(true);
                break;
            case 8:
                gradient1Init(false);
                break;
            case 9:
                gradient2Init(false);
                break;
            case 10:
                gradient1Init(true);
                break;
            case 11:
                gradient2Init(true);
                break;
        }
    }
}

void updateLedBrightness() {
    if (patternBrightnessMode == BRIGHTNESS_MODE_AUDIO_PULSE) {
        uint8_t reading = getPickupReading();
        FastLED.setBrightness(map(reading, 0, 255, 0, MAX_BRIGHTNESS_AUDIO));
    } else if (patternBrightnessMode == BRIGHTNESS_MODE_POT) {
        uint8_t reading = readPotScaled(BRIGHT_POT_PIN, 255);
        uint8_t gammaCorrected = pgm_read_byte(&gamma[reading]);
        uint8_t scaledBrightness = map(gammaCorrected, 0, 255, 0, MAX_BRIGHTNESS);
        FastLED.setBrightness(scaledBrightness);
    } else {
        FastLED.setBrightness(MAX_BRIGHTNESS);
    }
}

void updatePattern() {
    if ((millis() - lastUpdate) > interval) {
        lastUpdate = millis();
        switch (activePattern) {
            case PATTERN_SOUND_AMPLITUDE:
                soundAmplitudeMeterUpdate();
                break;
            case PATTERN_SOUND_PULSE_SOLID:
                soundPulseSolidUpdate();
                break;
            case PATTERN_SOLID_COLOR:
                solidColorUpdate();
                break;
            case PATTERN_RAINBOW:
                rainbowUpdate();
                break;
            case PATTERN_RAINBOW_CYCLE:
                rainbowCycleUpdate();
                break;
            case PATTERN_COLOR_WAVE:
                colorWaveUpdate();
                break;
            case PATTERN_COLOR_WAVE_RAINBOW:
                colorWaveRainbowUpdate();
                break;
            case PATTERN_COLOR_PULSE:
                twoColorFadeUpdate();
                break;
            case PATTERN_RANDOM_PULSE:
                randomPulseUpdate();
                break;
            case PATTERN_GRADIENT_1:
                gradient1Update();
                break;
            case PATTERN_GRADIENT_2:
                gradient2Update();
                break;
            default:
                break;
        }
    }
}

void incrementStep() {
    curStep++;
    if (curStep >= totalSteps) {
        curStep = 0;
    }
}

void calibratePickup() {
    pickupBaseline = 1023;
    for (uint8_t i = 0; i < 100; i++) {
        uint16_t reading = analogRead(PICKUP_PIN);

        if (reading < pickupBaseline) {
            pickupBaseline = reading;
        }

        delay(10);
    }
}

uint8_t getPickupReading() {
    uint16_t reading = analogRead(PICKUP_PIN);

    if (reading < pickupBaseline) {
        pickupBaseline = reading;
    }

    if (reading > pickupBaseline) {
        reading -= pickupBaseline;
    } else {
        reading = 0;
    }

    if (reading > PICKUP_NOISE_THRESHOLD) {
        reading -= PICKUP_NOISE_THRESHOLD;
    } else {
        reading = 0;
    }

    if (reading > 255) reading = 255;

    return (uint8_t) reading;
}

void soundPulseSolidInit() {
    activePattern = PATTERN_SOUND_PULSE_SOLID;
    patternBrightnessMode = BRIGHTNESS_MODE_AUDIO_PULSE;
    interval = 20;
    colorHSV1 = CHSV(0, 255, 255);
}

void soundPulseSolidUpdate() {
    colorHSV1.hue = readPotScaled(ADJ_POT_PIN, 255);
    // use brightness pot to set saturation, brightness is set from pickup reading
    colorHSV1.sat = readPotScaled(BRIGHT_POT_PIN, 100, 255);

    fill_solid(&(leds[0]), NUM_LEDS, colorHSV1);
}


// probably don't use this pattern, works but doesn't look too good imo
void soundAmplitudeMeterInit() {
    activePattern = PATTERN_SOUND_AMPLITUDE;
    patternBrightnessMode = BRIGHTNESS_MODE_POT;
    interval = 20;
    colorHSV1 = CHSV(0, 0, 255);
}

void soundAmplitudeMeterUpdate() {

    uint8_t numLeds = map(getPickupReading(), 0, 255, 0, 50);
    fill_solid(&(leds[0]), NUM_LEDS, CRGB(0, 0, 0));
    for (uint8_t i = 0; i < numLeds; i++) {
        if (i >= 41) break;
        leds[40 + i] = colorHSV1;
        leds[40 - i] = colorHSV1;
        // fading edges
        if (i == numLeds - 3) {
            leds[40 + i] %= pgm_read_byte(&gamma[192]);
            leds[40 - i] %= pgm_read_byte(&gamma[192]);
        } else if (i == numLeds - 2) {
            leds[40 + i] %= pgm_read_byte(&gamma[128]);
            leds[40 - i] %= pgm_read_byte(&gamma[128]);
        } else if (i == numLeds - 1) {
            leds[40 + i] %= pgm_read_byte(&gamma[64]);
            leds[40 - i] %= pgm_read_byte(&gamma[64]);
        } else if (i == numLeds) {
            leds[40 + i] %= pgm_read_byte(&gamma[32]);
            leds[40 - i] %= pgm_read_byte(&gamma[32]);
        }
    }
}

void solidColorInit(uint8_t sat) {
    activePattern = PATTERN_SOLID_COLOR;
    patternBrightnessMode = BRIGHTNESS_MODE_POT;
    colorHSV1 = CHSV(0, sat, 255);
}

void solidColorUpdate() {
    colorHSV1.hue = readPotScaled(ADJ_POT_PIN, 255);
    fill_solid(&(leds[0]), NUM_LEDS, colorHSV1);
}

void rainbowInit(bool soundPulse) {
    activePattern = PATTERN_RAINBOW;
    if (soundPulse) {
        patternBrightnessMode = BRIGHTNESS_MODE_AUDIO_PULSE;
    } else {
        patternBrightnessMode = BRIGHTNESS_MODE_POT;
    }
    colorHSV1 = CHSV(0, 255, 255);
}

void rainbowUpdate() {
    colorHSV1.sat = readPotScaled(ADJ_POT_PIN, RAINBOW_MIN_SAT, 255);
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        colorHSV1.hue = map(led, 0, NUM_LEDS, 0, 255);
        leds[led] = colorHSV1;
    }
}

void rainbowCycleInit(bool soundPulse) {
    activePattern = PATTERN_RAINBOW_CYCLE;
    if (soundPulse) {
        patternBrightnessMode = BRIGHTNESS_MODE_AUDIO_PULSE;
    } else {
        patternBrightnessMode = BRIGHTNESS_MODE_POT;
    }
    colorHSV1 = CHSV(0, 255, 255);
    totalSteps = 256;
    curStep = 0;
    interval = 60;
}

void rainbowCycleUpdate() {
    colorHSV1.sat = readPotScaled(ADJ_POT_PIN, RAINBOW_MIN_SAT, 255);
    if (interval == 50) interval = 10000;
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        colorHSV1.hue = map(led, 0, NUM_LEDS, 0, 255) + curStep;
        leds[led] = colorHSV1;
    }
    incrementStep();
}

void colorWaveInit() {
    activePattern = PATTERN_COLOR_WAVE;
    patternBrightnessMode = BRIGHTNESS_MODE_MAX;
    colorHSV1 = CHSV(0, 255, 255);
    totalSteps = 256;
    curStep = 0;
    interval = 30;
}

void colorWaveUpdate() {
    colorHSV1.sat = readPotScaled(BRIGHT_POT_PIN, 255);
    colorHSV1.hue = readPotScaled(ADJ_POT_PIN, 255);
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        uint16_t offset = map(led, 0, NUM_LEDS, 0, 2047);
        // colorHSV1.val = cubicwave8(curStep + offset);
        uint8_t val = map(cubicwave8(curStep + offset), 0, 255, 128, 255);
        colorHSV1.val = pgm_read_byte(&gamma[val]);
        leds[led] = colorHSV1;
    }

    incrementStep();
}

void colorWaveRainbowInit() {
    activePattern = PATTERN_COLOR_WAVE_RAINBOW;
    patternBrightnessMode = BRIGHTNESS_MODE_POT;
    colorHSV1 = CHSV(0, 255, 255);
    totalSteps = 256;
    curStep = 0;
    interval = 30;
}

void colorWaveRainbowUpdate() {
    colorHSV1.sat = readPotScaled(ADJ_POT_PIN, RAINBOW_MIN_SAT, 255);
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        uint16_t offset = map(led, 0, NUM_LEDS, 0, 2047);
        // colorHSV1.val = cubicwave8(curStep + offset);
        // uint8_t val = cubicwave8(curStep + offset);
        uint8_t val = map(cubicwave8(curStep + offset), 0, 255, 128, 255);
        colorHSV1.val = pgm_read_byte(&gamma[val]);

        colorHSV1.hue = map(led, 0, NUM_LEDS, 0, 255) + curStep;
        leds[led] = colorHSV1;
    }

    incrementStep();
}

void twoColorFadeInit(bool soundPulse) {
    activePattern = PATTERN_COLOR_PULSE;
    if (soundPulse) {
        patternBrightnessMode = BRIGHTNESS_MODE_AUDIO_PULSE;
    } else {
        patternBrightnessMode = BRIGHTNESS_MODE_POT;
    }
    colorHSV1 = CHSV(0, 255, 255);
    totalSteps = 256;
    curStep = 0;
    interval = 20;
}

void twoColorFadeUpdate() {
    uint8_t hue = readPotScaled(ADJ_POT_PIN, 255);
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        uint16_t waveOffset = map(led, 0, NUM_LEDS, 0, 511);
        uint8_t wave = quadwave8(curStep + waveOffset);

        colorHSV1.hue = hue + map(wave, 0, 255, 0, 50);
        leds[led] = colorHSV1;
    }
    incrementStep();
}

void randomPulseInit() {
    activePattern = PATTERN_RANDOM_PULSE;
    patternBrightnessMode = BRIGHTNESS_MODE_MAX;
    colorHSV1 = CHSV(0, 255, 255);
    totalSteps = 256;
    curStep = 0;
    interval = 10;
    for (uint8_t i = 0; i < NUM_RAND_PULSE_LEDS; i++) {
        randLeds[i] = random8(NUM_LEDS);
    }
}

void randomPulseUpdate() {
    colorHSV1.sat = readPotScaled(BRIGHT_POT_PIN, 255);

    colorHSV1.hue = readPotScaled(ADJ_POT_PIN, 255);

    if (colorHSV1.hue != lastHue) {
        lastHue = colorHSV1.hue;
        colorHSV1.val = 64;
        fill_solid(&(leds[0]), NUM_LEDS, colorHSV1);
    }

    for (uint8_t led = 0; led < NUM_RAND_PULSE_LEDS; led++) {
        uint8_t waveOffset = map(led, 0, NUM_RAND_PULSE_LEDS, 0, 255);

        uint8_t wave = quadwave8(curStep + waveOffset);
        
        if ((curStep + waveOffset) == 256) {
            uint8_t newRandLed = random8(NUM_LEDS);
            bool validLedPos = false;
            while (!validLedPos) {
                validLedPos = true;
                for (uint8_t i = 0; i < NUM_RAND_PULSE_LEDS; i++) {
                    if (i == led) continue;
                    uint8_t ledGap = randLeds[i] > newRandLed ? randLeds[i] - newRandLed : newRandLed - randLeds[i];
                    if (ledGap < 7) {
                        newRandLed = random8(NUM_LEDS);
                        validLedPos = false;
                        break;
                    }
                }
            }
            randLeds[led] = newRandLed;
        }

        colorHSV1.val = pgm_read_byte(&gamma[wave]);
        leds[randLeds[led]] = colorHSV1;

        if (randLeds[led] > 0) {
            leds[randLeds[led] - 1] = colorHSV1;
        }
        if (randLeds[led] < NUM_LEDS - 1) {
            leds[randLeds[led] + 1] = colorHSV1;
        }
        if (randLeds[led] > 1) {
            leds[randLeds[led] - 2] = colorHSV1;
            leds[randLeds[led] - 2] %= pgm_read_byte(&gamma[128]);
        }
        if (randLeds[led] < NUM_LEDS - 2) {
            leds[randLeds[led] + 2] = colorHSV1;
            leds[randLeds[led] + 2] %= pgm_read_byte(&gamma[128]);
        }
        if (randLeds[led] > 2) {
            leds[randLeds[led] - 3] = colorHSV1;
            leds[randLeds[led] - 3] %= pgm_read_byte(&gamma[64]);
        }
        if (randLeds[led] < NUM_LEDS - 3) {
            leds[randLeds[led] + 3] = colorHSV1;
            leds[randLeds[led] + 3] %= pgm_read_byte(&gamma[64]);
        }
    }

    colorHSV1.val = 64;

    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        leds[led] |= colorHSV1;
    }

    incrementStep();
}

void gradient1Init(bool soundPulse) {
    activePattern = PATTERN_GRADIENT_1;
    if (soundPulse) {
        patternBrightnessMode = BRIGHTNESS_MODE_AUDIO_PULSE;
    } else {
        patternBrightnessMode = BRIGHTNESS_MODE_POT;
    }
}

void gradient1Update() {
    setGradientColors();

    fill_gradient(&(leds[0]), LED_TOP_RIGHT, CHSV(gradHue1, 255, 255), LED_CENTRE_BOTTOM, CHSV(gradHue2, 255, 255));
    fill_gradient(&(leds[0]), LED_CENTRE_BOTTOM, CHSV(gradHue2, 255, 255), LED_TOP_LEFT, CHSV(gradHue1, 255, 255));

    mirrorAtEnds();
}

void gradient2Init(bool soundPulse) {
    activePattern = PATTERN_GRADIENT_2;
    if (soundPulse) {
        patternBrightnessMode = BRIGHTNESS_MODE_AUDIO_PULSE;
    } else {
        patternBrightnessMode = BRIGHTNESS_MODE_POT;
    }
}

void gradient2Update() {
    setGradientColors();

    fill_gradient(&(leds[0]), LED_TOP_RIGHT, CHSV(gradHue1, 255, 255), LED_CENTRE_RIGHT, CHSV(gradHue2, 255, 255));
    fill_gradient(&(leds[0]), LED_CENTRE_RIGHT, CHSV(gradHue2, 255, 255), LED_CENTRE_BOTTOM, CHSV(gradHue1, 255, 255));
    fill_gradient(&(leds[0]), LED_CENTRE_BOTTOM, CHSV(gradHue1, 255, 255), LED_CENTRE_LEFT, CHSV(gradHue2, 255, 255));
    fill_gradient(&(leds[0]), LED_CENTRE_LEFT, CHSV(gradHue2, 255, 255), LED_TOP_LEFT, CHSV(gradHue1, 255, 255));

    mirrorAtEnds();
}

void setGradientColors() {
    uint8_t colors = readPotScaled(ADJ_POT_PIN, NUM_GRADIENT_COLORS);

    switch (colors) {
        case GRADIENT_GREEN_PURPLE:
            gradHue1 = 96;
            gradHue2 = 192;
            break;
        case GRADIENT_AQUA_PURPLE:
            gradHue1 = 116;
            gradHue2 = 192;
            break;
        case GRADIENT_PURPLE_PINK:
            gradHue1 = 208;
            gradHue2 = 250;
            break;
        case GRADIENT_BLUE_AQUA:
            gradHue1 = 160;
            gradHue2 = 128;
            break;
        default:
            break;
    }
}

void rotateLeds(uint8_t direction) {
    if (direction == CLOCKWISE) {
        CRGB overflowColor = leds[NUM_LEDS - 1];
        for (uint8_t i = 1; i < NUM_LEDS; i++) {
            leds[NUM_LEDS - i] = leds[NUM_LEDS - i - 1];
        }
        leds[0] = overflowColor;
    } else {
        CRGB underflowColor = leds[0];
        for (uint8_t i = 0; i < NUM_LEDS - 1; i++) {
            leds[i] = leds[i + 1];
        }
        leds[NUM_LEDS - 1] = underflowColor;
    }
}

void mirrorAtEnds() {
    for (uint8_t i = 0; i < LED_STRIP_END - LED_TOP_LEFT; i++) {
        leds[LED_TOP_LEFT + i + 1] = leds[LED_TOP_LEFT - i - 1];
    }

    for (uint8_t i = 0; i < LED_TOP_RIGHT; i++) {
        leds[LED_TOP_RIGHT - i - 1] = leds[LED_TOP_RIGHT + i + 1];
    }
}




const PROGMEM uint8_t gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
    10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
    17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
    25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
    37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
    51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
    69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
    90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
    115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
    144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
    177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
    215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255};