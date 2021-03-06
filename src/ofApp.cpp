/*
 notes:
 - i'm testing doing the cv with a smaller image and drawing the image from VideGrabber
 - It works not so good and I'm not sure if it computationally less demanding (cpu 74%, memory 98%).
 */


#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    
    // Define the capture size of the c�mera
    camWidth = 1280;
    camHeight = 720;
    
    /* THIS APLIES FX TO THE FISH - NOT TO BE USED
     fishFx.init(ofGetWindowWidth(), ofGetWindowHeight());
     fishFx.setFlip(false);
     
     //    fishFx.createPass<KaleidoscopePass>();
     //   fishFx.createPass<FxaaPass>();
     //   fishFx.createPass<BloomPass>();
     //   fishFx.createPass<DofPass>(); // blur
     //   fishFx.createPass<GodRaysPass>(); //
     //   fishFx.createPass<NoiseWarpPass>();
     //   fishFx.createPass<VerticalTiltShifPass>();
     //   fishFx.createPass<PixelatePass>();
     //    fishFx.createPass<EdgePass>();
     */
    
    flickIntensity = 0;
    masterBpm = 120;
    fadeScreenIntensity = 0;
    fadeScreenIntensityWhite = 0;
    lineHiVel = 0;
    lineLowVel = 0;
    rectOpacity = 0;
    rainInt = 0;
    rainF = 0;
    prtInt = 255;
    
    // ********* FOR PARTICLES (line around the fish) *****************
    int num = 15;
    p.assign(num, demoParticle());
    currentMode = PARTICLE_MODE_ATTRACT;
    currentModeStr = "1 - PARTICLE_MODE_ATTRACT: attracts to mouse";
    resetParticles();
    
    
    // ********* FOR Glitches *****************
    // myFbo.allocate(ofGetWidth(),ofGetHeight());
    // myGlitch.setup(&myFbo);
    
    
    // ********* define an initial ROI - Region Of Interest *********
    ROI.width = 1265; // set it to camWidth to have ROI = to camera size
    ROI.height = 390;// set it to camHeight to have ROI = to camera size
    ROI.x = 10; // set it to zero to get ROI = camwidth
    ROI.y = 330; // set it to zero to get ROI = camheight
    
    paralax_x = (camWidth-ROI.width)*0.5;
    paralax_y = (camHeight-ROI.height)*0.5;
    
    // Define a scale ratio to resize the original image for analysis
    scaleRatio = 4;
    
#ifdef _USE_LIVE_VIDEO
    // Get back a list of devices (cameras).
    vector<ofVideoDevice> devices = vidGrabber.listDevices();
    
    for(int i = 0; i < devices.size(); i++){
        cout << devices[i].id << ": " << devices[i].deviceName;
        if( devices[i].bAvailable ){
            cout << endl;
        }else{
            cout << " - unavailable " << endl;
        }
    }
    vidGrabber.setDeviceID(0);  // use camera 0 for the analysis
    
    vidGrabber.initGrabber(camWidth,camHeight);
#else
    vidPlayer.loadMovie("fish_movie.mov");
    vidPlayer.play();
#endif
    
    // **** allocate memory for different images used along the way
    colorImg.allocate(camWidth, camHeight);
    grayTempImage.allocate(camWidth, camHeight);
    grayBg.allocate(ROI.width/scaleRatio, ROI.height/scaleRatio);
    
    
    //******** Selects the method for learning background ***********
    bLearnBakground = false; // learn from video ('space bar')
    bLoadPictureBakground = true; // load from picture file ('p' key)
    
    //******** threshold used for image analysis ******************
    threshold = 50;
    
    
    // *********** MIDI IN SETUP **************
    // print input ports to console
    midiIn.listPorts(); // via instance
    
    // open port by number (you may need to change this)
    //midiIn.openPort(1);
    midiIn.openPort("nanoKONTROL SLIDER/KNOB");	// by name
    
    // don't ignore sysex, timing, & active sense messages,
    // these are ignored by default
    midiIn.ignoreTypes(false, false, false);
    
    // add testApp as a listener
    midiIn.addListener(this);
    
    // print received messages to the console
    midiIn.setVerbose(true);
    
    
    // *********** MIDI OUT **************
    midiOut.listPorts();
    midiOut.openPort(0);
    
    
    ofSetFrameRate(15);
    
    // *********** RAIN **************
    myRain.reset();
}



//--------------------------------------------------------------
void ofApp::update(){
    
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    masterBpm = 120;
    
    ofBackground(255,255,255);
    bool bNewFrame = false;
    
    paralax_x = (camWidth-ROI.width)*0.5;
    paralax_y = (camHeight-ROI.height)*0.5;
    
    
    // ******* for glitch *************************
    // myGlitch.setFx(OFXPOSTGLITCH_CONVERGENCE,true);
    
    
    // ******************************************************************
    // ************** MIDI SETUP ****************************************
    // ******************************************************************
    
    // ---------------------- Rhythm Drone Intensity ----------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 17) {
        flickIntensity = midiMessage.value*2;
    }
    
    // ---------------------- Screen Fade out to black ----------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 33) {
        fadeScreenIntensity = midiMessage.value*2;
    }
    
    // ---------------------- Bubbles trigger ----------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 14 && midiMessage.value == 127) {
        fishBreath.resetParticles();
    }
    
    // ---------------------- Bubbles intensity (number of bubbles) ----------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 19) {
        fishBreath.bintensity = midiMessage.value;
    }
    
    // ---------------------- Fish Fade in (white) ----------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 40) {
        fadeScreenIntensityWhite = midiMessage.value*2;
    }
    
    // ---------------------- Line Low ----------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 37) {
        lineLowVel = midiMessage.value;
    }
    
    // ---------------------- Line Hi --------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 38) {
        lineHiVel = midiMessage.value;
    }
    
    // ---------------------- Line Low ----------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 21) {
        firstLine->setOpacity(midiMessage.value*2);
    }
    
    // ---------------------- Line Hi --------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 22) {
        secondLine->setOpacity(midiMessage.value*2);
    }
    
    // ---------------------- Retangle Opacity --------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 23) {
        rectOpacity = midiMessage.value*2;
    }
    // ---------------------- Rain Intensity --------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 18) {
        rainInt = midiMessage.value;
    }
    // ---------------------- Rain Frequency --------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 34) {
        rainF = midiMessage.value*3;
    }
    // ---------------------- Anchors Intensity --------------------------
    if (midiMessage.channel == 8 && midiMessage.status == MIDI_CONTROL_CHANGE && midiMessage.control == 20) {
        prtInt = midiMessage.value*2;
    }
    
    // ---------------------- MIDI OUT - Triggers midinotes everytime a drop of rain shows up on screen (channel 1) -----------------
    for (int i = 0; i < myRain.raining.size(); i++) {
        if (myRain.raining[i].pos.y > paralax_y && myRain.raining[i].pos.y < paralax_y+20) {
            midiOut.sendNoteOn(1, 100,  100);
        }
    }
    // ---------------------- MIDI OUT - Velocity of the fish (channel 2) -----------------
    // changes the way the SOLO fish sounds
    midiOut.sendNoteOn(2,myFish.velocity,  100);
    
    // ---------------------- MIDI OUT - Position of the fish (channel 3 and 4) -----------------
    // changes the way the SOLO fish sounds
    //midiOut.sendNoteOn(3,myFish.fishPos_1.x,  100); // for panning
    midiOut.sendControlChange(3, 180, myFish.fishPos_1.x/10);
    midiOut.sendControlChange(4, 180, ofMap(myFish.fishPos_1.y, 150.0, 600.0, 0.0, 127.0));
    //cout << "X: " << myFish.fishPos_1.x << endl;
    //cout << "Y: " << ofMap(myFish.fishPos_1.y, 150.0, 600.0, 0.0, 127.0) << endl;

    // ---------------------- MIDI OUT - Line 1  (channel 5) -----------------
//    midiOut.sendNoteOn(5,firstLine->getLinePosition(),  100);
    if (firstLine->getLinePosition() < 5 || firstLine->getLinePosition() > 1255) {
        midiOut.sendNoteOn(5, 100,  100);
    } ;
    
    // ---------------------- MIDI OUT - Line 2  (channel 6) -----------------
    //    midiOut.sendNoteOn(5,firstLine->getLinePosition(),  100);
    if (secondLine->getLinePosition() < 5 || secondLine->getLinePosition() > 1255) {
        midiOut.sendNoteOn(6, 100,  100);
    } ;

    // ---------------------- MIDI OUT - Square Size  (channel 7) -----------------
    midiOut.sendNoteOn(7, 0.2*(lineSquare(firstLine->getLinePosition(), secondLine->getLinePosition(), firstLine->getOpacity(), secondLine->getOpacity(), rectOpacity)), 100);

    
    // ************ LINE UPDATE *************************************************
    
    firstLine->lineUpdate(lineLowVel, 5);
    secondLine->lineUpdate(lineHiVel, 1);
    
    
#ifdef _USE_LIVE_VIDEO
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();
#else
    vidPlayer.update();
    bNewFrame = vidPlayer.isFrameNew();
#endif
    
    
    // Assigns a frame from the video/camera to a color image
    if (bNewFrame){
#ifdef _USE_LIVE_VIDEO
        colorImg.setFromPixels(vidGrabber.getPixels(), camWidth, camHeight);
#else
        colorImg.setFromPixels(vidPlayer.getPixels(), camWidth, camHeight);
#endif
        
        grayTempImage.clear();
        grayTempImage.allocate(camWidth, camHeight);
        grayTempImage = colorImg;
        grayTempImage.setROI(ROI);
        
        grayImage.clear();
        grayImage.allocate(ROI.width, ROI.height);
        grayImage.setFromPixels(grayTempImage.getRoiPixels(), ROI.width, ROI.height);
        grayImage.resize(ROI.width/scaleRatio, ROI.height/scaleRatio);
        
        
        //******** LEARN BACKGROUND (space bar) *******************
        if (bLearnBakground == true){
            grayBg.clear();
            grayBg.allocate(ROI.width/scaleRatio, ROI.height/scaleRatio);
            grayImage.scaleIntoMe(grayBg);
            grayBg = grayImage;
            
            bgImg.allocate(ROI.width/scaleRatio, ROI.height/scaleRatio, OF_IMAGE_GRAYSCALE);
            unsigned char * pixels = grayBg.getPixels();
            bgImg.setFromPixels(pixels, ROI.width/scaleRatio, ROI.height/scaleRatio, OF_IMAGE_GRAYSCALE);
            bgImg.saveImage("background-"+ofGetTimestampString()+".png");
            
            bLearnBakground = false;
            cout << "------------------" << endl;
            cout << "------------------" << endl;
            cout << "CAPTURE BACKGROUNG" << endl;
            cout << "------------------" << endl;
            cout << "------------------" << endl;
            
        }
        
        //******** LOAD BACKGROUND PICTURE ('p' key) *******************
        if (bLoadPictureBakground == true){
            loader.loadImage("background.png");
            cout << "load image from file" << endl;
            loader.setImageType(OF_IMAGE_GRAYSCALE);
            loader.resize(ROI.width/scaleRatio, ROI.width/scaleRatio);
            cout << "resize image" << endl;
            grayBg.clear();
            grayBg.allocate(ROI.width/scaleRatio, ROI.height/scaleRatio);
            grayBg.setFromPixels(loader.getPixels(),ROI.width/scaleRatio, ROI.height/scaleRatio);
            bLoadPictureBakground = false;
        }
        
        
        // take the abs value of the difference between background and incoming and then threshold:
        grayDiff.clear();
        grayDiff.allocate(ROI.width/scaleRatio, ROI.height/scaleRatio);
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        
        // **** find contours *******
        contourFinder.findContours(grayDiff, 1, (ROI.width/scaleRatio*ROI.height/scaleRatio/4), 1, false);
    }
    
    
    if (contourFinder.nBlobs > 0){
        fishPosSmall = ofVec2f(contourFinder.blobs[0].centroid.x, contourFinder.blobs[0].centroid.y);
        fishPosBig = ofVec2f(fishPosSmall.x*scaleRatio+(paralax_x), fishPosSmall.y*scaleRatio+(paralax_y));
    }
    
    // this method sends information for fish class variables
    myFish.makeFishToWork(camWidth, camHeight, fishPosBig.x, fishPosBig.y, ROI.width, ROI.height, paralax_x, paralax_y, 100);
    
    // cout << "velocity: " << myFish.getVelocity(fishPosBig.x,fishPosBig.y) << endl;
    
    
    // ************* Release bubbles *******************
    fishBreath.bubblesUpdate(fishPosBig);
    
    
    // *********  FOR PARTICLES ***************
    for(unsigned int i = 0; i < p.size(); i++){
        p[i].setFishPosPrt(fishPosBig);
        p[i].setMode(currentMode);
        p[i].update();
    }
    //lets add a bit of movement to the attract points
    for(unsigned int i = 0; i < attractPointsWithMovement.size(); i++){
        attractPointsWithMovement[i].x = attractPoints[i].x + ofSignedNoise(i * 10, ofGetElapsedTimef() * 0.7) * 12.0;
        attractPointsWithMovement[i].y = attractPoints[i].y + ofSignedNoise(i * -10, ofGetElapsedTimef() * 0.7) * 12.0;
    }
    
    // ************* Update Rain *******************
    myRain.rainUpdate(rainInt, rainF);
    
    // ************* Update Anchors Intensity *******************
    for (int i = 0; i < p.size(); i++){
        p[i].prtIntensity = prtInt;
    }
}



//--------------------------------------------------------------
void ofApp::draw(){
    
    // draw the incoming, the grayscale, the bg and the thresholded difference
    ofSetHexColor(0xffffff);
    
    // ********* GLITCH EFFECT ********* NOTTTT WORKING *****
    //  myFbo.draw((paralax_x)-ROI.x, (paralax_y)-ROI.y);
    //  myGlitch.generateFx();
    //  myFbo.draw((paralax_x)-ROI.x, (paralax_y)-ROI.y);
    
    
    // *********** draw the video **************************
    colorImg.draw((paralax_x)-ROI.x, (paralax_y)-ROI.y);
    
    
    // *********** Show/whide the fish **************************
    fadeScreenToWhite(fadeScreenIntensityWhite);
    
    
    //*********** CALIBRATION SYSTEM (z) *****************
    if (showCalibrationScreen) {
        
        // *** draw graySmall Image use (ROI scaled) ***
        ofSetHexColor(0xffffff);
        grayImage.draw(0, 0);
        ofNoFill();
        ofSetColor(255, 0, 0);
        ofRect(paralax_x, paralax_y, ROI.width, ROI.height);
        
        // *** draw background image in use ***
        ofSetHexColor(0xffffff);
        grayBg.draw(camWidth/scaleRatio, 0);
        
        
        if (contourFinder.nBlobs > 0){
            
            // *** draw point and contour on small image ***
            ofSetColor(0, 255, 0);
            ofFill();
            ofCircle(contourFinder.blobs[0].centroid.x, contourFinder.blobs[0].centroid.y, 10);
            contourFinder.blobs[0].draw(0, 0);
            
            // *** draw point on big image ***
            ofSetColor(255, 0, 0);
            ofCircle(fishPosBig.x, fishPosBig.y, 10);
        }
        
        
        // *************** A report ('z' key) ***********
        ofSetHexColor(0xffffff);
        stringstream reportStr;
        reportStr << "bg subtraction and blob detection" << endl
        << "press ' ' to capture bg" << endl
        << "threshold " << threshold << " (press: +/-)" << endl << "num blobs found " << contourFinder.nBlobs << ", fps: " << ofGetFrameRate() <<endl  << "'p': // loads the picture as learning background" << "'y':  threshold ++ " << "'r': threshold --;" << "'o': ROI width  +;" << "'l': ROI width - ; " << endl <<"'i': ROI height +" <<       " 'k': ROI height - " << "'w': ROI y -" << "'s': ROI y + "<< "'a': ROI x -" << " 'd': ROI x + " << "'z': showCalibrationScreen " << "'q':";
        
        ofDrawBitmapString(reportStr.str(), 20, 600);
        ofDrawBitmapString(ofToString(ofGetFrameRate()), 10, 10);
        
    }else{
        
        // ********** draw point on big image *** (comment on real use)
        if (contourFinder.nBlobs > 0){
            ofSetColor(0, 0, 0);
            ofFill();
            //ofCircle(fishPosBig.x,fishPosBig.y, 2);
        }
        
        // ********** draw white frame arround display window ***
        ofSetColor(0, 0, 0);
        ofFill();
        ofRect(0, 0, camWidth, paralax_y);
        ofRect(0, (paralax_y)+ROI.height, camWidth, camHeight);
        ofRect(0, 0, paralax_x, camHeight);
        ofRect((paralax_x)+ROI.width, 0, camWidth, camHeight);
    }
    
    // ************** Draw lines ************************
    ofSetLineWidth(5);
    firstLine->drawLine();
    ofSetLineWidth(1);
    secondLine->drawLine();
    
    // ************** Draw lines_Square ************************
    lineSquare(firstLine->getLinePosition(), secondLine->getLinePosition(), firstLine->getOpacity(), secondLine->getOpacity(), rectOpacity);
    //cout << "draw square: "<< firstLine.getLinePosition() << endl;
    
    
    // ************** Rhythmic Drone ************************
    flickering(0, 0, ofGetWindowWidth(), ofGetWindowHeight(),  flickIntensity, masterBpm);     //The final argument changes the intensity
    
    
    // ************************ DRAW Particles (lines around fish) ******************
    
    
    for(unsigned int i = 0; i < p.size(); i++){
        p[i].draw();
    }
    
    //cordeiro -  not sure if this part belwo is needed!!!
    /*
     ofSetColor(190);
     if( currentMode == PARTICLE_MODE_NEAREST_POINTS ){
     for(unsigned int i = 0; i < attractPoints.size(); i++){
     ofNoFill();
     ofCircle(attractPointsWithMovement[i], 10);
     ofFill();
     ofCircle(attractPointsWithMovement[i], 4);
     }
     }
     */
    
    
    // ***************** MIDI Menu ('m' key) **********************
    if (showMidi) {
        
        ofSetColor(255, 0, 0);
        ofRect(0, 0, ofGetWidth(), 300);
        
        ofSetColor(0);
        
        // draw the last recieved message contents to the screen
        text << "Received: " << ofxMidiMessage::getStatusString(midiMessage.status);
        ofDrawBitmapString(text.str(), 20, 20);
        text.str(""); // clear
        
        text << "channel: " << midiMessage.channel;
        ofDrawBitmapString(text.str(), 20, 34);
        text.str(""); // clear
        
        text << "pitch: " << midiMessage.pitch;
        ofDrawBitmapString(text.str(), 20, 48);
        text.str(""); // clear
        ofRect(20, 58, ofMap(midiMessage.pitch, 0, 127, 0, ofGetWidth()-40), 20);
        
        text << "velocity: " << midiMessage.velocity;
        ofDrawBitmapString(text.str(), 20, 96);
        text.str(""); // clear
        ofRect(20, 105, ofMap(midiMessage.velocity, 0, 127, 0, ofGetWidth()-40), 20);
        
        text << "control: " << midiMessage.control;
        ofDrawBitmapString(text.str(), 20, 144);
        text.str(""); // clear
        ofRect(20, 154, ofMap(midiMessage.control, 0, 127, 0, ofGetWidth()-40), 20);
        
        text << "value: " << midiMessage.value;
        ofDrawBitmapString(text.str(), 20, 192);
        text.str(""); // clear
        if(midiMessage.status == MIDI_PITCH_BEND) {
            ofRect(20, 202, ofMap(midiMessage.value, 0, MIDI_MAX_BEND, 0, ofGetWidth()-40), 20);
        }
        else {
            ofRect(20, 202, ofMap(midiMessage.value, 0, 127, 0, ofGetWidth()-40), 20);
        }
        
        text << "delta: " << midiMessage.deltatime;
        ofDrawBitmapString(text.str(), 20, 240);
        text.str(""); // clear
    }
    
    
    // *********** DRAW Bubbles ***************
    fishBreath.bubblesDraw();
    
    // *********** DRAW Fade SCREEN ***************
    fadeScreen(fadeScreenIntensity);
    
    // *********** DRAW Rain ***************
    myRain.rainDraw();
}




//--------------------------------------------------------------
void ofApp::resetParticles(){
    
    for(unsigned int i = 0; i < p.size(); i++){
        p[i].setMode(currentMode);
        p[i].reset();
    }
}


//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    if( key == 'm' ){
        showMidi = false;
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    // ************** For Particles *****************
    
    if( key == '1'){
        currentMode = PARTICLE_MODE_ATTRACT;
        currentModeStr = "1 - PARTICLE_MODE_ATTRACT: attracts to mouse";
    }
    if( key == '2'){
        currentMode = PARTICLE_MODE_REPEL;
        currentModeStr = "2 - PARTICLE_MODE_REPEL: repels from mouse";
    }
    if( key == '3'){
        currentMode = PARTICLE_MODE_NEAREST_POINTS;
        currentModeStr = "3 - PARTICLE_MODE_NEAREST_POINTS: hold 'f' to disable force";
    }
    if( key == '4'){
        currentMode = PARTICLE_MODE_NOISE;
        currentModeStr = "4 - PARTICLE_MODE_NOISE: snow particle simulation";
        resetParticles();
    }
    if( key == '5'){
        fishBreath.resetParticles();
    }
    if( key == '6' ){
        resetParticles();
    }
    
    
    
    // ************** For MIDI *****************
    if( key == 'm' ){
        showMidi = true;
    }
    
    
    // ************** For fish tracking *****************
    
    switch (key) {
            
        case ' ': // loads the picture as learning background
            bLearnBakground = true;
            break;
            
        case 'p': // loads the picture as learning background
            bLoadPictureBakground = true;
            break;
        case 'y':
            threshold ++;
            if (threshold > 255) threshold = 255;
            break;
        case 'r':
            threshold --;
            if (threshold < 0) threshold = 0;
            break;
        case 'o':
            ROI.width = ROI.width+5;
            break;
        case 'l':
            ROI.width = ROI.width-5;
            break;
        case 'i':
            ROI.height = ROI.height+5;
            break;
        case 'k':
            ROI.height = ROI.height-5;
            break;
        case 'w':
            ROI.y = ROI.y-5;
            break;
        case 's':
            ROI.y = ROI.y+5;
            break;
        case 'a':
            ROI.x = ROI.x-5;
            break;
        case 'd':
            ROI.x = ROI.x+5;
            break;
        case 'z':
            showCalibrationScreen = !showCalibrationScreen;
            break;
        case 'q':
            cout <<"***************" << endl;
            cout << "GRAY BG: " << grayBg.width << " x " << grayBg.height << endl;
            cout << "GRAY IMG: " << grayImage.width << " x " << grayImage.height << endl;
            cout << "COLOR IMG: " << colorImg.width << " x " << colorImg.height << endl;
            cout << "GRAY TEMP: " << grayTempImage.width << " x " << grayTempImage.height << endl;
            cout << "ROI TEMP: " << grayTempImage.getROI().width << " x " << grayTempImage.getROI().height << endl;
            cout << "ROI x, y: " << grayTempImage.getROI().x << " , " << grayTempImage.getROI().y << endl << endl;
            break;
    }
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
    // firstLine.setVelocity(x / 100);
    //secondLine.setVelocity(y / 100);
    
}



//--------------------------------------------------------------
void ofApp::exit() {
    
    // clean up
    midiIn.closePort();
    midiOut.closePort();
    midiIn.removeListener(this);
}


//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {
    
    // make a copy of the latest message
    midiMessage = msg;
    
    cout << "************************" << endl;
    cout << "midiMessage.channel: " << midiMessage.channel << endl;
    cout << "midiMessage.control: " << midiMessage.control << endl;
    cout << "midiMessage.status: " << midiMessage.status << endl;
    cout << "midiMessage.velocity: " << midiMessage.velocity << endl;
    cout << "midiMessage.value: " << midiMessage.value << endl;
    
}
