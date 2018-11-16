#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "gui/imagemanager.h"
#include "gui/segnetcolourinspector.h"
#include "gui/audiowaveform.h"
#include "gui/pointcloudrenderer.h"
#include "gui/elements/uielements.h"
#include "util/databaseloader.h"
#include "util/featurecontrol.h"

class ofApp : public ofBaseApp{
public:
    //Init functions
    void initAudio();
    void initNames();
    void setLayout();

    //Draw/update functions
    void drawColors();
    void drawControls();
    void drawControlsText();
    void drawDebug();

    //Coms
    void updateOSC();
    void updatePlayingVideo(string video);
    void handlePythonMessages();


    void setSpeed(int value);
    void toggleFeatureTarget(int index);

    float getColorValue(string id);
    void handleKnobInput(ofxOscMessage m);
    void handleButtonInput(int index);
    void updateActiveFeature(int index);

    bool vectorsAreEqual(vector<string>v1, vector<string> v2);
    void toggleLanguage();
    void playRandomVideo();

    //Default functions
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
    void audioIn(ofSoundBuffer &b);

    void logFilename();
    void receiveBuffer(ofSoundBuffer & buffer);
    AudioWaveform * getWaveform(){
        return &waveform;
    }

    //GUI elements
    ImageManager imageManager;
    SegnetColourInspector colourInspector;
    AudioWaveform waveform;
    DatabaseLoader databaseLoader;
    CommunicationManager coms;
    PointCloudRenderer pointCloudRender;
    FeatureControl* fc;

    vector<unique_ptr<CircleFeatureGuiElement>> featureGuiElements;

    ofTrueTypeFont font;
    ofSoundStream soundStream;

    int rectangleWidth;
    int rectangleMaxHeight;
    int rectangleOffset;
    int spaceRemainder;
    int rectangleTop;
    int minHeight;
    int numKnobs;

    int incomingPlayerMessageCounter;
    int incomingControllerMessageCounter;

    int playingFileDuration =1000;
    vector<int> videoIndexes;
    vector<string> featureNamesEn;
    vector<string> featureNamesFr;
    vector<string> featureNames;
    string durationName, neighbourName;
    string durationNameFr, neighbourNameFr;
    string durationNameEn, neighbourNameEn;


    bool audioToggle;
    bool speedChanged = false;
    bool DEV_MODE;
    bool languageIsEnglish;

    uint64_t log_timer;

    int speedSetting;
    string currentPlayingVideo;
    vector<string> lastVideos;
    vector<ofRectangle> clickRectangles;
    int clickIndex;
    int clickY =0;

    ofFbo drawFBO;
    ofFbo drawFBO2;

    ofFile filenameLog;

//    ofShader FXAAshader;



};
