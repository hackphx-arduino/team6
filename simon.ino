#include "Shieldbot.h"

// Free pins on the ShieldBot: 1, 2, 3, 11, 12, 13, A4, A5
Shieldbot shieldbot = Shieldbot();

#define TRUE 1
#define FALSE 0

#define PLAYER_1 1
#define PLAYER_2 2

#define WIN_SCORE 20

#define LED1_PIN 1
#define LED2_PIN 2
#define LED3_PIN 3
#define LED4_PIN 12
#define LED5_PIN 13

#define SPEAKER_PIN 11 // A4 or A5 

#define EASY 1
#define MEDIUM 2
#define HARD 3

#define EASY_STEPS 3
#define MEDIUM_STEPS 5
#define HARD_STEPS 7

#define EASY_LEVELS 4
#define MEDIUM_LEVELS 4
#define HARD_LEVELS 4

int EASY_PUZZLES[EASY_LEVELS][EASY_STEPS] = {
  { 4, 2, 3 },
  { 3, 4, 5 },
  { 4, 3, 5 },
  { 2, 4, 3 }
};

int MEDIUM_PUZZLES[MEDIUM_LEVELS][MEDIUM_STEPS] = {
  { 4, 2, 3, 2, 2 },
  { 3, 4, 5, 3, 2 },
  { 5, 3, 5, 4, 2 },
  { 2, 4, 3, 5, 2 }
};

int HARD_PUZZLES[HARD_LEVELS][HARD_STEPS] = {
  { 3, 2, 2, 3, 4, 5, 4 },
  { 3, 4, 5, 2, 5, 2, 2 },
  { 1, 3, 5, 3, 5, 3, 5 },
  { 2, 4, 3, 4, 5, 4, 5 }
};

int S1, S2, S3, S4, S5;

int currentPlayer = PLAYER_1;
int player1Score = 0;
int player2Score = 0;

int winningPlayer = 0;
int roundsPlayed = 0;
int currentDifficulty;
int currentSteps;
int commonTurnDelay = 1050;

int* activePuzzle;

int playing = TRUE;
int forwardSteps = 0;

void setup(){
  Serial.begin(9600);
  shieldbot.setMaxSpeed(50, 65);//255 is max
  
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(LED5_PIN, OUTPUT);

  lightOn(LED1_PIN); lightOn(LED2_PIN); lightOn(LED3_PIN); lightOn(LED4_PIN); lightOn(LED5_PIN);
  delay(5000);
  lightOff(LED1_PIN); lightOff(LED2_PIN); lightOff(LED3_PIN); lightOff(LED4_PIN); lightOff(LED5_PIN);
}

void loop(){
  if(playing) {   
    if( roundsPlayed <= EASY_LEVELS ) {
      // Easy
      currentDifficulty = EASY;
      currentSteps = EASY_STEPS;
    }
    else if( roundsPlayed > (EASY_LEVELS + MEDIUM_LEVELS) ) {
      // Hard
      currentDifficulty = HARD;
      currentSteps = HARD_STEPS;
    }
    else {
      // Medium
      currentDifficulty = MEDIUM;
      currentSteps = MEDIUM_STEPS;
    }
    
    playPuzzle();
    waitForResponse();
    int passed = readResponse();
    logScore(passed);
    
    if(checkWin()) {
      celebrate();
      exit();
    }
    
    forwardSteps = 0;
    roundsPlayed++;
    currentPlayer = currentPlayer == PLAYER_1 
      ? PLAYER_2
      : PLAYER_1;
  }
}

void playPuzzle() {  
  if(currentDifficulty == EASY) {
    activePuzzle = EASY_PUZZLES[roundsPlayed-1];
  }
  else if(currentDifficulty == MEDIUM) {
    activePuzzle = MEDIUM_PUZZLES[roundsPlayed-1 - EASY_STEPS];
  }
  else { // Hard
    activePuzzle = HARD_PUZZLES[roundsPlayed-1 - EASY_STEPS - MEDIUM_STEPS];
  }
  
  for(int i =0; i <= currentSteps; i++) {
    int pin;
    
    if( activePuzzle[i] == 1) { pin = LED1_PIN; }
    else if( activePuzzle[i] == 2) { pin = LED2_PIN; }
    else if( activePuzzle[i] == 3) { pin = LED3_PIN; }
    else if( activePuzzle[i] == 4) { pin = LED4_PIN; }
    else /*if( activePuzzle[i] == 5)*/ { pin = LED5_PIN; }
    
    playSound(activePuzzle[i], pin);
    delay(750);
  }
  
  delay(1000);
}

void waitForResponse() { 
  delay(10000);
}

int readResponse() {  
  delay(200);
  
  return TRUE;
  
  // Perform checks
  if(currentDifficulty == EASY) {
    return checkEasy();
  }
  else if(currentDifficulty == MEDIUM) {
    return checkMedium();
  }
  else { // Hard
    return checkHard();
  }
}

void logScore(int response) {  
  if(response) {
    int score = 5 * currentDifficulty;
    
    if(currentPlayer == PLAYER_1) {
      player1Score += score;
      movePlayer1();
    }
    else if(currentPlayer == PLAYER_2) {
      player2Score += score;
      movePlayer2();
    }
  }
}

void movePlayer1() {
    shieldbot.drive(-128,127);
    delay(commonTurnDelay);
    shieldbot.stop();
    
    delay(100);
    
    shieldbot.forward();
    delay(100 * currentDifficulty);
    shieldbot.stop();  

    delay(100);
    
//    shieldbot.drive(127,-128);
//    delay(commonTurnDelay);
//    shieldbot.stop();
}

void movePlayer2() {
    shieldbot.drive(127,-128);
    delay(commonTurnDelay);
    shieldbot.stop();
    
    delay(100);
    
    shieldbot.forward();
    delay(100 * currentDifficulty);
    shieldbot.stop();
    
    delay(100);
    
//    shieldbot.drive(-128,127);
//    delay(commonTurnDelay);
//    shieldbot.stop();
}

int checkWin() {
  if(currentPlayer == PLAYER_1 && player1Score >= WIN_SCORE) {
    // Player 1 wins
    winningPlayer = PLAYER_1;
    return TRUE;
  }
  else if(currentPlayer == PLAYER_2 && player2Score >= WIN_SCORE) {
    // Player 2 wins
    winningPlayer = PLAYER_2;
    return TRUE;
  }
  
  return FALSE;
}

void celebrate() {
  // Lights, sounds, fun!
  shieldbot.drive(-128,127);   //spin
  
  playMelody();
  
  int delayTotal = 0;
  while(delayTotal < 5000) {
    if(delayTotal % 2 == 0) {
      lightOn(LED1_PIN); lightOff(LED2_PIN); lightOn(LED3_PIN); lightOff(LED4_PIN); lightOn(LED5_PIN); 
    }
    else {
      lightOff(LED1_PIN); lightOn(LED2_PIN); lightOff(LED3_PIN); lightOn(LED4_PIN); lightOff(LED5_PIN); 
    }
    
    delay(500);
    delayTotal += 500;
  }
  
  shieldbot.stop();
 
  lightOff(LED1_PIN); lightOff(LED2_PIN); lightOff(LED3_PIN); lightOff(LED4_PIN); lightOff(LED5_PIN);
}

void exit() {
  playing = FALSE;
}

void readAllSensors() {
  //Read all the sensors 
  S1 = shieldbot.readS1();
  S2 = shieldbot.readS2();
  S3 = shieldbot.readS3();
  S4 = shieldbot.readS4();
  S5 = shieldbot.readS5();
}

int findNote() {
 
  int count = 0;
  int tries = 0;
  int note  = 0;
 
  do {
    tries++;
    count = 0;
 
    stepForward();
    if (S1 == LOW) { count++; note = 1; }
    if (S2 == LOW) { count++; note = 2; }
    if (S3 == LOW) { count++; note = 3; }
    if (S4 == LOW) { count++; note = 4; }
    if (S5 == LOW) { count++; note = 5; }
  }
  while (count == 0 && tries < 5);
 
  if (count == 1) {
    return note;
  } else {  
    return 0;
  }
 
}
 
int check(int level) {
  int correctCount = 0;
   
  for(int i = 0; i <= level; i++) {
    if(findNote() == activePuzzle[i]) {
      correctCount++;
    }
    else {
      // Give up
      break;
    }
  }
  
  while(forwardSteps) {
    // Backup the correct # of steps
    stepBackwards();
  }
    
  if(correctCount == level) {
    return true;
  }
  else {
    return false;
  }
}
 
int checkEasy() {
  return check(EASY_STEPS);
}
 
int checkMedium() {
  return check(MEDIUM_STEPS);
}
 
int checkHard() {
  return check(HARD_STEPS);
}

void stepForward() {
  shieldbot.forward();
  delay(250);
  shieldbot.stop();
  
  forwardSteps++;
}

void stepBackwards() {
  shieldbot.backward();
  delay(250);
  shieldbot.stop();
  
  forwardSteps--;
}

void lightOn(int pin) {
  digitalWrite(pin, LOW);  
}

void lightOff(int pin) {
  digitalWrite(pin, HIGH);  
}



/* Play Melody
 * -----------
 *
 * Program to play melodies stored in an array, it requires to know
 * about timing issues and about how to play tones.
 *
 * The calculation of the tones is made following the mathematical
 * operation:
 *
 *       timeHigh = 1/(2 * toneFrequency) = period / 2
 *
 * where the different tones are described as in the table:
 *
 * note   frequency 	period 	PW (timeHigh)	
 * c 	        261 Hz 	        3830 	1915 	
 * d 	        294 Hz 	        3400 	1700 	
 * e 	        329 Hz 	        3038 	1519 	
 * f 	        349 Hz 	        2864 	1432 	
 * g 	        392 Hz 	        2550 	1275 	
 * a 	        440 Hz 	        2272 	1136 	
 * b 	        493 Hz 	        2028	1014	
 * C	        523 Hz	        1912 	956
 *
 * (cleft) 2005 D. Cuartielles for K3
 */
          
byte names[] = {'1', '2', '3', '4', '5', };  
int tones[] = {1915, 1700, 1519, 1432, 1275, 0 };
byte melody[] = "431p431p441p451p451p441p431p421p411p411p421p531p231p431p43";;
// count length: 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
//                                10                  20                  30
int count = 0;
int count2 = 0;
int count3 = 0;
int MAX_COUNT = 24;
 
void playMelody() {
  
  analogWrite(SPEAKER_PIN, 0);     
  for (count = 0; count < MAX_COUNT; count++) {
    for (count3 = 0; count3 <= (melody[count*2] - 48) * 30; count3++) {
      for (count2=0;count2<8;count2++) {
        if (names[count2] == melody[count*2 + 1]) {       
          analogWrite(SPEAKER_PIN,500);
          delayMicroseconds(tones[count2]);
          analogWrite(SPEAKER_PIN, 0);
          delayMicroseconds(tones[count2]);
        } 
        if (melody[count*2 + 1] == 'p') {
          // make a pause of a certain size
          analogWrite(SPEAKER_PIN, 0);
          delayMicroseconds(500);
        }
      }
    }
  }
}

void playSound( int  note, int pin ) {
  
  lightOn(pin);
  
  for(int i = 0; i < 20; i++) {
    analogWrite(SPEAKER_PIN,500);
    delayMicroseconds(tones[note]);
    analogWrite(SPEAKER_PIN, 0);
    delayMicroseconds(tones[note]);
  }
  
  lightOff(pin);
}





