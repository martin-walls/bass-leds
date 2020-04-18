#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 87
#define MAX_BRIGHTNESS 64

// signal from op amp
#define PICKUP_PIN A0
// control knobs
#define MODE_ROTARY_PIN A3
#define ADJ_POT_PIN A4
#define BRIGHT_POT_PIN A5

#define PATTERN_SOLID_COLOR 0
#define PATTERN_RAINBOW 1
#define PATTERN_RAINBOW_CYCLE 2
#define PATTERN_COLOR_WAVE 3
#define PATTERN_COLOR_WAVE_RAINBOW 4
#define PATTERN_COLOR_PULSE 5
#define PATTERN_SOUND_AMPLITUDE 6
#define PATTERN_SOUND_PULSE 7

#define RAINBOW_MIN_SAT 128

#define WAVE_INTERVAL 20

#define ENERGY_HISTORY_SIZE 10
#define NUM_SAMPLES 256
#define BEAT_DETECTION_CONSTANT 13

extern const uint8_t gamma[];

CRGB leds[NUM_LEDS];

uint8_t activePattern;
uint32_t interval;
uint32_t lastUpdate;

uint16_t totalSteps;
uint16_t curStep;

CHSV colorHSV1;

uint8_t curMode;


uint8_t pickupBaseline;
uint8_t pickupLocalMax;
uint8_t pickupValue;

uint16_t energyHistory[ENERGY_HISTORY_SIZE];
// uint16_t sampleHistoryHead = 0;
uint16_t newSamples[NUM_SAMPLES];
uint16_t sampleIndex = 0;

void setup() {
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

    // this reduces flicker at low brightness levels -- test whether needed on battery power
    FastLED.setDither(0);

    // solidColorInit(0);
    // rainbowInit();
    // rainbowCycleInit();
    // colorWaveInit(255);
    // colorWaveRainbowInit();
}

void loop() {
    updateLedBrightness();

    updateMode();

    updatePattern();

    updatePickupReading();

    // readPickup();
    // if (sampleIndex == 0) {
    //     if (detectBeat()) {
            // fill_solid(&(leds[0]), NUM_LEDS, CRGB(255, 255, 255));
    //     } else {
    //         fill_solid(&(leds[0]), NUM_LEDS, CRGB(0, 0, 0));
    //     }
    // }

    // uint16_t reading = analogRead(PICKUP_PIN);
    // uint8_t val = map(reading, 0, 1023, 0, 255);
    // uint8_t gammaCorrected = pgm_read_byte(&gamma[pickupValue]);
    // uint8_t scaledBrightness = map(gammaCorrected, 0, 255, 0, MAX_BRIGHTNESS);
    // FastLED.setBrightness(scaledBrightness);


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
        curMode = mode;
        switch (curMode) {
            case 0:
                solidColorInit(255);
                break;
            case 1:
                solidColorInit(192);
                break;
            case 2:
                solidColorInit(128);
                break;
            case 3:
                rainbowInit();
                break;
            case 4:
                rainbowCycleInit();
                break;
            case 5:
                colorWaveInit(255);
                break;
            case 6:
                colorWaveRainbowInit();
                break;
            case 7:
                colorPulseInit();
                break;
            case 8:
                soundAmplitudeInit();
                break;
            case 9:
                soundPulseInit();
                break;
            case 10:
                break;
            case 11:
                break;
        }
    }
}

void updateLedBrightness() {
    uint8_t reading = readPotScaled(BRIGHT_POT_PIN, 255);
    uint8_t gammaCorrected = pgm_read_byte(&gamma[reading]);
    uint8_t scaledBrightness = map(gammaCorrected, 0, 255, 0, MAX_BRIGHTNESS);
    FastLED.setBrightness(scaledBrightness);
}

void updatePattern() {
    if ((millis() - lastUpdate) > interval) {
        lastUpdate = millis();
        switch (activePattern) {
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
                colorPulseUpdate();
                break;
            case PATTERN_SOUND_AMPLITUDE:
                soundAmplitudeUpdate();
                break;
            case PATTERN_SOUND_PULSE:
                soundPulseUpdate();
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

void solidColorInit(uint8_t sat) {
    activePattern = PATTERN_SOLID_COLOR;
    colorHSV1 = CHSV(0, sat, 255);
}

void solidColorUpdate() {
    colorHSV1.hue = readPotScaled(ADJ_POT_PIN, 255);
    fill_solid(&(leds[0]), NUM_LEDS, colorHSV1);
}

void rainbowInit() {
    activePattern = PATTERN_RAINBOW;
    colorHSV1 = CHSV(0, 255, 255);
}

void rainbowUpdate() {
    colorHSV1.sat = readPotScaled(ADJ_POT_PIN, RAINBOW_MIN_SAT, 255);
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        colorHSV1.hue = map(led, 0, NUM_LEDS, 0, 255);
        leds[led] = colorHSV1;
    }
}

void rainbowCycleInit() {
    activePattern = PATTERN_RAINBOW_CYCLE;
    colorHSV1 = CHSV(0, 255, 255);
    totalSteps = 256;
    curStep = 0;
    interval = 20;
}

void rainbowCycleUpdate() {
    colorHSV1.sat = readPotScaled(ADJ_POT_PIN, RAINBOW_MIN_SAT, 255);
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        colorHSV1.hue = map(led, 0, NUM_LEDS, 0, 255) + curStep;
        leds[led] = colorHSV1;
    }
    incrementStep();
}

void colorWaveInit(uint8_t sat) {
    activePattern = PATTERN_COLOR_WAVE;
    colorHSV1 = CHSV(0, sat, 255);
    totalSteps = 256;
    curStep = 0;
    interval = WAVE_INTERVAL;
}

void colorWaveUpdate() {
    colorHSV1.hue = readPotScaled(ADJ_POT_PIN, 255);
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        uint16_t offset = map(led, 0, NUM_LEDS, 0, 2047);
        colorHSV1.val = cubicwave8(curStep + offset);
        leds[led] = colorHSV1;
    }

    incrementStep();
}

void colorWaveRainbowInit() {
    activePattern = PATTERN_COLOR_WAVE_RAINBOW;
    colorHSV1 = CHSV(0, 255, 255);
    totalSteps = 256;
    curStep = 0;
    interval = WAVE_INTERVAL;
}

void colorWaveRainbowUpdate() {
    colorHSV1.sat = readPotScaled(ADJ_POT_PIN, RAINBOW_MIN_SAT, 255);
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        uint16_t offset = map(led, 0, NUM_LEDS, 0, 2047);
        colorHSV1.val = cubicwave8(curStep + offset);

        colorHSV1.hue = map(led, 0, NUM_LEDS, 0, 255) + curStep;
        leds[led] = colorHSV1;
    }

    incrementStep();
}

void colorPulseInit() {
    activePattern = PATTERN_COLOR_PULSE;
    colorHSV1 = CHSV(0, 255, 255);
    totalSteps = 256;
    curStep = 0;
    interval = 20;
}

void colorPulseUpdate() {
    uint8_t hue = readPotScaled(ADJ_POT_PIN, 255);
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        uint16_t waveOffset = map(led, 0, NUM_LEDS, 0, 511);
        uint8_t wave = quadwave8(curStep + waveOffset);

        colorHSV1.hue = hue + map(wave, 0, 255, 0, 50);
        leds[led] = colorHSV1;
    }
    incrementStep();
}

uint8_t readPickup() {
    return map(analogRead(PICKUP_PIN), 0, 1023, 0, 255);
}

// sound reactive modes
void updatePickupReading() {
    uint8_t reading = readPickup();
    if (reading < pickupBaseline) {
        pickupBaseline = reading;
        pickupValue = 0;
    } else if (reading > pickupLocalMax) {
        pickupLocalMax = reading;
        pickupValue = 255;
    } else {
        scale8(pickupLocalMax, 250);
        if (pickupLocalMax <= pickupBaseline) {
            pickupLocalMax = pickupBaseline + 1;
        }
        reading -= pickupBaseline;
        pickupValue = map(reading, 0, pickupLocalMax - pickupBaseline, 0, 255);
    }

    // pickupValue = readPickup();



    // newSamples[sampleIndex] = reading;
    // sampleIndex++;
    // if (sampleIndex >= NUM_SAMPLES) sampleIndex = 0;
}

bool detectBeat() {
    uint32_t instantEnergy = 0;
    for (uint16_t i = 0; i < NUM_SAMPLES; i++) {
        instantEnergy += newSamples[i] * newSamples[i];
    }

    uint32_t localAverageEnergy = 0;
    for (uint8_t i = 0; i < ENERGY_HISTORY_SIZE; i++) {
        localAverageEnergy += (energyHistory[i] * energyHistory[i]) / ENERGY_HISTORY_SIZE;
    }

    // uint32_t energyVariance = 0;
    // for (uint8_t i = 0; i < ENERGY_HISTORY_SIZE; i++) {
    //     energyVariance += (energyHistory[i] - localAverageEnergy) * (energyHistory[i] - localAverageEnergy) / 10;
    // }

    for (uint8_t i = 1; i < ENERGY_HISTORY_SIZE; i++) {
        energyHistory[ENERGY_HISTORY_SIZE - i] = energyHistory[ENERGY_HISTORY_SIZE - i - 1];
    }

    energyHistory[0] = instantEnergy;

    // sampleHistoryHead += 1024;
    // if (sampleHistoryHead >= SAMPLE_HISTORY_SIZE) {
    //     sampleHistoryHead -= SAMPLE_HISTORY_SIZE;
    // }

    // if (sampleHistoryHead >= 1024) {
    //     memcpy(&(sampleHistory[sampleHistoryHead - 1024]), &(newSamples[0]), sizeof(newSamples));
    // } else {
    //     memcpy(&(sampleHistory[sampleHistoryHead + SAMPLE_HISTORY_SIZE - 1024]), &(newSamples[0]), 2 * (1024 - sampleHistoryHead));
    //     memcpy(&(sampleHistory[0]), &(newSamples[sampleHistoryHead]), 2 * sampleHistoryHead);
    // }

    return instantEnergy * 10 > localAverageEnergy * BEAT_DETECTION_CONSTANT;
}

void soundAmplitudeInit() {
    activePattern = PATTERN_SOUND_AMPLITUDE;
    interval = 20;
    colorHSV1 = CHSV(0, 0, 255);
}

void soundAmplitudeUpdate() {
    // replace this with pickup reading
    uint8_t numLeds = readPotScaled(ADJ_POT_PIN, 45);
    fill_solid(&(leds[0]), NUM_LEDS, CRGB(0, 0, 0));
    for (uint8_t i = 0; i < numLeds; i++) {
        if (i >= 45) break;
        leds[43 + i] = colorHSV1;
        leds[43 - i] = colorHSV1;
        // fading edges
        if (i == numLeds - 3) {
            leds[43 + i] %= pgm_read_byte(&gamma[192]);
            leds[43 - i] %= pgm_read_byte(&gamma[192]);
        } else if (i == numLeds - 2) {
            leds[43 + i] %= pgm_read_byte(&gamma[128]);
            leds[43 - i] %= pgm_read_byte(&gamma[128]);
        } else if (i == numLeds - 1) {
            leds[43 + i] %= pgm_read_byte(&gamma[64]);
            leds[43 - i] %= pgm_read_byte(&gamma[64]);
        } else if (i == numLeds) {
            leds[43 + i] %= pgm_read_byte(&gamma[32]);
            leds[43 - i] %= pgm_read_byte(&gamma[32]);
        }
    }
}

void soundPulseInit() {
    activePattern = PATTERN_SOUND_PULSE;
    interval = 20;
    // use adj pot to change color instead
    colorHSV1 = CHSV(0, 0, 255);
}

void soundPulseUpdate() {
    // replace with pickup reading
    // only want first half of wave (up to top)
    uint8_t reading = readPotScaled(ADJ_POT_PIN, 128);
    // map wave fn upwards so theres always a min brightness
    uint8_t val = map(quadwave8(reading), 0, 255, 128, 255);
    uint8_t gammaVal = pgm_read_byte(&gamma[val]);
    colorHSV1.val = gammaVal;
    fill_solid(&(leds[0]), NUM_LEDS, colorHSV1);
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