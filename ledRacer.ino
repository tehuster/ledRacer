#include "FastLED.h"

//////////////////////////////////////// DEFINES

#define BUTTONPINP1LEFT 3
#define BUTTONPINP1RIGHT 4
#define BUTTONPINP2LEFT 5
#define BUTTONPINP2RIGHT 6
#define BUTTONPINP3LEFT 7
#define BUTTONPINP3RIGHT 8
#define BUTTONPINP4LEFT 9
#define BUTTONPINP4RIGHT 10

#define LED1 11
#define LED2 12
#define LED3 13
#define LED4 14

#define LEFT 0
#define RIGHT 1
#define LASTLEFT 2 
#define LASTRIGHT 3

#define VELOCITY 0
#define FLOATPOS 1
#define SPEED 2
#define LANE 0
#define POS 1
#define OLDPOS 2

///////////////////////////////////////// redOrbInfo

int redOrbAmount = 10;
float redOrbMov[64][2]; // 0 VELOCITY  1 FLOATPOS
int redOrbPos[64][2];   // 0 LANE      1 POS  
int direction = 1;

const int NUM_LEDS = 300;

////////////////////////////////////////  playerInfo

const int playerButtonPins[4][2] = {
    {BUTTONPINP1LEFT, BUTTONPINP1RIGHT},
    {BUTTONPINP2LEFT, BUTTONPINP2RIGHT},
    {BUTTONPINP3LEFT, BUTTONPINP3RIGHT},
    {BUTTONPINP4LEFT, BUTTONPINP4RIGHT}
};

int playerAmount = 4;
boolean playerButtonState[4][4]; // 0 LEFT      1 RIGHT    2 LASTLEFT   3 LASTRIGHT
float playerMov[4][3];           // 0 VELOCITY  1 FLOATPOS 2 SPEED
int playerPos[4][2];             // 0 LANE      1 POS      2 OLDPOS
//float friction = 0.99;           // Not needed when fixed speed..

/////////////////////////////////////////  updateInfo

unsigned long previousMillis = 0;
const long interval = 1;

/////////////////////////////////////////  ledInfo
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];
CRGB leds3[NUM_LEDS];
CRGB leds4[NUM_LEDS];

/////////////////////////////////////////////////////////////////// SETUP - fix: set default speed

void setup()
{
    delay(3000);

    for (int i = 0; i < playerAmount; i++)
    {
        pinMode(playerButtonPins[i][0], INPUT);
        pinMode(playerButtonPins[i][1], INPUT);
    }

    FastLED.addLeds<NEOPIXEL, LED1>(leds1, NUM_LEDS);
    FastLED.addLeds<NEOPIXEL, LED2>(leds2, NUM_LEDS);
    FastLED.addLeds<NEOPIXEL, LED3>(leds3, NUM_LEDS);
    FastLED.addLeds<NEOPIXEL, LED4>(leds4, NUM_LEDS);

    createOrbs();
}

/////////////////////////////////////////////////////////////////// LOOP

void loop()
{
    FastLED.clear();
    readInput();
    hitDetection();
    hitDetectionOrbs();
    movement();
    updateLeds();
    FastLED.show();
}

/////////////////////////////////////////////////////////////////// INPUT

void readInput()
{
    laneChangeInput();
}

void laneChangeInput()
{
    for (int i = 0; i < playerAmount; i++) //0 LEFT  1 RIGHT 2 LASTLEFT 3 LASTRIGHT
    {
        playerButtonState[i][LEFT] = digitalRead(playerButtonPins[i][LEFT]);
        playerButtonState[i][RIGHT] = digitalRead(playerButtonPins[i][RIGHT]);

        if (playerButtonState[i][LEFT] != playerButtonState[i][LASTLEFT])
        {
            if (playerButtonState[i][LEFT] == HIGH)
            {
                changePlayerLane(i, LEFT);
            }
        }
        playerButtonState[i][LASTLEFT] = playerButtonState[i][LEFT];
        if (playerButtonState[i][RIGHT] != playerButtonState[i][LASTRIGHT])
        {
            if (playerButtonState[i][RIGHT] == HIGH && playerPos[i][LANE] != 3)
            {
                changePlayerLane(i, RIGHT);
            }
        }
        playerButtonState[i][LASTRIGHT] = playerButtonState[i][RIGHT];
    }
}

/////////////////////////////////////////////////////////////////// CREATE

void createOrbs()
{
    for (int i = 0; i < redOrbAmount; i++)
    {
        int lane = random(0, 4);
        float posFloat = random(0, 200.0);
        float velocity = random(10, 40);
        velocity = velocity / 100;
        redOrbPos[i][LANE] = lane;
        redOrbMov[i][VELOCITY] = velocity;
        redOrbMov[i][FLOATPOS] = posFloat;
    }
}

/////////////////////////////////////////////////////////////////// MOVEMENT

void movement()
{
    moveOrb();
    movePlayer();
}

void moveOrb()
{
    for (int i = 0; i < redOrbAmount; i++)
    {
        redOrbMov[i][FLOATPOS] += redOrbMov[i][VELOCITY];
        if (redOrbMov[i][FLOATPOS] > NUM_LEDS)
        {
            redOrbMov[i][FLOATPOS] = 0.0;
        };
        redOrbPos[i][POS] = (int)(redOrbMov[i][FLOATPOS] + 0.5);
    }
}

void movePlayer()
{
    //speed 0-100 / 10000 = 0,0000-0,0100
    for (int i = 0; i < playerAmount; i++)
    {
        playerPos[i][VELOCITY] += playerPos[i][SPEED];
        //playerPos[i][0] = playerPos[i][0] * friction; // Not needed when fixed speed..]
        playerPos[i][FLOATPOS] += playerPos[i][VELOCITY];
        if (playerPos[i][FLOATPOS] > NUM_LEDS)
        {
            playerPos[i][FLOATPOS] += (playerPos[i][FLOATPOS] - NUM_LEDS);
        }
        else if (playerPos[i][FLOATPOS] < 0.0)
        {
            playerPos[i][FLOATPOS] += NUM_LEDS;
        }
        playerPos[i][POS] = (int)(playerPos[i][FLOATPOS] + 0.5);
        
        if(overtakeDetection(playerPos[i][OLDPOS], playerPos[i][POS])){
            playerMov[i][SPEED] += 0.0010;
        }

        playerPos[i][OLDPOS] = playerPos[i][POS];
    }
};

void changePlayerLane(int player, int direction)
{
    if (direction == RIGHT && playerPos[player][LANE] != 3)
    {
        playerPos[player][LANE]++; // 0 LANE      1 POS
    }
    else if (direction == LEFT && playerPos[player][LANE] != 0)
    {
        playerPos[player][LANE]--;
    }
};

void changeOrbLane(int orb)
{
    if (redOrbPos[orb][LANE] == 0)
    {
        redOrbPos[orb][LANE]++;
    }
    else if (redOrbPos[orb][LANE] == 3)
    {
        redOrbPos[orb][LANE]--;
    }
    else
    {
        direction = direction * -1;
        redOrbPos[orb][LANE] += direction;
    }
};

/////////////////////////////////////////////////////////////////// HIT DETECTION - fix: set speed to default when hit
void hitDetectionOrbs()
{
    for (int i = 0; i < redOrbAmount; i++)
    {
        for (int j = 0; j < redOrbAmount; j++)
        {
            if (i != j)
            {
                if (redOrbPos[i][LANE] == redOrbPos[j][LANE] && redOrbPos[j][POS] == (redOrbPos[i][POS] - 1))
                {
                    if (redOrbMov[i][VELOCITY] > redOrbMov[j][VELOCITY]) // klopt dit nog wel helemaal?
                    {
                        changeOrbLane(i);
                        //     redOrbPos[i][POS]++;  // But Why?
                    }
                    // else                          // But Why?
                    // {
                    //     changeOrbLane(j);         
                    //     redOrbPos[j][LANE]++;
                    // }
                }
            }
        }
    }
}

void hitDetection()
{ 
    for (int i = 0; i < redOrbAmount; i++)
    {
        for (int p = 0; p < playerAmount; p++)
            if (playerPos[p][LANE] == redOrbPos[i][LANE] && playerPos[p][POS] == redOrbPos[i][POS])
            {
                playerMov[p][VELOCITY] -= 0.1; //change depending on fixedSpeed
            }
    }
};

/////////////////////////////////////////////////////////////////// OVERTAKING - not easy to expand method

boolean overtakeDetection(int oldPos, int newPos){
    for(int i = 0; i < redOrbAmount; i++){
        if(oldPos < redOrbPos[i][POS] && newPos > redOrbPos[i][POS]){
            return true;
        }else{
            return false;
        }
    }
}

/////////////////////////////////////////////////////////////////// UPDATE LEDS

void updateLeds()
{
    unsigned long currentMillis = micros();
    if (currentMillis - previousMillis >= interval)
    {
        updatePlayer();
        updateOrb();
        previousMillis = currentMillis;
    }
}

void updatePlayer()
{
    for(int i = 0; i < playerAmount; i++){
        switch (playerPos[i][LANE]) {
            case 0:
                leds1[playerPos[i][POS]] = CRGB::Cyan;
                break;
            case 1:
                leds2[playerPos[i][POS]] = CRGB::Cyan;
                break;   
            case 2:
                leds3[playerPos[i][POS]] = CRGB::Cyan;
                break;
            case 3:
                leds4[playerPos[i][POS]] = CRGB::Cyan;
                break;         
        }
    }
}

void updateOrb()
{
   for(int i = 0; i < redOrbAmount; i++){
        switch (redOrbPos[i][LANE]) {
            case 0:
                leds1[redOrbPos[i][POS]] = CRGB::Red;
                break;
            case 1:
                leds2[redOrbPos[i][POS]] = CRGB::Red;
                break;   
            case 2:
                leds3[redOrbPos[i][POS]] = CRGB::Red;
                break;
            case 3:
                leds4[redOrbPos[i][POS]] = CRGB::Red;
                break;         
        }
    }
}
