#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 87

// signal from op amp
#define PICKUP_PIN A0
// control knobs
#define MODE_ROTARY_PIN A3
#define ADJ_POT_PIN A4
#define BRIGHT_POT_PIN A5

#define MAX_BRIGHTNESS 64
#define MAX_BRIGHTNESS_AUDIO 64
#define BASELINE_BRIGHTNESS 128

#define AUDIO_MODE_OFF 0
#define AUDIO_MODE_PULSE 1
#define AUDIO_MODE_OTHER 2

#define PATTERN_SOLID_COLOR 0
#define PATTERN_RAINBOW 1
#define PATTERN_RAINBOW_CYCLE 2
#define PATTERN_COLOR_WAVE 3
#define PATTERN_COLOR_WAVE_RAINBOW 4
#define PATTERN_COLOR_PULSE 5
#define PATTERN_SOUND_AMPLITUDE 6
#define PATTERN_SOUND_PULSE 7
#define PATTERN_RANDOM_PULSE 8
#define PATTERN_GRADIENT_1 9
#define PATTERN_GRADIENT_2 10

#define RAINBOW_MIN_SAT 128

#define WAVE_INTERVAL 20

#define NUM_RAND_PULSE_LEDS 8

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
// bool isAudioPattern = false;
uint8_t patternAudioMode = AUDIO_MODE_OFF;

uint16_t totalSteps;
uint16_t curStep;

CHSV colorHSV1;
uint8_t randLeds[NUM_RAND_PULSE_LEDS];
uint8_t gradHue1;
uint8_t gradHue2;

uint8_t curMode;


uint16_t pickupBaseline = 1023;
// uint16_t pickupLocalMax;
uint16_t pickupValue;

uint32_t lastPickupLocalMax;


uint16_t samples[PICKUP_NUM_SAMPLES];
uint8_t sampleIndex = 0;

uint16_t lastReading;


uint8_t pickupUpdateValue;
uint32_t pickupUpdateValueLastUpdate;

uint8_t pickupLocalMax;

uint8_t pickupFadeValue;
uint8_t pickupFadeValueLastUpdate;



uint16_t energyHistory[ENERGY_HISTORY_SIZE];
// uint16_t sampleHistoryHead = 0;
uint16_t newSamples[NUM_SAMPLES];
// uint16_t sampleIndex = 0;

void setup() {


    
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

    // this reduces flicker at low brightness levels -- test whether needed on battery power
    FastLED.setDither(0);

    // random seed
    random16_add_entropy(analogRead(A1));




    // white for testing
    // fill_solid(&(leds[0]), NUM_LEDS, CRGB(255, 255, 255));

    // soundAmplitudeInit();

    // Serial.begin(115200);
}

void loop() {



    
    updateLedBrightness();

    updateMode();

    updatePattern();
    




    

    // uint16_t reading = readPickup();




    // if (reading < pickupBaseline) {
    //     pickupBaseline = reading;
    // }

    // // move down to 0
    // reading -= pickupBaseline;

    // // remove noise
    // if (reading > PICKUP_NOISE_THRESHOLD) {
    //     reading -= PICKUP_NOISE_THRESHOLD;
    // } else {
    //     reading = 0;
    // }

    // if (reading > 255) reading = 255;

    // fill_solid(&(leds[0]), NUM_LEDS, CRGB(reading, reading, reading));

    // uint8_t gammaCorrect = pgm_read_byte(&gamma[reading]);
    // uint8_t scaledBright = map(reading, 0, 255, 0, MAX_BRIGHTNESS_AUDIO);
    // FastLED.setBrightness(scaledBright);




    // Serial.println(reading);



    // now from 0 - ~60

    // samples[sampleIndex] = reading;
    // sampleIndex++;
    // if (sampleIndex >= PICKUP_NUM_SAMPLES) sampleIndex = 0;

    // pickupValue = ((pickupValue * 7) + reading) >> 3;

    // if (pickupValue > pickupLocalMax || millis() - lastPickupLocalMax > 5) {
    //     pickupLocalMax = ((pickupLocalMax * 3) + pickupValue) >> 2;
    //     lastPickupLocalMax = millis();
    // }

    // if (reading - pickupBaseline > PICKUP_NOISE_THRESHOLD) {
    //     uint16_t correctedReading = map(reading - pickupBaseline - PICKUP_NOISE_THRESHOLD, 0, 60, 0, 255);
    //     if (correctedReading > pickupValue) {
    //         pickupValue = correctedReading;
    //     } else {
    //         if ((millis() - lastPickupLocalMax) > PICKUP_LOCAL_MAX_MILLIS) {
    //             lastPickupLocalMax = millis();
    //             pickupValue = scale8(pickupValue, 240);
    //         }
    //     }
    // } else {
    //     if ((millis() - lastPickupLocalMax) > PICKUP_LOCAL_MAX_MILLIS) {
    //         lastPickupLocalMax = millis();
    //         pickupValue = scale8(pickupValue, 254);
    //     }
    // }

    // corReading = map(corReading, 0, 60, 0, 255);

    // uint16_t maxSample = 0;
    // uint16_t sum;
    // for (uint8_t i = 0; i < PICKUP_NUM_SAMPLES; i++) {
    //     // if (samples[i] > maxSample) {
    //     //     maxSample = samples[i];
    //     // }
    //     sum += samples[i];
    // } 

    // uint16_t avg = sum / PICKUP_NUM_SAMPLES;


/*


    uint16_t outVal = ((lastReading * 15) + reading) >> 4;
    lastReading = reading;


*/


    // uint8_t gammaCorrect = pgm_read_byte(&gamma[map(outVal, 0, 60, 0, 255)]);
    // uint8_t scaledBright = map(gammaCorrect, 0, 255, 0, MAX_BRIGHTNESS);
    // FastLED.setBrightness(scaledBright);
    

/*


    uint8_t scaledOutVal = map(reading, 0, 60, 0, 255);



    if (millis() - pickupFadeValueLastUpdate > 10) {
        pickupFadeValue -= 10;
    }

    if (scaledOutVal > pickupFadeValue) {
        pickupFadeValue = scaledOutVal;
    }


*/


    // if (scaledOutVal > pickupLocalMax) {
    //     pickupLocalMax = scaledOutVal;
    // }
    // if (millis() - pickupUpdateValueLastUpdate > 50) {
    //     pickupUpdateValueLastUpdate = millis();
    //     pickupUpdateValue = pickupLocalMax;
    //     pickupFadeValue = pickupUpdateValue;
    //     pickupLocalMax = 0;
    // }

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
        fill_solid(&(leds[0]), NUM_LEDS, CRGB(0, 0, 0));
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
                randomPulseInit();
                break;
            case 11:
                // gradient2Init();
                soundPulseInit();
                break;
        }
    }
}

void updateLedBrightness() {
    if (patternAudioMode == AUDIO_MODE_PULSE) {
        uint8_t reading = getPickupReading();
        FastLED.setBrightness(map(reading, 0, 255, 0, MAX_BRIGHTNESS_AUDIO));
    } else {
        uint8_t reading = readPotScaled(BRIGHT_POT_PIN, 255);
        uint8_t gammaCorrected = pgm_read_byte(&gamma[reading]);
        uint8_t scaledBrightness = map(gammaCorrected, 0, 255, 0, MAX_BRIGHTNESS);
        FastLED.setBrightness(scaledBrightness);
    }
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

// sound reactive modes

uint8_t getPickupReading() {
    uint16_t reading = analogRead(PICKUP_PIN);

    if (reading < pickupBaseline) {
        pickupBaseline = reading;
    }

    reading -= pickupBaseline;

    if (reading > PICKUP_NOISE_THRESHOLD) {
        reading -= PICKUP_NOISE_THRESHOLD;
    } else {
        reading = 0;
    }

    if (reading > 255) reading = 255;

    return (uint8_t) reading;
}

void soundPulseInit() {
    activePattern = PATTERN_SOUND_PULSE;
    patternAudioMode = AUDIO_MODE_PULSE;
    
    colorHSV1 = CHSV(0, 255, 255);
}

void soundPulseUpdate() {
    colorHSV1.hue = readPotScaled(ADJ_POT_PIN, 255);
    // use brightness pot to set saturation, brightness is set from pickup reading
    colorHSV1.sat = readPotScaled(BRIGHT_POT_PIN, 255);

    fill_solid(&(leds[0]), NUM_LEDS, colorHSV1);
}

void soundAmplitudeInit() {
    activePattern = PATTERN_SOUND_AMPLITUDE;
    patternAudioMode = AUDIO_MODE_OTHER;
    interval = 20;
    colorHSV1 = CHSV(0, 0, 255);
}

void soundAmplitudeUpdate() {
    // replace this with pickup reading
    // uint8_t numLeds = readPotScaled(ADJ_POT_PIN, 45);
    uint8_t numLeds = map(pickupFadeValue, 0, 255, 0, 39);
    fill_solid(&(leds[0]), NUM_LEDS, CRGB(0, 0, 0));
    for (uint8_t i = 0; i < numLeds; i++) {
        if (i >= 40) break;
        leds[39 + i] = colorHSV1;
        leds[39 - i] = colorHSV1;
        // fading edges
        if (i == numLeds - 3) {
            leds[39 + i] %= pgm_read_byte(&gamma[192]);
            leds[39 - i] %= pgm_read_byte(&gamma[192]);
        } else if (i == numLeds - 2) {
            leds[39 + i] %= pgm_read_byte(&gamma[128]);
            leds[39 - i] %= pgm_read_byte(&gamma[128]);
        } else if (i == numLeds - 1) {
            leds[39 + i] %= pgm_read_byte(&gamma[64]);
            leds[39 - i] %= pgm_read_byte(&gamma[64]);
        } else if (i == numLeds) {
            leds[39 + i] %= pgm_read_byte(&gamma[32]);
            leds[39 - i] %= pgm_read_byte(&gamma[32]);
        }
    }
}


// other modes

void solidColorInit(uint8_t sat) {
    activePattern = PATTERN_SOLID_COLOR;
    patternAudioMode = AUDIO_MODE_OFF;
    colorHSV1 = CHSV(0, sat, 255);
}

void solidColorUpdate() {
    colorHSV1.hue = readPotScaled(ADJ_POT_PIN, 255);
    fill_solid(&(leds[0]), NUM_LEDS, colorHSV1);
}

void rainbowInit() {
    activePattern = PATTERN_RAINBOW;
    patternAudioMode = AUDIO_MODE_OFF;
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
    patternAudioMode = AUDIO_MODE_OFF;
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
    patternAudioMode = AUDIO_MODE_OFF;
    colorHSV1 = CHSV(0, sat, 255);
    totalSteps = 256;
    curStep = 0;
    interval = WAVE_INTERVAL;
}

void colorWaveUpdate() {
    colorHSV1.hue = readPotScaled(ADJ_POT_PIN, 255);
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        uint16_t offset = map(led, 0, NUM_LEDS, 0, 2047);
        // colorHSV1.val = cubicwave8(curStep + offset);
        uint8_t val = map(cubicwave8(curStep + offset), 0, 255, BASELINE_BRIGHTNESS, 255);
        colorHSV1.val = pgm_read_byte(&gamma[val]);
        leds[led] = colorHSV1;
    }

    incrementStep();
}

void colorWaveRainbowInit() {
    activePattern = PATTERN_COLOR_WAVE_RAINBOW;
    patternAudioMode = AUDIO_MODE_OFF;
    colorHSV1 = CHSV(0, 255, 255);
    totalSteps = 256;
    curStep = 0;
    interval = WAVE_INTERVAL;
}

void colorWaveRainbowUpdate() {
    colorHSV1.sat = readPotScaled(ADJ_POT_PIN, RAINBOW_MIN_SAT, 255);
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
        uint16_t offset = map(led, 0, NUM_LEDS, 0, 2047);
        // colorHSV1.val = cubicwave8(curStep + offset);
        // uint8_t val = cubicwave8(curStep + offset);
        uint8_t val = map(cubicwave8(curStep + offset), 0, 255, BASELINE_BRIGHTNESS, 255);
        colorHSV1.val = pgm_read_byte(&gamma[val]);

        colorHSV1.hue = map(led, 0, NUM_LEDS, 0, 255) + curStep;
        leds[led] = colorHSV1;
    }

    incrementStep();
}

void colorPulseInit() {
    activePattern = PATTERN_COLOR_PULSE;
    patternAudioMode = AUDIO_MODE_OFF;
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

void randomPulseInit() {
    activePattern = PATTERN_RANDOM_PULSE;
    patternAudioMode = AUDIO_MODE_OFF;
    colorHSV1 = CHSV(0, 0, 255);
    totalSteps = 256;
    curStep = 0;
    interval = 20;
    for (uint8_t i = 0; i < NUM_RAND_PULSE_LEDS; i++) {
        randLeds[i] = random8(NUM_LEDS);
    }
}

void randomPulseUpdate() {
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
                    if (ledGap < 5) {
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
            leds[randLeds[led] - 1] %= pgm_read_byte(&gamma[128]);
        }
        if (randLeds[led] < NUM_LEDS - 1) {
            leds[randLeds[led] + 1] = colorHSV1;
            leds[randLeds[led] + 1] %= pgm_read_byte(&gamma[128]);
        }
        if (randLeds[led] > 1) {
            leds[randLeds[led] - 2] = colorHSV1;
            leds[randLeds[led] - 2] %= pgm_read_byte(&gamma[64]);
        }
        if (randLeds[led] < NUM_LEDS - 2) {
            leds[randLeds[led] + 2] = colorHSV1;
            leds[randLeds[led] + 2] %= pgm_read_byte(&gamma[64]);
        }
    }

    incrementStep();
}

void gradient1Init() {
    activePattern = PATTERN_GRADIENT_1;
    patternAudioMode = AUDIO_MODE_OFF;
}

void gradient1Update() {
    setGradientColors();

    fill_gradient(&(leds[0]), 0, CHSV(gradHue1, 255, 255), NUM_LEDS / 2, CHSV(gradHue2, 255, 255));
    fill_gradient(&(leds[0]), NUM_LEDS / 2, CHSV(gradHue2, 255, 255), NUM_LEDS - 1, CHSV(gradHue1, 255, 255));

}

void gradient2Init() {
    activePattern = PATTERN_GRADIENT_2;
    patternAudioMode = AUDIO_MODE_OFF;
}

void gradient2Update() {
    setGradientColors();

    fill_gradient(&(leds[0]), 0, CHSV(gradHue1, 255, 255), NUM_LEDS / 4, CHSV(gradHue2, 255, 255));
    fill_gradient(&(leds[0]), NUM_LEDS / 4, CHSV(gradHue2, 255, 255), NUM_LEDS / 2, CHSV(gradHue1, 255, 255));
    fill_gradient(&(leds[0]), NUM_LEDS / 2, CHSV(gradHue1, 255, 255), NUM_LEDS * 3 / 4, CHSV(gradHue2, 255, 255));
    fill_gradient(&(leds[0]), NUM_LEDS * 3 / 4, CHSV(gradHue2, 255, 255), NUM_LEDS - 1, CHSV(gradHue1, 255, 255));
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