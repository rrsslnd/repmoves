#include "ofApp.h"
#include <vector>
#include "ofUtils.h"
#include <iostream>
#include <string>
#include <sstream>

//--------------------------------------------------------------
void ofApp::setup(){
    
    //OSC
    //    listen on the given port
    //    cout << "listening for osc messages on port " << rPORT << "\n";
    receiver.setup(rPORT);
    sender.setup(HOST, sPORT);
    
    //SAMPLE TIMER
    prevTime = ofGetElapsedTimeMicros();
    samplePeriod = 200000.0;
    
    //BEAT TIMER
    movementAvg = 0.0f;
    
    //READ SERIAL
    ofSetVerticalSync(true);
//    serial.setup("/dev/tty.usbserial-AI02SPJL", 9600); //INSTALLATION
//    serial.setup("/dev/cu.wchusbserial1420", 9600); //DEV
//    serial.startContinuousRead();
//    ofAddListener(serial.NEW_MESSAGE,this,&ofApp::onNewMessage);
//    message = "";
    
    //READ SONG FILE
    // this is our buffer to store the text data
    ofBuffer buffer = ofBufferFromFile("songList.txt");
    if(buffer.size()) {
        for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it) {
            string line = *it;
            // copy the line to draw later
            // make sure its not a empty line
//            if(line.empty() == false) {
                songList.push_back(line);
//            }
            // print out the line
//                        cout << line << endl;
        }
    }
    
    //READ MOVIE FILE
    // this is our buffer to store the text data
    ofBuffer movieBuffer = ofBufferFromFile("movieList.txt");
    if(movieBuffer.size()) {
        for (ofBuffer::Line it = movieBuffer.getLines().begin(), end = movieBuffer.getLines().end(); it != end; ++it) {
            string line = *it;
            // copy the line to draw later
            // make sure its not a empty line
            //            if(line.empty() == false) {
            movieList.push_back(line);
            //            }
            // print out the line
                                    cout << line << endl;
        }
    }
    
    //MOVIE
    ofSetFrameRate(50);
    // Uncomment this to show movies with alpha channels
    // movie.setPixelFormat(OF_PIXELS_RGBA);
    selectedMovie = movieList[5];
    movie.load("movies/" + selectedMovie);
    movie.setLoopState(OF_LOOP_NORMAL);
    movie.play();

    //PLAYBACK CONTROL
    bPause = false;
    bTheEnd = false;
    
    //STARTUP SETTINGS
    mode = "setup";
    bModeChanged = false;
    radius = 40;
    selectionDuration = 1500.0f;
    bSelectionTimerReached = true;
    interfacePosX = ofGetWidth()/2;
    interfacePosY = ofGetHeight()/2;
    index = 0;
    sessionEndIndex = 0;
    selectedSong = "Alice Russell - High Up On The Hook";
    defaultBeatPeriod = 785;

    
    //SCORE
    score = 0.0f;
    
    //INTERFACE
    rightHand.load("images/right_hand.png");
    leftHand.load("images/left_hand.png");
    rectCornerRad = 4;
    ofToggleFullscreen();
    
    //TEXT
    ofTrueTypeFont::setGlobalDpi(72);
    verdana30.load("Optima.ttc", 30, true, true);
    verdana30.setLineHeight(34.0f);
    verdana30.setLetterSpacing(1.035);
}

//--------------------------------------------------------------
void ofApp::update(){
    
//    for (int i = 0; i < songList.size(); i++) {
//        ofSendMessage("Index: " + ofToString(i));
//        ofSendMessage("Line: " + ofToString(songList[i]));
//    }
    
    if (bFirstRun) {
        checkModeChange("setup");
        cout << "FirstRun!!" << "\n";
        bFirstRun = false;
    }
    
    movie.update();
    // check for waiting messages
    while(receiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        //UPDATE SONG PROGRESS
        if(m.getAddress() == "/song_progress") {
            songProgress = m.getArgAsFloat(0);
        }
        
        //SAVE SESSION DATA AFTER EACH SONG HAS FINISHED
        if(m.getAddress() == "/the_end"){
            saveData();
        }
        
        //ENTER SETUP MODE
        if (mode == "setup") {
            bPause = true;
            checkModeChange("setup");
            if(m.getAddress() == "/lhnd1x"){
                //update incoming value
                lhnd[0] = lowPassFilter1(m.getArgAsFloat(0));
            } else if (m.getAddress() == "/rhnd1x") {
                //update incoming value
                rhnd[0] = lowPassFilter2(m.getArgAsFloat(0));
            } else if (m.getAddress() == "/lhnd1y") {
                //update incoming value
                lhnd[1] = lowPassFilter3(m.getArgAsFloat(0));
            } else if (m.getAddress() == "/rhnd1y") {
                //update incoming value
                rhnd[1] = lowPassFilter4(m.getArgAsFloat(0));
            } else if (m.getAddress() == "/head1x") {
                head[0] = lowPassFilter5(m.getArgAsFloat(0));
            } else if (m.getAddress() == "/head1y") {
                head[1] = lowPassFilter6(m.getArgAsFloat(0));
            }
            checkHandPosition();
        }
        
        //CONDUCTOR
        if (mode == "conductor") {
            movementPattern = mode;
            checkModeChange(mode);
            if (axis == 'x') {
                if(m.getAddress() == "/lhnd1x"){
                    //update incoming value
                    lhnd[0] = lowPassFilter1(m.getArgAsFloat(0));
                } else if (m.getAddress() == "/rhnd1x") {
                    //update incoming value
                    rhnd[0] = lowPassFilter2(m.getArgAsFloat(0));
                }
            } else if (axis == 'y'){
                if (m.getAddress() == "/lhnd1y") {
                    //update incoming value
                    lhnd[1] = lowPassFilter1(m.getArgAsFloat(0));
                } else if (m.getAddress() == "/rhnd1y") {
                    //update incoming value
                    rhnd[1] = lowPassFilter2(m.getArgAsFloat(0));
                }
            } else if (axis == 'z') {
                if (m.getAddress() == "/lhnd1z") {
                    //update incoming value
                    lhnd[2] = lowPassFilter1(m.getArgAsFloat(0));
                } else if (m.getAddress() == "/rhnd1z") {
                    //update incoming value
                    rhnd[2] = lowPassFilter2(m.getArgAsFloat(0));
                }
            }
                if (sampleTimer()) {
                    if (conductor()) {
                        storeBeatTime();
    //                    calcMetrics(); //calculates performance metrics
                        
                    }
                }
        }
        //POWER WALK
        else if (mode == "power walk") {
            movementPattern = mode;
            checkModeChange(mode);
            if (axis == 'x') {
                if(m.getAddress() == "/lhnd1x"){
                    // store y value
                    lhnd[0] = lowPassFilter1(m.getArgAsFloat(0));
                } else if(m.getAddress() == "/rhnd1x"){
                    // store y value
                    rhnd[0] = lowPassFilter2(m.getArgAsFloat(0));
                }
            } else if (axis == 'y') {
                if(m.getAddress() == "/lhnd1y"){
                    // store y value
                    lhnd[1] = lowPassFilter1(m.getArgAsFloat(0));
                } else if(m.getAddress() == "/rhnd1y"){
                    // store y value
                    rhnd[1] = lowPassFilter2(m.getArgAsFloat(0));
                }
            } else if (axis == 'z') {
                if(m.getAddress() == "/lhnd1z"){
                    //update incoming value
                    lhnd[2] = lowPassFilter1(m.getArgAsFloat(0));
                } else if(m.getAddress() == "/rhnd1z"){
                    //update incoming value
                    rhnd[2] = lowPassFilter2(m.getArgAsFloat(0));
                }
            }
            
            if (sampleTimer()) {
                if (powerWalk()) {
                    //get beat / movement period
                    storeBeatTime(); // detect beat period
//                    calcMetrics(); //calculates performance metrics
                }
            }
        }
        //STEVIE WONDER
        else if (mode == "stevie wonder") {
            movementPattern = mode;
            checkModeChange(mode);
            if (axis == 'x') {
                if(m.getAddress() == "/head1x"){
                    // store x value
                    head[0] = lowPassFilter1(m.getArgAsFloat(0));
                }
            } else if (axis == 'y') {
                if(m.getAddress() == "/head1y"){
                    // store x value
                    head[1] = lowPassFilter1(m.getArgAsFloat(0));
                }
            } else if (axis == 'z') {
                if(m.getAddress() == "/head1z"){
                    // store x value
                    head[2] = lowPassFilter1(m.getArgAsFloat(0));
                }
            }
            if (sampleTimer()) {
                if (stevieWonder()) {
                    //get beat / movement period
                    storeBeatTime(); // detect beat period
//                    calcMetrics(); //calculates performance metrics
                }
            }
        }
        //KNEE BEND
        else if (mode == "knee bend") {
            movementPattern = mode;
            checkModeChange(mode);
            if(m.getAddress() == "/head1y"){
                // store y value
                head[1] = lowPassFilter1(m.getArgAsFloat(0));
            }
            if (sampleTimer()) {
                if (kneeBend()) {
                    //get beat / movement period
                    storeBeatTime(); // detect beat period
//                    calcMetrics(); //calculates performance metrics
                }
            }
        }
        
        //KNEE LIFT
        else if (mode == "knee lift") {
            movementPattern = mode;
            checkModeChange(mode);
            if(m.getAddress() == "/lkne1z"){
                //update incoming value
                lknee[2] = lowPassFilter1(m.getArgAsFloat(0));
            }
            else if(m.getAddress() == "/rkne1z"){
                //update incoming value
                rknee[2] = lowPassFilter2(m.getArgAsFloat(0));
            }
        
            if (sampleTimer()) {
                if (kneeLift()) {
                    //get beat / movement period
                    storeBeatTime(); // detect beat period
                    //                    calcMetrics(); //calculates performance metrics
                }
            }
        }
        //CONDUCTOR MIRROR
        if (mode == "conductor mirror") {
            movementPattern = mode;
            checkModeChange(mode);
            if(m.getAddress() == "/lhnd1x"){
                //update incoming value
                lhnd[0] = lowPassFilter1(m.getArgAsFloat(0));
            } else if (m.getAddress() == "/rhnd1x") {
                //update incoming value
                rhnd[0] = lowPassFilter2(m.getArgAsFloat(0));
            }
            if (sampleTimer()) {
                if (conductorMirror()) {
                    storeBeatTime();
                    //                    calcMetrics(); //calculates performance metrics
                    
                }
            }
        }
        
        //SHOULDER TWIST
        else if (mode == "shoulder twist") {
            movementPattern = mode;
            checkModeChange(mode);
            
            if(m.getAddress() == "/lsho1z"){
                //update incoming value
                lsho[2] = lowPassFilter1(m.getArgAsFloat(0));
            } else if(m.getAddress() == "/rsho1z"){
                //update incoming value
                rsho[2] = lowPassFilter2(m.getArgAsFloat(0));
            }
            
            if (sampleTimer()) {
                if (shoulderTwist()) {
                    //get beat / movement period
                    storeBeatTime(); // detect beat period
                    //                    calcMetrics(); //calculates performance metrics
                }
            }
        }
        
        //PLAY MUSIC IN DEFAULT TEMPO
        else if (mode == "play music") {
            speed = 1.0f;
        }
    }
    float tmpAvg = 0.0f; // used to check if average has changed
    float sum = 0.0f; //used to calculate average
    for (int i = movementData.size() - 4; i < movementData.size(); i++) { //sums the values of the last 4 movements -> USER ADJUSTABLE!
        sum += movementData[i];
    }
    tmpAvg = sum*0.25f;
    if (tmpAvg != movementAvg && !bPause && mode != "setup" && (movementData.size()-sessionEndIndex >= 4)){ //if average has changed AND the user is moving
        movementAvg = tmpAvg; //update average value
        speed = defaultBeatPeriod / movementAvg;
        bpm = round(60000.0f/movementAvg);
        ofSendMessage("Avg: " + ofToString(movementAvg));
    } else if (bPause) {
        speed = 0.0f;
    }
    if (!bDisableMusic) {
        if (mode == "play music") {
            speed = 1.0f;
        }
        ofxOscMessage m; // create an OSC message
        m.setAddress("/speed"); // create an OSC address
        m.addFloatArg(speed); //store average value in message
        sender.sendMessage(m, false); // send value via OSC
    }

    float movieSpeed;
    if (bDisableMovie) {
        movieSpeed = 0.0f;
    } else {
        movieSpeed = speed;
    }
    movie.setSpeed(movieSpeed);
    //    ofSendMessage("Speed: " + ofToString(speed));
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(255);
    
    //    string buf = "listening for osc messages on port : " + ofToString(rPORT);
    //    ofDrawBitmapStringHighlight(buf, 10, 20);

    movie.draw(0,0);
    
//    bPause = false;
    if (notMovingCounter > 5) {
        bPause = true;
        //        ofSendMessage("Pause");
    } else {
        bPause = false;
    }

    if (mode == "setup"){
        if (lhnd[1] > rhnd[1]) {
            drawCircleX = ofMap(rhnd[0], 0.0f, 1.0f, 1000, 0);
            drawCircleY = ofMap(rhnd[1], 1.0f, 0.0f, 680, 0);
        } else {
            drawCircleX = ofMap(lhnd[0], 0.0f, 1.0f, 1000, 0);
            drawCircleY = ofMap(lhnd[1], 1.0f, 0.0f, 680, 0);
        }
//        drawInterface();
        if (lhnd[1] > rhnd[1]) {
            rightHand.draw(drawCircleX - 10, drawCircleY - 10);
        } else {
            leftHand.draw(drawCircleX - 10, drawCircleY - 10);
        }
    }
    if (mode == "conductor") {
        float thirdHand = (lhnd[0] + rhnd[0]) /2.0; // average of both hands as virtual "third" hand
        drawCircleX = ofMap(thirdHand, 1, 0, 800, 1100);
        
    } else if (mode == "power walk") {
        if (lhnd[2] > rhnd[2]) {
            drawCircleX = ofMap(rhnd[2], 0, 4, 1000, 1050);
        } else {
            drawCircleX = ofMap(lhnd[2], 0, 4, 950, 1000);
        }
        
    } else if (mode == "stevie wonder") {
        drawCircleX = ofMap(head[0], 1, 0, 800, 1100);
    
    } else if (mode == "knee bend") {
        drawCircleX = ofMap(head[1], 0.3, 0.7, 900, 1100);
    } else if (mode == "knee lift") {
        if (lknee[2] > rknee[2]) {
            drawCircleX = ofMap(rknee[2], 0, 4, 1000, 1050);
        } else {
            drawCircleX = ofMap(lknee[2], 0, 4, 950, 1000);
        }
    }
    
    //SESSION INFO
    ofColor txtBgColor(250, 100); // sets the background color of interface text
    ofSetColor(txtBgColor);
    ofDrawRectangle(0, 660, 1280, 60);
    
    scoreColor = ofMap(incrScore, 0.0, 1.0, 0, 255);
    ofSetColor(255-scoreColor, scoreColor, 0);
    if (bShowMoveIndicator) {
        ofDrawCircle(drawCircleX, 690, 20 ); //movement indicator
    }
    

    stringstream ss(selectedMovie);
    string movieName = "";
    getline(ss, movieName, '.'); // removes file extension from movie filename
    ofSetColor(10);
    verdana30.drawString(movieName,20,700);
    verdana30.drawString(selectedSong ,370,700);

    
//    verdana30.drawString("Tempo: " + ofToString(bpm) + " BPM" ,20,630);
//    verdana30.drawString("Hastighet: " + ofToString(movie.getSpeed(),2),20,650);
//    verdana30.drawString("Forandring: " + ofToString(movDelta),20,530);
    
    //PERFORMANCE METRICS
    ofSetColor(255-scoreColor, scoreColor, 0);
    verdana30.drawString("Poeng: " + ofToString(score, 1),1000,700);
    verdana30.drawString("/" + ofToString(movementData.size()-sessionEndIndex),1200,700);
    ofDrawRectangle(ofMap(songProgress, 0.0, 1.0, 0.0, 1280), 640,10,20); //progress indicator
    ofSetColor(255);

    }


//}

//----------------------------------------------------------
void ofApp::nextSong() {
    for (int i = 0; i < songList.size(); i++) {
        if (songList[i] == selectedSong) {
            selectedSong = songList[i+2];
            defaultBeatPeriod = ofToInt(songList[i+3]);
            break;
        }
    }
}

//----------------------------------------------------------
void ofApp::previousSong() {
    for (int i = 0; i < songList.size(); i++) {
        if (songList[i] == selectedSong) {
            selectedSong = songList[i-2];
            defaultBeatPeriod = ofToInt(songList[i-1]);
            break;
        }
    }
}

//----------------------------------------------------------
void ofApp::nextMovie() {
    for (int i = 0; i < movieList.size(); i++) {
        ofSendMessage(movieList[i] + ", " + selectedMovie);
        if (movieList[i] == selectedMovie) {
            selectedMovie = movieList[i+1];
            ofSendMessage("NEXT MOVIE!");
            break;
        }
    }
}

//----------------------------------------------------------
void ofApp::prevMovie() {
    for (int i = 0; i < movieList.size(); i++) {
        ofSendMessage(movieList[i] + ", " + selectedMovie);
        if (movieList[i] == selectedMovie) {
            selectedMovie = movieList[i-1];
            ofSendMessage("PREV MOVIE!");
            break;
        }
    }
}
//--------------------------------------------------------------
//void ofApp::calcMetrics(){
//    movDelta = -abs((movie.getSpeed()-prevSpeed)*(100.0f));
//    prevSpeed = movie.getSpeed();
//}

//--------------------------------------------------------------
void ofApp::checkModeChange(string currentMode) {
    if (bModeChanged && mode != "setup") {
//        serial.stopContinuousRead();
        ofSendMessage("Selected gesture: \"" + currentMode +"\"");
        bModeChanged = false;
    } else if (/*bModeChanged && */ mode == "setup") {
//        serial.startContinuousRead();
//        ofAddListener(serial.NEW_MESSAGE,this,&ofApp::onNewMessage);
        message = "";
        //        ofSendMessage("Mode: setup");
        bModeChanged = false;
     }
}

//--------------------------------------------------------------
void ofApp::selectionTimer() {
    if (bSelectionTimerReached && bHandIsInside) {
        selectionStartTime = ofGetElapsedTimeMillis();  // get the start time
        bSelectionTimerReached = false;
    }
    // update the timer this frame
    if (!bSelectionTimerReached && bHandIsInside) {
        float timer = ofGetElapsedTimeMillis() - selectionStartTime;
        if(timer >= selectionDuration) {
            bSelectionTimerReached = true;
            sessionEndIndex = movementData.size();
            ofMessage msg("Timer Reached");
            ofSendMessage(msg);
        }
    } else if (!bHandIsInside) {
        selectionStartTime = ofGetElapsedTimeMillis();
    }
    
    //    ofMessage msg("selectionTimer!!");
    //    ofSendMessage(msg);
}

//--------------------------------------------------------------
void ofApp::drawInterface() //DRAWS USER INTERFACE IN SETUP MODE
{
    int hue = 127;
    int sat = 200;
    int bright = 155;
    int alpha = 200;
    ofColor green = ofColor::fromHsb(hue, sat, bright, alpha);
    hue = 43;
    sat = 185;
    bright = 220;
    ofColor yellow = ofColor::fromHsb(hue, sat, bright, alpha);
    hue = 243;
    sat = 125;
    bright = 255;
    ofColor pink = ofColor::fromHsb(hue, sat, bright, alpha);
    
    
    ofColor txtBgColor(10, 10, 10, 200); // sets the background color of interface text
    //CENTER ELEMENT
    ofSetColor(txtBgColor);
    ofDrawRectRounded(interfacePosX - 100, interfacePosY - 175, 205, 50, rectCornerRad);
//    int hue = 127;
//    int sat = 200;
//    int bright = 155;
//    int alpha = 150;
//    ofColor color = ofColor::fromHsb(hue, sat, bright, alpha);
//    ofSetColor(color);
    ofSetColor(green);
    ofDrawCircle(interfacePosX, interfacePosY - 80, radius);
    ofSetColor(yellow);
    ofDrawCircle(interfacePosX, interfacePosY - 80, radius-15);
    ofSetColor(255);
    verdana30.drawString("Svai fra side til side", interfacePosX - 90, interfacePosY - 140);
    
    //LEFT ELEMENT
    ofSetColor(txtBgColor);
    ofDrawRectRounded(interfacePosX - 370, interfacePosY - 45, 190, 50, rectCornerRad);
    ofSetColor(green);
    ofDrawCircle(interfacePosX - 130, interfacePosY - 20, radius);
    ofSetColor(yellow);
    ofDrawCircle(interfacePosX - 130, interfacePosY - 20, radius-15);
    ofSetColor(pink);
    ofDrawCircle(interfacePosX - 130, interfacePosY - 20, radius-30);
    ofSetColor(255);
    verdana30.drawString("Sving med armene", interfacePosX - 360, interfacePosY - 10);

    //RIGHT ELEMENT
    ofSetColor(txtBgColor);
    ofDrawRectRounded(interfacePosX + 180, interfacePosY - 45, 95, 50, rectCornerRad);
    ofSetColor(green);
    ofDrawCircle(interfacePosX + 130, interfacePosY - 20, radius);
    ofSetColor(pink);
    ofDrawCircle(interfacePosX + 130, interfacePosY - 20, radius-15);
    ofSetColor(255);
    verdana30.drawString("Knebøy", interfacePosX + 190, interfacePosY - 10);

    //FAR LEFT ELEMENT
    ofSetColor(txtBgColor);
    ofDrawRectRounded(interfacePosX - 465, interfacePosY + 55, 215, 50, rectCornerRad);
    ofSetColor(green);
    ofDrawCircle(interfacePosX - 200, interfacePosY + 80, radius);
    ofSetColor(yellow);
    ofDrawCircle(interfacePosX - 200, interfacePosY + 80, radius-15);
    ofSetColor(pink);
    ofDrawCircle(interfacePosX - 200, interfacePosY + 80, radius-30);
    ofSetColor(255);
    verdana30.drawString("Hånd fra side til side", interfacePosX - 455, interfacePosY + 90);
    
    //FAR RIGHT ELEMENT
    ofSetColor(txtBgColor);
    ofDrawRectRounded(interfacePosX + 250, interfacePosY + 55, 100, 50, rectCornerRad);
    ofSetColor(green);
    ofDrawCircle(interfacePosX + 200, interfacePosY + 80, radius);
    ofSetColor(yellow);
    ofDrawCircle(interfacePosX + 200, interfacePosY + 80, radius-15);
    ofSetColor(pink);
    ofDrawCircle(interfacePosX + 200, interfacePosY + 80, radius-30);
    ofSetColor(255);
    verdana30.drawString("Kneløft", interfacePosX + 260, interfacePosY + 90);
    
//    ofSetColor(0);
//    verdana30.drawString("X: " + ofToString(interfacePosX),20,480);
//    verdana30.drawString("Y: " + ofToString(interfacePosY),20,505);
    ofSetColor(100);
}

//--------------------------------------------------------------
void ofApp::checkHandPosition() //USED IN SETUP MODE TO ALLOW USER SELECTION
{
    interfacePosX = ofMap(head[0], 0.0f, 1.0f, 1000, 0);
    interfacePosY = ofMap(head[1], 0.0f, 1.0f, 100, 680);
    if (lhnd[1] > rhnd[1]) { // CHECK WHICH HAND IS IN FRONT OF THE OTHER
        selectorHandX = ofMap(rhnd[0], 0.0f, 1.0f, 1000, 0);
        selectorHandY = ofMap(rhnd[1], 1.0f, 0.0f, 680, 0);
    } else {
        selectorHandX = ofMap(lhnd[0], 0.0f, 1.0f, 1000, 0);
        selectorHandY = ofMap(lhnd[1], 1.0f, 0.0f, 680, 0);
    }
    if (selectorHandX > (interfacePosX + 155) &&
        selectorHandX < (interfacePosX + 205) &&
        selectorHandY > (interfacePosY + 35) &&
        selectorHandY < (interfacePosY + 85)) { //FAR RIGHT POSITION
        bHandIsInside = true;
        selectionTimer();
        if (bSelectionTimerReached) {
            mode = "knee lift";
            noiseThreshold = 0.05f; //LOW-PASS FILTER
            bModeChanged = true;
            sendSongTitleOsc();
            score = 0;
        }
    } else if (selectorHandX < (interfacePosX - 195) &&
               selectorHandX > (interfacePosX - 245) &&
               selectorHandY > (interfacePosY + 35) &&
               selectorHandY < (interfacePosY + 85)) { //FAR LEFT POSITION
        bHandIsInside = true;
        selectionTimer();
        if (bSelectionTimerReached) {
            mode = "conductor";
            noiseThreshold = 0.01f; //LOW-PASS FILTER
            bModeChanged = true;
            sendSongTitleOsc();
            score = 0;
        }
    } else if (selectorHandX > (interfacePosX + 90) &&
               selectorHandX < (interfacePosX + 140) &&
               selectorHandY > (interfacePosY - 75) &&
               selectorHandY < (interfacePosY - 25)) { //RIGHT POSITION
        bHandIsInside = true;
        selectionTimer();
        if (bSelectionTimerReached) {
            mode = "knee bend";
            noiseThreshold = 0.005f; //LOW-PASS FILTER
            bModeChanged = true;
            sendSongTitleOsc();
            score = 0;
        }
    } else if (selectorHandX < (interfacePosX - 120) &&
               selectorHandX > (interfacePosX - 170) &&
               selectorHandY > (interfacePosY - 75) &&
               selectorHandY < (interfacePosY - 25)) { //LEFT POSITION
        bHandIsInside = true;
        selectionTimer();
        if (bSelectionTimerReached) {
            mode = "power walk";
            noiseThreshold = 0.1f; //LOW-PASS FILTER
            bModeChanged = true;
            sendSongTitleOsc();
            score = 0;
        }
    } else if (selectorHandX > (interfacePosX - 40) &&
               selectorHandX < (interfacePosX + 10) &&
               selectorHandY < (interfacePosY - 75) &&
               selectorHandY > (interfacePosY - 125)) { //CENTER POSITION
        bHandIsInside = true;
        selectionTimer();
        if (bSelectionTimerReached) {
            mode = "stevie wonder";
            noiseThreshold = 0.005f; //LOW-PASS FILTER
            bModeChanged = true;
            sendSongTitleOsc();
            score = 0;
        }
    } else {
        bHandIsInside = false;
        selectionTimer();
    }
}

//--------------------------------------------------------------
void ofApp::sendSongTitleOsc() {
    ofxOscMessage song; // create an OSC message
    song.setAddress("/song"); // create an OSC address
    song.addStringArg(selectedSong); // assign a value to the message
    sender.sendMessage(song, false); // send message via OSC
    if (previousMovie != selectedMovie) {
        changeMovie();
        previousMovie = selectedMovie;
    }

}

//--------------------------------------------------------------
void ofApp::changeMovie() {
    movie.load("movies/" + selectedMovie);
    movie.setLoopState(OF_LOOP_NORMAL);
    movie.play();
//
    
}

//--------------------------------------------------------------
//void ofApp::onNewMessage(string & message)
//{
//    delSpaces(message);
//    nfcId = message;
//    cout << "nfcId:" << nfcId << "\n";
//    for (int i = 0; i < songList.size(); i++) {
//        if (songList[i] == nfcId) { //check if there is a song with an ID that matches the nfcId
//            selectedSong = songList[i+1]; // select the song title on the next line
//            defaultBeatPeriod = ofToInt(songList[i+2]);
//            ofSendMessage("Selected song: " + selectedSong + ", Tempo: " + ofToString(defaultBeatPeriod));
//        }
//    }
//    for (int i = 0; i < movieList.size(); i++) {
//        if (movieList[i] == nfcId) { //check if there is a song with an ID that matches the nfcId
//            selectedMovie = movieList[i+1]; // select the song title on the next line
//            ofSendMessage("Selected movie: " + selectedMovie);
//        }
//    }
//    
//}

//--------------------------------------------------------------
float ofApp::lowPassFilter1(float sensorVal){
    filterValue1[0] = filterValue1[1];
    filterValue1[1] = filterValue1[2];
    filterValue1[2] = filterValue1[3];
    filterValue1[3] = sensorVal;
    float returnVal = 0.25 * filterValue1[3] + 0.25 * filterValue1[2] + 0.25 * filterValue1[1] + 0.25 * filterValue1[0];
    return returnVal;
}

float ofApp::lowPassFilter2(float sensorVal){
    filterValue2[0] = filterValue2[1];
    filterValue2[1] = filterValue2[2];
    filterValue2[2] = filterValue2[3];
    filterValue2[3] = sensorVal;
    float returnVal = 0.25 * filterValue2[3] + 0.25 * filterValue2[2] + 0.25 * filterValue2[1] + 0.25 * filterValue2[0];
    return returnVal;
}

float ofApp::lowPassFilter3(float sensorVal){
    filterValue3[0] = filterValue3[1];
    filterValue3[1] = filterValue3[2];
    filterValue3[2] = filterValue3[3];
    filterValue3[3] = sensorVal;
    float returnVal = 0.25 * filterValue3[3] + 0.25 * filterValue3[2] + 0.25 * filterValue3[1] + 0.25 * filterValue3[0];
    return returnVal;
}

float ofApp::lowPassFilter4(float sensorVal){
    filterValue4[0] = filterValue4[1];
    filterValue4[1] = filterValue4[2];
    filterValue4[2] = filterValue4[3];
    filterValue4[3] = sensorVal;
    float returnVal = 0.25 * filterValue4[3] + 0.25 * filterValue4[2] + 0.25 * filterValue4[1] + 0.25 * filterValue4[0];
    return returnVal;
}

float ofApp::lowPassFilter5(float sensorVal){
    filterValue5[0] = filterValue5[1];
    filterValue5[1] = filterValue5[2];
    filterValue5[2] = filterValue5[3];
    filterValue5[3] = sensorVal;
    float returnVal = 0.25 * filterValue5[3] + 0.25 * filterValue5[2] + 0.25 * filterValue5[1] + 0.25 * filterValue5[0];
    return returnVal;
}

float ofApp::lowPassFilter6(float sensorVal){
    filterValue6[0] = filterValue6[1];
    filterValue6[1] = filterValue6[2];
    filterValue6[2] = filterValue6[3];
    filterValue6[3] = sensorVal;
    float returnVal = 0.25 * filterValue6[3] + 0.25 * filterValue6[2] + 0.25 * filterValue6[1] + 0.25 * filterValue6[0];
    return returnVal;
}



//--------------------------------------------------------------
bool ofApp::conductor(){
    bool retVal;
    float tmp_hand, tmp_prev_hand;
    
    if (axis == 'x') {
        tmp_hand = (lhnd[0] + rhnd[0]) /2.0; // average of both hands as virtual "third" hand
        tmp_prev_hand = (prev_lhnd[0] + prev_rhnd[0]) /2.0;
        prev_lhnd[0] = lhnd[0];
        prev_rhnd[0] = rhnd[0];
    } else if (axis == 'y') {
        tmp_hand = (lhnd[1] + rhnd[1]) /2.0; // average of both hands as virtual "third" hand
        tmp_prev_hand = (prev_lhnd[1] + prev_rhnd[1]) /2.0;
        prev_lhnd[1] = lhnd[1];
        prev_rhnd[1] = rhnd[1];
    } else if (axis == 'z') {
        tmp_hand = (lhnd[2] + rhnd[2]) /2.0; // average of both hands as virtual "third" hand
        tmp_prev_hand = (prev_lhnd[2] + prev_rhnd[2]) /2.0;
        prev_lhnd[2] = lhnd[2];
        prev_rhnd[2] = rhnd[2];
    }
    
    if (tmp_hand - tmp_prev_hand > noiseThreshold && lastDir == "right") {
        //moving left, reset not-moving counter
        retVal = true;
        lastDir = "left";
        notMovingCounter = 0;
        ofMessage msg("LEFT");
        ofSendMessage(msg);
    } else if (tmp_hand - tmp_prev_hand < -noiseThreshold && lastDir == "left"){
        //moving right, reset not-moving counter
        retVal = true;
        lastDir = "right";
        notMovingCounter = 0;
        ofMessage msg("RIGHT");
        ofSendMessage(msg);
    } else if (tmp_hand - tmp_prev_hand >= -noiseThreshold && tmp_hand - tmp_prev_hand <= noiseThreshold) {
        // count to 5-10, not moving -> close
        retVal = false;
        notMovingCounter++;
        //                ofMessage msg("NOT MOVING!! Count: " + ofToString(notMovingCounter));
        //                ofSendMessage(msg);
    } else {
        retVal = false;
        //                ofMessage msg("REPEAT");
        //                ofSendMessage(msg);
    }
    return retVal;
    
}

//--------------------------------------------------------------
bool ofApp::powerWalk(){
    bool retVal;
    float difference;
    if (axis == 'x') {
        difference = lhnd[0] - rhnd[0];
    } else if (axis == 'y') {
        difference = lhnd[1] - rhnd[1];
    } else if (axis == 'z') {
        difference = lhnd[2] - rhnd[2];
    }

    if (difference > noiseThreshold && lastDir == "left") {
        //right arm forward, reset not-moving counter
        retVal = true;
        lastDir = "right";
        notMovingCounter = 0;
        ofMessage msg("RIGHT");
        ofSendMessage(msg);
    } else if (difference < -noiseThreshold && lastDir == "right"){
        //left arm forward, reset not-moving counter
        retVal = true;
        lastDir = "left";
        notMovingCounter = 0;
        ofMessage msg("LEFT");
        ofSendMessage(msg);
    } else if (difference >= -noiseThreshold && difference <= noiseThreshold) {
        // count to 5-10, not moving -> close
        retVal = false;
        notMovingCounter++;
        //                ofMessage msg("NOT MOVING!! Count: " + ofToString(notMovingCounter));
        //                ofSendMessage(msg);
    } else {
        retVal = false;
    }
    //    ofMessage msg("Left value : right value: difference " + ofToString(lhnd[2]) + " : " + ofToString(rhnd[2]) + " : " + ofToString(difference));
    //    ofSendMessage(msg);
//    prev_lhnd[2] = lhnd[2];
//    prev_rhnd[2] = rhnd[2];
    return retVal;
    
}

//--------------------------------------------------------------
bool ofApp::stevieWonder(){
    bool retVal;
    float current;
    float previous;
    
    if (axis == 'x') {
        current = head[0];
        previous = prev_head[0];
        prev_head[0] = head[0];
    } else if (axis == 'y') {
        current = head[1];
        previous = prev_head[1];
        prev_head[1] = head[1];
    } else if (axis == 'z') {
        current = head[2];
        previous = prev_head[2];
        prev_head[2] = head[2];
    }
    
    if (current - previous > noiseThreshold && lastDir == "right") {
        //direction has changed, moving left, reset not-moving counter
        retVal = true;
        lastDir = "left";
        notMovingCounter = 0;
        ofMessage msg("LEFT");
        ofSendMessage(msg);
    } else if (current - previous < -noiseThreshold && lastDir == "left"){
        //direction has changed, moving right, reset not-moving counter
        retVal = true;
        lastDir = "right";
        notMovingCounter = 0;
        ofMessage msg("RIGHT");
        ofSendMessage(msg);
    } else if (current - previous >= -noiseThreshold && current - previous <= noiseThreshold) {
        // count to 5-10, not moving -> close
        retVal = false;
        notMovingCounter++;
        //        ofMessage msg("NOT MOVING!! Count: " + ofToString(notMovingCounter));
        //        ofSendMessage(msg);
    } else {
        retVal = false;
        //        ofMessage msg("REPEAT");
        //        ofSendMessage(msg);
    }
    return retVal;
}

//--------------------------------------------------------------
bool ofApp::kneeBend(){
    bool retVal;
    float current = head[1];
    float previous = prev_head[1];
    prev_head[1] = head[1];
    
    if (current - previous > noiseThreshold && lastDir == "right") {
        //direction has changed, moving left, reset not-moving counter
        retVal = true;
        lastDir = "left";
        notMovingCounter = 0;
        ofMessage msg("LEFT");
        ofSendMessage(msg);
    } else if (current - previous < -noiseThreshold && lastDir == "left"){
        //direction has changed, moving right, reset not-moving counter
        retVal = true;
        lastDir = "right";
        notMovingCounter = 0;
        ofMessage msg("RIGHT");
        ofSendMessage(msg);
    } else if (current - previous >= -noiseThreshold && current - previous <= noiseThreshold) {
        // count to 5-10, not moving -> close
        retVal = false;
        notMovingCounter++;
        //        ofMessage msg("NOT MOVING!! Count: " + ofToString(notMovingCounter));
        //        ofSendMessage(msg);
    } else {
        retVal = false;
        //        ofMessage msg("REPEAT");
        //        ofSendMessage(msg);
    }
    return retVal;
}

//--------------------------------------------------------------
bool ofApp::kneeLift(){
    bool retVal;
    float difference = lknee[2] - rknee[2];
    if (difference > noiseThreshold && lastDir == "left") {
        //right arm forward, reset not-moving counter
        retVal = true;
        lastDir = "right";
        notMovingCounter = 0;
        ofMessage msg("RIGHT");
        ofSendMessage(msg);
    } else if (difference < -noiseThreshold && lastDir == "right"){
        //left arm forward, reset not-moving counter
        retVal = true;
        lastDir = "left";
        notMovingCounter = 0;
        ofMessage msg("LEFT");
        ofSendMessage(msg);
    } else if (difference >= -noiseThreshold && difference <= noiseThreshold) {
        // count to 5-10, not moving -> close
        retVal = false;
        notMovingCounter++;
        //                ofMessage msg("NOT MOVING!! Count: " + ofToString(notMovingCounter));
        //                ofSendMessage(msg);
    } else {
        retVal = false;
    }
    //    ofMessage msg("Left value : right value: difference " + ofToString(lhnd[2]) + " : " + ofToString(rhnd[2]) + " : " + ofToString(difference));
    //    ofSendMessage(msg);
    prev_lknee[2] = lknee[2];
    prev_rknee[2] = rknee[2];
    return retVal;
    
}

//--------------------------------------------------------------
bool ofApp::conductorMirror(){
    bool retVal;
    float tmp_hand, tmp_prev_hand;
    
    tmp_hand = (lhnd[0] - rhnd[0]); // average of both hands as virtual "third" hand
    tmp_prev_hand = (prev_lhnd[0] - prev_rhnd[0]);
    prev_lhnd[0] = lhnd[0];
    prev_rhnd[0] = rhnd[0];
    
    if (tmp_hand - tmp_prev_hand > noiseThreshold && lastDir == "right") {
        //moving left, reset not-moving counter
        retVal = true;
        lastDir = "left";
        notMovingCounter = 0;
        ofMessage msg("LEFT");
        ofSendMessage(msg);
    } else if (tmp_hand - tmp_prev_hand < -noiseThreshold && lastDir == "left"){
        //moving right, reset not-moving counter
        retVal = true;
        lastDir = "right";
        notMovingCounter = 0;
        ofMessage msg("RIGHT");
        ofSendMessage(msg);
    } else if (tmp_hand - tmp_prev_hand >= -noiseThreshold && tmp_hand - tmp_prev_hand <= noiseThreshold) {
        // count to 5-10, not moving -> close
        retVal = false;
        notMovingCounter++;
        //                ofMessage msg("NOT MOVING!! Count: " + ofToString(notMovingCounter));
        //                ofSendMessage(msg);
    } else {
        retVal = false;
        //                ofMessage msg("REPEAT");
        //                ofSendMessage(msg);
    }
    return retVal;
    
}

//--------------------------------------------------------------
bool ofApp::shoulderTwist(){
    bool retVal;
    float difference = lsho[2] - rsho[2];
    
    if (difference > noiseThreshold && lastDir == "left") {
        //right arm forward, reset not-moving counter
        retVal = true;
        lastDir = "right";
        notMovingCounter = 0;
        ofMessage msg("RIGHT");
        ofSendMessage(msg);
    } else if (difference < -noiseThreshold && lastDir == "right"){
        //left arm forward, reset not-moving counter
        retVal = true;
        lastDir = "left";
        notMovingCounter = 0;
        ofMessage msg("LEFT");
        ofSendMessage(msg);
    } else if (difference >= -noiseThreshold && difference <= noiseThreshold) {
        // count to 5-10, not moving -> close
        retVal = false;
        notMovingCounter++;
        //                ofMessage msg("NOT MOVING!! Count: " + ofToString(notMovingCounter));
        //                ofSendMessage(msg);
    } else {
        retVal = false;
    }
    //    ofMessage msg("Left value : right value: difference " + ofToString(lsho[2]) + " : " + ofToString(rsho[2]) + " : " + ofToString(difference));
    //    ofSendMessage(msg);
    //    prev_lsho[2] = lsho[2];
    //    prev_rsho[2] = rsho[2];
    return retVal;
    
}

//--------------------------------------------------------------
bool ofApp::sampleTimer(){
    float currentTime = ofGetElapsedTimeMicros();
    // update the timer this frame
    float timerVal = currentTime - prevTime;
    bool retVal;
    if(timerVal >= samplePeriod) {
        retVal = true;
        prevTime = currentTime;
        //        ofMessage msg("Beat time: " + ofToString(timerVal));
        //        ofSendMessage(msg);
        
        
    } else retVal = false;
    
    return retVal;
}

//--------------------------------------------------------------
void ofApp::storeBeatTime(){
    unsigned int currentTime = ofGetElapsedTimeMicros();
    // update the timer this frame
    float beatTime = (currentTime - prevBeatTime) * .001;
    prevBeatTime = currentTime;
    
    if (beatTime > 350 && beatTime < 3500) { // if beat period is longer than 500 ms (eliminates state-transition jitter) AND shorter than 2500 ms (ignores too long movements) -> USER ADJUSTABLE!
        movementData.push_back(beatTime); //store beatTime
        beatPeriod.push_back(movementAvg);
        updateScore(beatTime);
        incrScoreSum.push_back(score);
        movementPatternChanges.push_back(movementPattern);
        axisChanges.push_back(axis);
        
        ofMessage msg("Gesture time: " + ofToString(movementData.operator[](movementData.size() - 1)) + "ms, Index: " + ofToString(movementData.size() - 1 - sessionEndIndex));
        ofSendMessage(msg);
    }
}

//--------------------------------------------------------------
void ofApp::updateScore(float lastMove){
    float tmp = lastMove / movementAvg;
    incrScore = 0.0f;
    
    if (tmp < 2 ) {
        if (tmp > 1.0) incrScore = 1 - (tmp - 1.0);
        else if (tmp <= 1.0) incrScore = tmp;
    }
    ofSendMessage("Debug: lastMove = " + ofToString(lastMove));
    ofSendMessage("Debug: movementAvg = " + ofToString(movementAvg));
    ofSendMessage("Debug: tmp (lastMove/movementAvg) = " + ofToString(tmp));
    ofSendMessage("Debug: incrScore = " + ofToString(incrScore));
    if (movementData.size()-sessionEndIndex > 4) { // don't update the score for the first
        score = score + incrScore; //Update new score sum
        
    }

}

//--------------------------------------------------------------
void ofApp::saveData(){
    mode = "setup";
    
    string fileName = ofGetTimestampString();
    saveFile.open (fileName + ".txt");
    saveFile << selectedSong << " / " << selectedMovie << "\n";
    saveFile << "Movement pattern: " << movementPattern << "\n";
    saveFile << "Default beat period: " << defaultBeatPeriod << "\n";
    saveFile << "Poeng: " << score << "/" << movementData.size()-sessionEndIndex << "\n";
    for (int i = sessionEndIndex; i < movementData.size(); i++) {
        saveFile << i-sessionEndIndex << ", " << movementData[i] << ", " << beatPeriod[i] << ", " << incrScoreSum[i] << ", " << movementPatternChanges[i] << ", " << axisChanges[i] << "\n";
        
    }
    saveFile.close();
    //            ofSendMessage("movementData.size = " + ofToString(movementData.size()));
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key) {
        case 'f':
            ofToggleFullscreen();
            break;
        case 'p':
            if (mode == "setup") {
                mode = prevMode;
            } else {
                prevMode = mode;
                mode = "setup";
            }
            break;
        case '0':
            sendSongTitleOsc();
            sessionEndIndex = movementData.size();
            score = 0;
            break;
        case '1':
            mode = "conductor";
            ofSendMessage("Selected gesture: \"conductor\"");
            noiseThreshold = 0.01f; //LOW-PASS FILTER
            bModeChanged = true;
            break;
        case '2':
            mode = "power walk";
            ofSendMessage("Selected gesture: \"power walk\"");
            noiseThreshold = 0.1f; //LOW-PASS FILTER
            bModeChanged = true;
            break;
        case '3':
            mode = "stevie wonder";
            ofSendMessage("Selected gesture: \"stevie wonder\"");
            noiseThreshold = 0.005f; //LOW-PASS FILTER
            bModeChanged = true;
            break;
        case '4':
            mode = "knee bend";
            ofSendMessage("Selected gesture: \"knee bend\"");
            noiseThreshold = 0.005f; //LOW-PASS FILTER
            bModeChanged = true;
            break;
        case '5':
            mode = "knee lift";
            ofSendMessage("Selected gesture: \"knee lift\"");
            noiseThreshold = 0.05f; //LOW-PASS FILTER
            bModeChanged = true;
            break;
        case '6':
            mode = "conductor mirror";
            ofSendMessage("Selected gesture: \"conductor mirror\"");
            noiseThreshold = 0.01f; //LOW-PASS FILTER
            bModeChanged = true;
            break;
        case '7':
            mode = "shoulder twist";
            ofSendMessage("Selected gesture: \"shoulder twist\"");
            noiseThreshold = 0.01f; //LOW-PASS FILTER
            bModeChanged = true;
            break;
        case '9':
            mode = "play music";
            bModeChanged = true;
            break;
        case 'm':
            if (bShowMoveIndicator) {
                bShowMoveIndicator = false;
            } else {
                bShowMoveIndicator = true;
            }
            break;
        case 's':
            saveData();
            //ofSendMessage("Setup mode!");
            bModeChanged = true;
            break;
        case 'x':
            axis = 'x';
            break;
        case 'y':
            axis = 'y';
            break;
        case 'z':
            axis = 'z';
            break;
        case ',':
            bDisableMovie = false;
            bDisableMusic = true;
            break;
        case '.':
            bDisableMovie = true;
            bDisableMusic = false;
            break;
        case '-':
            bDisableMovie = false;
            bDisableMusic = false;
            break;
        case OF_KEY_RIGHT:
            nextSong();
            break;
        case OF_KEY_LEFT:
            previousSong();
            break;
        case OF_KEY_UP:
            prevMovie();
            ofSendMessage("UP!");
            break;
        case OF_KEY_DOWN:
            nextMovie();
//            changeMovie();
            ofSendMessage("DOWN!");
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
string ofApp::delSpaces(string &str){
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    return str;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    cout << "Debug: " + msg.message << endl;
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}
