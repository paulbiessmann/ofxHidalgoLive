#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    bHideCursor = true;
    
    //ofBackground(34, 34, 34);
    ofBackground(0);
    ofSetFrameRate(25);
    ofSetBackgroundAuto(true);
    
    ofSetVerticalSync(true);
    ofEnableAlphaBlending();
    
    int sampleRate = 44100;
    bufferSize = 512;
    int outChannels = 0;
    int inChannels = 2;
    
    
    // MIDI
    midiIn.listPorts();
    midiIn.openPort(3); //3
    midiIn.ignoreTypes(false, false, false);
    midiIn.addListener(this);
    midiIn.setVerbose(true);
    
    // setup the sound stream
    soundStream.setup(this, outChannels, inChannels, sampleRate, bufferSize, 3);
    soundStream.printDeviceList();
   // soundStream.setDeviceID(3); //3
    
    //setup ofxAudioAnalyzer with the SAME PARAMETERS
    audioAnalyzer.setup(sampleRate, bufferSize, inChannels);
    
    img.load("test.png");
    img.resize(fullWidth, fullHeight);
   // player.load("TriggerPiano.wav");
    
    gui.setup();
    gui.setPosition(20, 150);
    gui.add(smoothing.setup  ("Smoothing", 0.0, 0.0, 1.0));
    
    
    //create the socket and set to send to 127.0.0.1:11999
    udpConnection.Create();
    udpConnection.Connect("10.101.3.4",3040); //3040
    udpConnection.SetNonBlocking(true);
    
    
    /** setup tex vecs glitch */
    fbo.allocate(fullWidth, fullHeight);
    shader.load("invertShader");
    
    center = ofVec2f(ofGetWidth()/2, ofGetHeight()/2);
    
    glitchNum = 50;
    glitchSize = 30;
    textures.resize(glitchNum);
    
    colors.resize(glitchNum);
    
    glitchPosX.resize(glitchNum);
    glitchPosY.resize(glitchNum);
    glitchPosZ.resize(glitchNum);
    
    for(int i = 0; i < glitchNum; i++) {
        textures[i].allocate(glitchSize,fullHeight, GL_RGB);
        glitchPosX[i]   = ofRandom((int) ofGetWidth());
        glitchPosY[i]   = ofRandom((int) 0);
        glitchPosZ[i]   = ofRandom((int) ofGetHeight());
        colors[i]       = ofColor(ofRandom(240,255), ofRandom(240,255), ofRandom(240,255), ofRandom(255));
    }
    
    screenGrabber.allocate(fullWidth, fullHeight);
    myGlitch.setup(&screenGrabber);

    
    sizeY.resize(glitchNum);
    sizeX.resize(glitchNum);
    growing = ofRandom(50);
    for(int i=0; i<glitchNum; i++){
        sizeX[i]   = ofRandom(10);
        sizeY[i]   = fullHeight;
    }
    
    /** Kinect */
    // enable depth->video image calibration
    kinect.setRegistration(true);
    
    kinect.init();
    //kinect.init(true); // shows infrared instead of RGB video image
    kinect.init(false, false); // disable video image (faster fps)
    
    kinect.open();        // opens first available kinect

    // print the intrinsic IR sensor values
    if(kinect.isConnected()) {
        ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
        ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
        ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
        ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
    }
    kinect.setDepthClipping(40, 5000);
   // void setDepthClipping(float nearClip=500, float farClip=4000);

}

//--------------------------------------------------------------
void ofApp::update(){
    
    //----------------- global MIDI  ------------------
    if(midiMessage.control == 0){ // Nanokontrol fader 1 = cc 0
        audioSensitivity = ofMap(midiMessage.value, 0, 127, 0, 1);
    }
    if(midiMessage.control == 1){ // fader 2
        glitchAmount  = ofMap(midiMessage.value, 0, 127, 1, 0);
    }
    if(midiMessage.control == 2){ // fader 3
        whiteStuff  = ofMap(midiMessage.value, 0, 127, 0, 1);
    }
    if(midiMessage.control == 3){ // fader 4
        glitchLineWidth  = ofMap(midiMessage.value, 0, 127, 0, 1);
    }
    if(midiMessage.control == 7){ // fader 8
        opacityMan  = ofMap(midiMessage.value, 0, 127, 0, 100);
    }
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    //-:Get buffer from sound player:
  //  soundBuffer = player.getCurrentSoundBuffer(bufferSize);
    
    
    //-:ANALYZE SOUNDBUFFER:
//    audioAnalyzer.analyze(soundBuffer);
    
    //-:get Values:
    rms     = audioAnalyzer.getValue(RMS, 0, smoothing);
    rmsR    = audioAnalyzer.getValue(RMS, 1, smoothing);
    power   = audioAnalyzer.getValue(POWER, 0, smoothing);
    pitchFreq = audioAnalyzer.getValue(PITCH_FREQ, 0, smoothing);
    pitchFreq_R = audioAnalyzer.getValue(PITCH_FREQ, 1, smoothing);

    pitchConf = audioAnalyzer.getValue(PITCH_CONFIDENCE, 0, smoothing);
    pitchConf_R = audioAnalyzer.getValue(PITCH_CONFIDENCE, 1, smoothing);

    pitchSalience  = audioAnalyzer.getValue(PITCH_SALIENCE, 0, smoothing);
    inharmonicity   = audioAnalyzer.getValue(INHARMONICITY, 0, smoothing);
    hfc = audioAnalyzer.getValue(HFC, 0, smoothing);
    specComp = audioAnalyzer.getValue(SPECTRAL_COMPLEXITY, 0, smoothing);
    centroid = audioAnalyzer.getValue(CENTROID, 0, smoothing);
    rollOff = audioAnalyzer.getValue(ROLL_OFF, 0, smoothing);
    oddToEven = audioAnalyzer.getValue(ODD_TO_EVEN, 0, smoothing);
    strongPeak = audioAnalyzer.getValue(STRONG_PEAK, 0, smoothing);
    strongPeak_R = audioAnalyzer.getValue(STRONG_PEAK, 1, smoothing);

    strongDecay = audioAnalyzer.getValue(STRONG_DECAY, 0, smoothing);
    //Normalized values for graphic meters:
    pitchFreqNorm   = audioAnalyzer.getValue(PITCH_FREQ, 0, smoothing, TRUE);
    hfcNorm     = audioAnalyzer.getValue(HFC, 0, smoothing, TRUE);
    specCompNorm = audioAnalyzer.getValue(SPECTRAL_COMPLEXITY, 0, smoothing, TRUE);
    centroidNorm = audioAnalyzer.getValue(CENTROID, 0, smoothing, TRUE);
    rollOffNorm  = audioAnalyzer.getValue(ROLL_OFF, 0, smoothing, TRUE);
    oddToEvenNorm   = audioAnalyzer.getValue(ODD_TO_EVEN, 0, smoothing, TRUE);
    strongPeakNorm  = audioAnalyzer.getValue(STRONG_PEAK, 0, smoothing, TRUE);
    strongDecayNorm = audioAnalyzer.getValue(STRONG_DECAY, 0, smoothing, TRUE);
    
    dissonance = audioAnalyzer.getValue(DISSONANCE, 0, smoothing);
    
    spectrum = audioAnalyzer.getValues(SPECTRUM, 0, smoothing);
    melBands = audioAnalyzer.getValues(MEL_BANDS, 0, smoothing);
    mfcc = audioAnalyzer.getValues(MFCC, 0, smoothing);
    hpcp = audioAnalyzer.getValues(HPCP, 0, smoothing);
    
    tristimulus = audioAnalyzer.getValues(TRISTIMULUS, 0, smoothing);
    
    isOnset = audioAnalyzer.getOnsetValue(0);
  
    
    /** Kinect */
    kinect.update();
   
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    
    /** Post **/
    screenGrabber.begin();

    //img.draw(0,0);
    ofSetColor(255);
    
    
    if(bDrawKinect){
        drawKinectStuff();
    }
    else if(bDrawAudio){
        ofClear(0,0,0,255);
        drawAudioStuff();
    }
    

    
    screenGrabber.end();
    
    myGlitch.generateFx();
    screenGrabber.draw(0,0);
    

    
    
//   // growing++;
//    int size = textures.size() - ofRandom(textures.size()/4);
//    for (int i=0; i< glitchNum; i++){
//
//        float dirX =  (glitchPosY[i] - center.y)*i * growing * 100;
//        float dirY =  (glitchPosY[i] - center.y) * growing ;//* 0.01;
//
//        ofSetColor(colors[i]);
//       // textures[i].loadScreenData(glitchPosX[i], glitchPosY[i], sizeX[i], sizeX[i]);
//        shader.begin();
//      //  textures[i].draw(glitchPosX[i] + 50, glitchPosY[i] ,  sizeX[i], sizeY[i]);
//
//        int rTime = ofRandom(5);
//        if (rTime > 4){
//            rand = ofRandom(i);
//            ofMap(rand, 0, 100, 0, 1);
//        }
//        shader.setUniform1f("rand", rand);
//
//
//
//        tex.drawSubsection(glitchPosX[i], glitchPosY[i], sizeX[i], sizeY[i], glitchPosX[i] + rand*20, glitchPosY[i]+rand*10, sizeX[i]+ 100, sizeY[i]-rand*20);
//
//        //screenGrabber.getTexture().drawSubsection(glitchPosX[i], glitchPosY[i], sizeX[i], sizeY[i], glitchPosX[i] + dirX, glitchPosY[i], sizeX[i], sizeY[i]);
//        shader.end();
//
//
//    }
    ofSetColor(255);

    if(1){
        if(pitchFreq > 800 && pitchFreq < 850 && pitchConf > glitchAmount ){ //0.4
            myGlitch.setFx(OFXPOSTGLITCH_CONVERGENCE    , true);
        }else{
            myGlitch.setFx(OFXPOSTGLITCH_CONVERGENCE    , false);
        }
        if(pitchFreq > 400 && pitchFreq < 450 && pitchConf > glitchAmount  ){
            myGlitch.setFx(OFXPOSTGLITCH_SHAKER          , true);
        }else{
            myGlitch.setFx(OFXPOSTGLITCH_SHAKER          , false);
        }
        if(pitchFreq > 300 && pitchFreq < 350 && pitchConf > glitchAmount ){
            myGlitch.setFx(OFXPOSTGLITCH_CUTSLIDER        , true);
        }else{
            myGlitch.setFx(OFXPOSTGLITCH_CUTSLIDER        , false);
        }
        if(pitchFreq > 200 && pitchFreq < 250 && pitchConf > glitchAmount  ){
            myGlitch.setFx(OFXPOSTGLITCH_TWIST            , true);
        }
        else{
            myGlitch.setFx(OFXPOSTGLITCH_TWIST            , false);
        }
        if(pitchFreq > 100 && pitchFreq < 150 && pitchConf > glitchAmount){
            myGlitch.setFx(OFXPOSTGLITCH_SLITSCAN        , true);
        }else{
            myGlitch.setFx(OFXPOSTGLITCH_SLITSCAN        , false);

        }
        if(pitchFreq > 1000 && pitchFreq < 1050&& pitchConf > glitchAmount || strongPeak > 1.5 - glitchAmount  ){ //0.7
            myGlitch.setFx(OFXPOSTGLITCH_CR_BLUERAISE    , true);
        }else{
            myGlitch.setFx(OFXPOSTGLITCH_CR_BLUERAISE    , false);
        }
    }
    
    
    // PB Tests
    // hirn wlan
    if(bReactSinger && (pitchFreq_R > 310 && pitchFreq_R < 330 && pitchConf_R > 0.4)){
        //ofDrawRectangle(50, 500, 10*rms, 100);
        opacityLive = ofMap(rmsR, 0, 1, 0, 100);
        opacityLive *= audioSensitivity;
        //cout << "opacityLive 1 " << opacityLive << "\n";
        
    }
    
    //piano
    //if(strongPeak > 0.7){
    else if(bReactPiano && ((pitchFreq > 1000 && pitchConf > 0.4) || strongPeak > 0.7)){
        //ofSetColor(0,255,0);
        //ofDrawRectangle(200, 500, 10*rms, 100);
        
        opacityLive = ofMap(rms, 0, 1, 0, 100);
        opacityLive *= audioSensitivity;
        //cout << "opacityLive 2 " << opacityLive << "\n";

    }
    /** Wasser Netz Oszillation */
    else if(bReactWater && ((pitchFreq > 700 && pitchConf > 0.4) || strongPeak > 0.7)){
        
        opacityLive = ofMap(rms, 0, 1, 0, 100);
        opacityLive *= audioSensitivity;
        
    }
    
    else {
        opacityLive = 0;
    }
    
    if(opacityMan > 0.01){
        opacityLive = opacityMan;
    }
    
    if(ofGetFrameNum() % 3 == 0){
        cout << "opacityLive " << opacityLive << "\n";
        string message="setInput \"opacityLive\" " + ofToString(opacityLive);
        udpConnection.Send(message.c_str(),message.length());
    }
    
}
//--------------------------------------------------------------
void ofApp::drawKinectStuff(){
    
    if(bDrawPointCloud){
        drawPointCloud();
    }
    else{
        kinect.drawDepth(0, 0, ofGetWidth(), ofGetHeight());
    }

    
    if(whiteStuff > 0.001){
        int stepsize = 8;
        for(int i=0; i<fullWidth; i+=stepsize){
            for(int j=0; j<fullHeight; j+=stepsize/2){
                ofColor col = ofColor(255, ofRandom(255) * whiteStuff + 255*whiteStuff);
                ofSetColor(col);
                ofDrawRectangle(i, j,  stepsize/2, stepsize);
            }
        }
    }
    
    if(1){
        float glitchOpac = ofMap(glitchAmount, 1, 0, 0, 255);
        if(glitchLineWidth > 0.999999){
            ofSetColor(255, 255 * glitchOpac);
            ofDrawRectangle(0, 0, fullWidth, fullHeight);
        }
        else{
            for(int i=0; i<20; i++){
                glitchLinePos = abs(sin(ofGetElapsedTimef() * i * 0.001 * glitchOpac)) * sin(i) * fullHeight;
               //glitchLinePos = abs(sin(ofGetElapsedTimef() * i * 0.00001 * glitchOpac)) * fullHeight;
                //glitchLinePos += i*glitchAmount * 0.01 + 1;
                
                if(glitchLinePos > fullHeight){glitchLinePos = 0;}
                ofSetColor(255, glitchOpac + ofRandom(50));
                ofDrawRectangle(0, glitchLinePos + ofNoise((100*i*ofGetElapsedTimef())), fullWidth, 200*glitchLineWidth + ofRandom(20)*glitchLineWidth);
            }
        }
    }
    
    ofSetColor(255);


//    for (int i=0; i<size; i++){
//
//        float dirX =  (glitchPosX[i] - center.x) * growing ;//* 0.01;
//        float dirY =  (glitchPosY[i] - center.y) * growing ;//* 0.01;
//        textures[i].loadScreenData(glitchPosX[i], glitchPosY[i],sizeX[i], sizeX[i]);
//
//        shader.begin();
//       // ofSetColor(colors[i] /4 );
//        textures[i].draw(glitchPosX[i] + dirX, glitchPosY[i] +  dirY,glitchPosZ[i] * growing * 0.01,  sizeX[i], sizeY[i]);
//        shader.end();
//    }
}

//**********************************************************
void ofApp::drawPointCloud() {
    int w = fullWidth;
    int h = fullHeight;
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_POINTS);
    int step = 3;
    
    
    for(int y = 0; y < h; y += step) {
        for(int x = 0; x < w; x += step) {
            //            ofLogNotice() << "kinect get distance: " << kinect.getDistanceAt(x, y) << endl;
            
            if(kinect.getDistanceAt(x, y) > (255-nearThreshold) ){//&& kinect.getDistanceAt(x, y) > farThreshold) {
                //                mesh.addColor(kinect.getColorAt(x,y));
                //                mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
                mesh.addColor(255);
                mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
            }
            
        }
    }
    
    
    glPointSize(5);
    ofPushMatrix();
    // the projected points are 'upside down' and 'backwards'
    //ofScale(1, -1, -1);
    ofTranslate(0, 0, -1000); // center the points a bit
    ofEnableDepthTest();
    mesh.drawVertices();
    ofDisableDepthTest();
    ofPopMatrix();
}
//--------------------------------------------------------------
void ofApp::drawAudioStuff(){
    //-Single value Algorithms:
    
    ofPushMatrix();
    ofTranslate(350, 0);
    int mw = 250;
    int xpos = 0;
    int ypos = 30;
    
    float value, valueNorm;
    
    ofSetColor(255);
    value = rms;
    string strValue = "RMS: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = rmsR;
    strValue = "RMS_R: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    
    ypos += 50;
    ofSetColor(255);
    value = power;
    strValue = "Power: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = pitchFreq;
    valueNorm = pitchFreqNorm;
    strValue = "Pitch Frequency: " + ofToString(value, 2) + " hz.";
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = pitchConf;
    strValue = "Pitch Confidence: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = pitchSalience;
    strValue = "Pitch Salience: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = inharmonicity;
    strValue = "Inharmonicity: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = hfc;
    valueNorm = hfcNorm;
    strValue = "HFC: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = specComp;
    valueNorm = specCompNorm;
    strValue = "Spectral Complexity: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = centroid;
    valueNorm = centroidNorm;
    strValue = "Centroid: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = dissonance;
    strValue = "Dissonance: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = rollOff;
    valueNorm = rollOffNorm;
    strValue = "Roll Off: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw , 10);
    
    ypos += 50;
    ofSetColor(255);
    value = oddToEven;
    valueNorm = oddToEvenNorm;
    strValue = "Odd To Even Harmonic Energy Ratio: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = strongPeak;
    valueNorm = strongPeakNorm;
    strValue = "Strong Peak: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = strongDecay;
    valueNorm = strongDecayNorm;
    strValue = "Strong Decay: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = isOnset;
    strValue = "Onsets: " + ofToString(value);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ofPopMatrix();
    
    //-Vector Values Algorithms:
    
    ofPushMatrix();
    
    ofTranslate(700, 0);
    
    int graphH = 75;
    int yoffset = graphH + 50;
    ypos = 30;
    
    ofSetColor(255);
    ofDrawBitmapString("Spectrum: ", 0, ypos);
    ofPushMatrix();
    ofTranslate(0, ypos);
    ofSetColor(ofColor::cyan);
    float bin_w = (float) mw / spectrum.size();
    for (int i = 0; i < spectrum.size(); i++){
        float scaledValue = ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    ofPopMatrix();
    
    ypos += yoffset;
    ofSetColor(255);
    ofDrawBitmapString("Mel Bands: ", 0, ypos);
    ofPushMatrix();
    ofTranslate(0, ypos);
    ofSetColor(ofColor::cyan);
    bin_w = (float) mw / melBands.size();
    for (int i = 0; i < melBands.size(); i++){
        float scaledValue = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    ofPopMatrix();
    
    ypos += yoffset;
    ofSetColor(255);
    ofDrawBitmapString("MFCC: ", 0, ypos);
    ofPushMatrix();
    ofTranslate(0, ypos);
    ofSetColor(ofColor::cyan);
    bin_w = (float) mw / mfcc.size();
    for (int i = 0; i < mfcc.size(); i++){
        float scaledValue = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);//clamped value
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    ofPopMatrix();
    
    ypos += yoffset;
    ofSetColor(255);
    ofDrawBitmapString("HPCP: ", 0, ypos);
    ofPushMatrix();
    ofTranslate(0, ypos);
    ofSetColor(ofColor::cyan);
    bin_w = (float) mw / hpcp.size();
    for (int i = 0; i < hpcp.size(); i++){
        //float scaledValue = ofMap(hpcp[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float scaledValue = hpcp[i];
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    ofPopMatrix();
    
    ypos += yoffset;
    ofSetColor(255);
    ofDrawBitmapString("Tristimulus: ", 0, ypos);
    ofPushMatrix();
    ofTranslate(0, ypos);
    ofSetColor(ofColor::cyan);
    bin_w = (float) mw / tristimulus.size();
    for (int i = 0; i < tristimulus.size(); i++){
        //float scaledValue = ofMap(hpcp[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float scaledValue = tristimulus[i];
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    ofPopMatrix();
    
    
    ofPopMatrix();
    
    //-Gui & info:
    
    gui.draw();
//    ofSetColor(255);
//    ofDrawBitmapString("ofxAudioAnalyzer\n\nALL ALGORITHMS EXAMPLE", 10, 32);
//    ofSetColor(ofColor::hotPink);
//    ofDrawBitmapString("Keys 1-6: Play audio tracks", 10, 100);
    
}


//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer &inBuffer){
    //ANALYZE SOUNDBUFFER:
 audioAnalyzer.analyze(inBuffer);
}
//--------------------------------------------------------------
void ofApp::exit(){
    ofSoundStreamStop();
    audioAnalyzer.exit();
    player.stop();
    kinect.setCameraTiltAngle(0); // zero the tilt on exit
    kinect.close();
    midiIn.closePort();
    midiIn.removeListener(this);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    player.stop();
    switch (key) {
       
        case 'a':
            player.load("HidalgoTest.wav");
            player.play();

            break;
        case 's':
            player.load("TriggerPiano.wav");
            player.play();

            break;
       
            

            
        case'p':
            bDrawPointCloud = !bDrawPointCloud;
            break;
            
            
        case 'h':
            bHideCursor = !bHideCursor;
            if (bHideCursor) ofHideCursor();
            else ofShowCursor();
            break;
            
        case 'f':
            bFullscreen = !bFullscreen;
            ofSetFullscreen(bFullscreen);
            break;
            
        case OF_KEY_UP:
            angle++;
            if(angle>30) angle=30;
            kinect.setCameraTiltAngle(angle);
            break;
            
        case OF_KEY_DOWN:
            angle--;
            if(angle<-30) angle=-30;
            kinect.setCameraTiltAngle(angle);
            break;
            
        case '+':
        case '=':
            nearThreshold ++;
            if (nearThreshold > 255) nearThreshold = 255;
            
            break;
            
        case '-':
            nearThreshold --;
            if (nearThreshold < 0) nearThreshold = 0;
            break;
            
        case 'y':
            kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
            break;
        default:
            break;
    }
    
    if (key == '1') myGlitch.setFx(OFXPOSTGLITCH_CONVERGENCE    , true);
    if (key == '2') myGlitch.setFx(OFXPOSTGLITCH_GLOW            , true);
    if (key == '3') myGlitch.setFx(OFXPOSTGLITCH_SHAKER            , true);
    if (key == '4') myGlitch.setFx(OFXPOSTGLITCH_CUTSLIDER        , true);
    if (key == '5') myGlitch.setFx(OFXPOSTGLITCH_TWIST            , true);
    if (key == '6') myGlitch.setFx(OFXPOSTGLITCH_OUTLINE        , true);
    if (key == '7') myGlitch.setFx(OFXPOSTGLITCH_NOISE            , true);
    if (key == '8') myGlitch.setFx(OFXPOSTGLITCH_SLITSCAN        , true);
    if (key == '9') myGlitch.setFx(OFXPOSTGLITCH_SWELL            , true);
    if (key == '0') myGlitch.setFx(OFXPOSTGLITCH_INVERT            , true);
    
    if (key == 'q') myGlitch.setFx(OFXPOSTGLITCH_CR_HIGHCONTRAST, true);
    if (key == 'w') myGlitch.setFx(OFXPOSTGLITCH_CR_BLUERAISE    , true);
    if (key == 'e') myGlitch.setFx(OFXPOSTGLITCH_CR_REDRAISE    , true);
    if (key == 'r') myGlitch.setFx(OFXPOSTGLITCH_CR_GREENRAISE    , true);
    if (key == 't') myGlitch.setFx(OFXPOSTGLITCH_CR_BLUEINVERT    , true);
    if (key == 'z') myGlitch.setFx(OFXPOSTGLITCH_CR_REDINVERT    , true);
    if (key == 'u') myGlitch.setFx(OFXPOSTGLITCH_CR_GREENINVERT    , true);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if(key == 'g'){
        
        growing = ofRandom(10);
        glitchNum = ofRandom(20);
        for(int i=0; i<glitchNum; i++){
            
            sizeX[i]   = ofRandom(50);
            sizeY[i]   = fullHeight;

        }
    }
    
    if (key == '1') myGlitch.setFx(OFXPOSTGLITCH_CONVERGENCE    , false);
    if (key == '2') myGlitch.setFx(OFXPOSTGLITCH_GLOW            , false);
    if (key == '3') myGlitch.setFx(OFXPOSTGLITCH_SHAKER            , false);
    if (key == '4') myGlitch.setFx(OFXPOSTGLITCH_CUTSLIDER        , false);
    if (key == '5') myGlitch.setFx(OFXPOSTGLITCH_TWIST            , false);
    if (key == '6') myGlitch.setFx(OFXPOSTGLITCH_OUTLINE        , false);
    if (key == '7') myGlitch.setFx(OFXPOSTGLITCH_NOISE            , false);
    if (key == '8') myGlitch.setFx(OFXPOSTGLITCH_SLITSCAN        , false);
    if (key == '9') myGlitch.setFx(OFXPOSTGLITCH_SWELL            , false);
    if (key == '0') myGlitch.setFx(OFXPOSTGLITCH_INVERT            , false);
    
    if (key == 'q') myGlitch.setFx(OFXPOSTGLITCH_CR_HIGHCONTRAST, false);
    if (key == 'w') myGlitch.setFx(OFXPOSTGLITCH_CR_BLUERAISE    , false);
    if (key == 'e') myGlitch.setFx(OFXPOSTGLITCH_CR_REDRAISE    , false);
    if (key == 'r') myGlitch.setFx(OFXPOSTGLITCH_CR_GREENRAISE    , false);
    if (key == 't') myGlitch.setFx(OFXPOSTGLITCH_CR_BLUEINVERT    , false);
    if (key == 'z') myGlitch.setFx(OFXPOSTGLITCH_CR_REDINVERT    , false);
    if (key == 'u') myGlitch.setFx(OFXPOSTGLITCH_CR_GREENINVERT    , false);
    
    switch(key){
        case 'x':
            bReactWater = false;
            bReactPiano = true;
            bReactSinger = false;
            bDrawKinect = true;
            bDrawAudio = false;
            break;
        case 'c':
            bReactWater = false;
            bReactPiano = true;
            bReactSinger = true;
            bDrawKinect = true;
            bDrawAudio = false;
            break;
        case 'v':
            bReactWater = false;
            bReactPiano = true;
            bReactSinger = true;
            bDrawKinect = false;
            bDrawAudio = true;
            break;
        case 'n':
            bReactWater = true;
            bReactPiano = true;
            bReactSinger = true;
            bDrawKinect = false;
            bDrawAudio = true;
            break;
        case 'm':
            bReactWater = true;
            bReactPiano = true;
            bReactSinger = true;
            bDrawKinect = true;
            bDrawAudio = false;
            break;
    }
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

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {
    
    // make a copy of the latest message
    midiMessage = msg;
}

