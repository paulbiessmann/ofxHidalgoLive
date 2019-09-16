#pragma once

#include "ofMain.h"
#include "ofxAudioAnalyzer.h"
#include "ofSoundPlayerExtended.h"
#include "ofxGui.h"
#include "ofxNetwork.h"
#include "ofxPostProcessing.h"
#include "ofxKinect.h"
#include "ofxPostGlitch.h"
#include "ofxMidi.h"

class ofApp : public ofBaseApp, public ofxMidiListener {

	public:
		void setup();
		void update();
		void draw();
        void exit();
    
        void drawAudioStuff();
        void drawKinectStuff();
        void drawPointCloud();
    
        void audioIn(ofSoundBuffer &inBuffer);
        void newMidiMessage(ofxMidiMessage& eventArgs);
    

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

        ofxAudioAnalyzer audioAnalyzer;
        ofSoundPlayerExtended player;

        int sampleRate;
        int bufferSize;

        ofSoundBuffer soundBuffer;

        float rms;
        float rmsR;
        float power;
        float power_R;
        float pitchFreq;
        float pitchFreq_R;

        float pitchFreqNorm;
        float pitchConf;
        float pitchConf_R;
        float pitchSalience;
        float hfc;
        float hfcNorm;
        float specComp;
        float specCompNorm;
        float centroid;
        float centroidNorm;
        float inharmonicity;
        float dissonance;
        float rollOff;
        float rollOffNorm;
        float oddToEven;
        float oddToEvenNorm;
        float strongPeak;
        float strongPeak_R;

        float strongPeakNorm;
        float strongDecay;
        float strongDecayNorm;

        vector<float> spectrum;
        vector<float> melBands;
        vector<float> mfcc;
        vector<float> hpcp;

        vector<float> tristimulus;

        bool isOnset;

        ofxPanel gui;
        ofxFloatSlider smoothing;

        ofSoundStream soundStream;

    
        int fullWidth = 1920;
        int fullHeight = 1080;
    
    
        bool bFxaa = false;
        bool bBloom = false;
        bool bDof = false;
        bool bNoise = false;
        bool bEdge = false;
        bool bTilt = false;
        bool bGod = false;
        bool bRgb = false;
        bool bZoom = false;
        bool bContrast = false;
        bool bSSAO = false;
    
        // udp
        ofxUDPManager udpConnection;
    
    
        /**  Post Effects  **/
    
        ofxPostProcessing post;
        ofEasyCam cam;

        bool bHideCursor = false;
        bool bFullscreen = false;

    
        /* Effects */
        ofShader shader;

        ofFbo fbo;
        ofTexture tex;
        ofPixels pixels;
    
        vector <ofTexture> textures;
        vector <ofColor> colors;
        vector <int> glitchPosX;
        vector <int> glitchPosY;
        vector <int> glitchPosZ;
        vector <float> sizeX;
        vector <float> sizeY;
    
        int glitchSize;
        int glitchNum;
        bool bGlitch = false;
        int growing = 0;
        ofVec2f center;
    
        ofImage img;
    
    
    
        /** Kinect */
        ofxKinect kinect;
    
        int nearThreshold = 0;
        int farThreshold;
        int angle;
        bool bDrawPointCloud = false;
    
        ofFbo screenGrabber;
    
    
        float rand = 0;
    
        /* post glitch */
        ofxPostGlitch    myGlitch;

    
        //MIDI
        stringstream text;
        ofxMidiIn midiIn;
        ofxMidiMessage midiMessage;
    
        float glitchAmount;
        float audioSensitivity;
        float whiteStuff;
    
        int glitchLinePos;
        float glitchLineWidth;
    
        float opacityLive = 0;
        float opacityTemp = 0;
        float opacityMan = 0;
        bool    bDrawKinect = false;
        bool    bDrawAudio  = true;
    
    bool bReactPiano = true;
    bool bReactSinger = false;
    bool bReactWater = false;
    bool bReactAll  = false;
};
