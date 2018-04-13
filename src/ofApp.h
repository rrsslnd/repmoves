#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxSimpleSerial.h"
#include <vector>

using namespace std;

// listen on port 12345
#define rPORT 12345
#define sPORT 12346
#define HOST "localhost"
#define NUM_MSG_STRINGS 20

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    //OSC
    ofTrueTypeFont font;
    ofxOscReceiver receiver;
    ofxOscSender sender;
    void sendSongTitleOsc();
    
    //DRAW HAND CIRCLE
    float drawCircleX;
    float drawCircleY;
    
    //POSITION DATA
    float lhnd[3];
    float prev_lhnd[3];
    float rhnd[3];
    float prev_rhnd[3];
    float head[3];
    float prev_head[3];
    float lknee[3];
    float prev_lknee[3];
    float rknee[3];
    float prev_rknee[3];
    float lsho[3];
    float prev_lsho[3];
    float rsho[3];
    float prev_rsho[3];
    
    //FILTER
    float lowPassFilter1(float sensorVal);
    float lowPassFilter2(float sensorVal);
    float lowPassFilter3(float sensorVal);
    float lowPassFilter4(float sensorVal);
    float lowPassFilter5(float sensorVal);
    float lowPassFilter6(float sensorVal);
    float filterValue1[4];
    float filterValue2[4];
    float filterValue3[4];
    float filterValue4[4];
    float filterValue5[4];
    float filterValue6[4];
    float noiseThreshold;
    
    //SAMPLE TIMER
    bool sampleTimer();
    float prevTime; // store when we start time timer
    float samplePeriod; // when do want to stop the timer
    bool  bTimerReached; // used as a trigger when we hit the timer
    
    //BEAT TIMER
    void storeBeatTime();
    unsigned int prevBeatTime;
    float movementAvg;
    
    //DATA STORAGE
    std::vector <float> movementData;
    std::vector <float> beatPeriod;
    std::vector <float> incrScoreSum;
    std::vector <string> movementPatternChanges;
    std::vector <char> axisChanges;
    int index;
    void saveData();
    
    //KEYBOARD INPUT
    int keyboard;
    
    //MOVEMENT DETECTION
    int notMovingCounter = 0;
    
    //CONDUCTOR
    bool conductor();
    string lastDir = "left";
    bool changeDir = false;
    //POWER WALK
    bool powerWalk();
    //STEVIE WONDER
    bool stevieWonder();
    //KNEE BEND
    bool kneeBend();
    //KNEE LIFT
    bool kneeLift();
    //CONDUCTOR MIRROR
    bool conductorMirror();
    //SHOULDER TWIST
    bool shoulderTwist();
    
    //MODESELECTA
    string prevMode;
    char axis;
    
    //FIRST RUN TEST
    bool bFirstRun = true;
    ofBuffer buffer;
    
    //PLAYBACK CONTROL
    float defaultBeatPeriod;
    float speed;
    float calcTempo(float movementPeriod);
    bool bPause;
    int bpm;
    bool bTheEnd;
    int startCountdown;
    
    //READ SERIAL
    ofxSimpleSerial	serial;
    string message;
    void onNewMessage(string & message);
    string nfcId;
    string delSpaces(string &str);
    
    //READ SONG FILE
    vector <string> songList;
    string selectedSong;
    string prevSong;
    void nextSong();
    void previousSong();
    
    //MOVIE
    ofVideoPlayer movie;
    vector <string> movieList;
    void changeMovie();
    string selectedMovie;
    string previousMovie;
    void nextMovie();
    void prevMovie();
    
    //MODE
    string mode;
    bool bModeChanged;
    void checkModeChange(string currentMode);
    
    //USER SELECT MOVEMENT PATTERN
    void checkHandPosition();
    void selectionTimer();
    float selectionStartTime; // store when we start time timer
    float selectionDuration; // when do want to stop the timer
    bool  bSelectionTimerReached; // used as a trigger when we hit the timer
    bool bNewSession;
    bool bHandIsInside;
    int handPosition;
    int selectorHandX;
    int selectorHandY;
    
    //INTERFACE
    int interfacePosX;
    int interfacePosY;
    void drawInterface();
    int radius;
    //        const int rightLeftDistance;
    //        const int farRightLeftDistance;
    //        const int farTopDistance;
    //        const int farBottomDistance;
    //        const int closeTopDistance;
    //        const int closeBottomDistance;
    //        const int centerTopDistance;
    ofImage rightHand;
    ofImage leftHand;
    //RUNTIME INTERFACE
    void calcMetrics();
    float prevSpeed;
    bool bShowMoveIndicator;
//    float movDelta;
    float songProgress;
    int rectCornerRad;
    
    
    //SAVE SESSION DATA TO FILE
    ofstream saveFile;
    string movementPattern;
    int sessionEndIndex;
    
    //SCORE
    void updateScore(float lastMove);
    float score;
    float incrScore;
    int scoreColor;
    
    //TEXT
    ofTrueTypeFont	verdana30;
    
    //MASTER CONTROL
    bool bDisableMovie = false;
    bool bDisableMusic = false;
    bool bDisableVariableTempo = false;
    int prevMuteSec;
    float muteCounter;
};
