#include "ofApp.h"
#include "ofxJsonSettings.h"



//--------------------------------------------------------------
void ofApp::setup(){
    ofGetWindowPtr()->setVerticalSync(true);
    ofSetVerticalSync(true);
//    ofSetFrameRate(60);

    ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL);
    ofEnableSmoothing();
    ofEnableAlphaBlending();
    ofSetCircleResolution(200);

    Settings::get().load("settings.json");
    string db_path = Settings::getString("db_path");
    DEV_MODE = Settings::getBool("dev_mode");

    font.load("futura.ttf", 12);
//    initAudio();
    initNames();
    //    ofSetLogLevel(OF_LOG_ERROR);
    ofSetLogLevel(OF_LOG_NOTICE);

    numKnobs =23;
    log_timer =ofGetElapsedTimeMillis();
    databaseLoader.loadHDF5Data(db_path);

    if (Settings::getInt("point_mode") ==0){
        pointCloudRender.setRotation(false);
        pointCloudRender.initPoints(databaseLoader.dimension_reduction_2D, databaseLoader.colors);
    }

    else if (Settings::getInt("point_mode") ==1){
        pointCloudRender.setRotation(true);
        pointCloudRender.initPoints(databaseLoader.dimension_reduction, databaseLoader.colors);
    }

    //Initialize feature gui elements
    int num_features =21;
    featureGuiElements.resize(num_features+2);
    featureGuiElements[0] = make_unique<DurationElement>();
    featureGuiElements[1] = make_unique<SearchRadiusElement>();

    for (int i =2; i<featureGuiElements.size(); i++){
        featureGuiElements[i]= make_unique<CircleFeatureGuiElement>();
    }

    //Init custom gui elements
    //Tilt
    featureGuiElements[10]= make_unique<TiltElement>();

    //Hue
    featureGuiElements[11] = make_unique<ColorCircle>();
//    dynamic_cast<ColorCircle*>(featureGuiElements[11].get())->setColorLimits(databaseLoader.color_min_max.first, databaseLoader.color_min_max.second);

    //Set the colours on the segnet elements
    int segnetStart =14;
    int segnetDoubleStart =20;
    int i =0;
    for (string name : segnetSingleFeatures){
        featureGuiElements[i +segnetStart]->setWeightColor(imageManager.getSegnetColor(name));
        i++;
    }
    i=0;
    for (string name : segnetPairedFeatures){
        pair<ofColor, ofColor> c = imageManager.getSegnetColorPair(name);
        featureGuiElements[i +segnetDoubleStart]->setWeightColors(c.first, c.second);
        i++;
    }

    speedSetting =0;
    currentPlayingVideo ="";

    //Initialize lights
    for(int i =0; i < num_features;i++){
        coms.sendLightControl(i+3, 0);
    }

    ofBackground(0);
    setLayout();

    ofFboSettings s;
    s.width=ofGetWidth();
    s.height=ofGetHeight();
//    s.useDepth = true;

    s.numSamples =0;
    drawFBO.allocate(s);

    s.numSamples =0;
    drawFBO2.allocate(s);

//    FXAAshader.load("fxaa.vert","fxaa.frag");

    fc =new FeatureControl(&databaseLoader, &coms, &featureGuiElements, &pointCloudRender);
}

void ofApp::setSamplePlayer(SimpleSamplePlayer* s){
    samplePlayer = s;
}

void ofApp::initNames(){
    featureNamesEn.push_back("Loudness");
    featureNamesEn.push_back("Pitch");
    featureNamesEn.push_back("Percussiveness");
    featureNamesEn.push_back("Spectral Bandwidth");
    featureNamesEn.push_back("Vehicle speed");
    featureNamesEn.push_back("Instability");
    featureNamesEn.push_back("Engine RPM");
    featureNamesEn.push_back("Temperature");
    featureNamesEn.push_back("Roll");
    featureNamesEn.push_back("Hue");
    featureNamesEn.push_back("Lightness");
    featureNamesEn.push_back("Uncertainty");
    featureNamesEn.push_back("Building");
    featureNamesEn.push_back("Pavement");
    featureNamesEn.push_back("Road");
    featureNamesEn.push_back("Sky");
    featureNamesEn.push_back("Trees");
    featureNamesEn.push_back("Vehicle");
    featureNamesEn.push_back("Signage");
    featureNamesEn.push_back("Fence/Pole");
    featureNamesEn.push_back("Bike/Pedestrian");

    featureNamesFr.push_back("Volume");
    featureNamesFr.push_back("Hauteur spectrale");
    featureNamesFr.push_back("Percussion");
    featureNamesFr.push_back("Largeur de bande");
    featureNamesFr.push_back("Vitesse");
    featureNamesFr.push_back("Instabilité");
    featureNamesFr.push_back("Régime Moteur");
    featureNamesFr.push_back("Température");
    featureNamesFr.push_back("Inclinaison");
    featureNamesFr.push_back("Teinte");
    featureNamesFr.push_back("Luminosité");
    featureNamesFr.push_back("Incertitude");
    featureNamesFr.push_back("Bâtiment");
    featureNamesFr.push_back("Trottoir");
    featureNamesFr.push_back("Route");
    featureNamesFr.push_back("Ciel");
    featureNamesFr.push_back("Arbre");
    featureNamesFr.push_back("Véhicule");
    featureNamesFr.push_back("Signalisation");
    featureNamesFr.push_back("Clôture/Poteau");
    featureNamesFr.push_back("Piéton/Vélo");
    durationNameEn= "Duration";
    neighbourNameEn= "Quantity";
    durationNameFr= "Durée";
    neighbourNameFr = "Quantité";
    languageIsEnglish = Settings::getBool("english");
    if (languageIsEnglish) {
        featureNames = featureNamesEn;
        durationName = durationNameEn;
        neighbourName = neighbourNameEn;
    }

    else {
        featureNames = featureNamesFr;
        durationName = durationNameFr;
        neighbourName = neighbourNameFr;
    }
}

void ofApp::initAudio(){
    audioToggle = true;
    soundStream.printDeviceList();
    std::vector<ofSoundDevice> devices = soundStream.getDeviceList();
    ofSoundStreamSettings s;
    int device_id = Settings::getInt("audio_device_id");
//    soundStream.setDeviceID(device_id);
//    s.numBuffers=4;
//    s.sampleRate=44100;
//    s.bufferSize=IN_AUDIO_BUFFER_LENGTH;
//    soundStream.setup(s);
    soundStream.setup(this, 0, 2, 44100, IN_AUDIO_BUFFER_LENGTH, 4);
    ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Interface audio init"<<endl;
}

void ofApp::setLayout(){
    int windowWidth =ofGetWidth();
    int windowHeight=ofGetHeight();

    //Set widget layouts
    float div = 14./23.;
    int leftWidth = int(floor(div*windowWidth));
    int rightWidth = windowWidth -int(floor(div*windowWidth));

    //Place knobs
    float knobWidth = windowWidth/float(numKnobs);
    int offset = knobWidth;

    int remainder = windowWidth - knobWidth*23;
    imageManager.setLayout(leftWidth -1, 0, rightWidth , 3*windowHeight/4);
    pointCloudRender.setLayout(0, windowHeight/4, leftWidth, 2* windowHeight/4);
    waveform.setLayout(0,0, leftWidth, windowHeight/4 );

    float max_row_1_height  =0;
    float max_row_2_height = 0;
    for (int i = 0; i < featureNames.size(); i++){
        if (i%2 ==0){
            max_row_1_height = max(font.stringHeight(featureNames[i]), max_row_1_height);
        }
        else{
            max_row_2_height = max(font.stringHeight(featureNames[i]), max_row_2_height);
        }
    }

    int controlSectionHeight = windowHeight/4 -5;
    int textHeight = max_row_1_height + max_row_2_height;
    int knobSectionHeight = controlSectionHeight;
    int elementHeight = knobSectionHeight/2;
    float elementWidth = knobWidth;
    int firstRowOffset =max_row_1_height;
    int secondRowOffset =elementHeight ;

    //Initialize special knobs
    featureGuiElements[0]->setName(durationName);
    featureGuiElements[1]->setName(neighbourName);
    clickRectangles.clear();
    float x, y;
    x =0;
    for (int i=0; i<featureGuiElements.size(); i++){
        y = windowHeight - controlSectionHeight;

        if (i%2 ==1){
            y +=secondRowOffset;
        }
        if (i ==0){
            featureGuiElements[i]->setPosition(x+6,y);
        }
        else if (i ==featureGuiElements.size()-1){
            featureGuiElements[i]->setPosition(x-6,y);
        }
        else{
            featureGuiElements[i]->setPosition(x,y);
        }

        featureGuiElements[i]->setSize(elementWidth, elementHeight);
        clickRectangles.push_back(ofRectangle(x,y,elementWidth, elementHeight));

        featureGuiElements[i]->increaseCircleOffset(15);

        if (i>=2){
            featureGuiElements[i]->setName(featureNames[i-2]);
        }
        x+=knobWidth;
    }
    featureGuiElements[11]->increaseCircleOffset(-5);


    ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Offset "<<rectangleOffset;
    ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Remaining space "<< spaceRemainder;
}

//--------------------------------------------------------------
void ofApp::update(){
    updateOSC();
    if (DEV_MODE){
        handlePythonMessages();
    }

    //Log incoming and outgoing messages
    uint64_t currentTime = ofGetElapsedTimeMillis();
    if ((currentTime - log_timer)>1000)
    {
        ofLogNotice()<<"Received " <<incomingControllerMessageCounter<< " OSC messages from controller in the last second"<<endl;
        ofLogNotice()<<"Received " <<incomingPlayerMessageCounter<< " OSC messages from player in the last second"<<endl;
        coms.logMessageCount();
        incomingControllerMessageCounter =0;
        incomingPlayerMessageCounter =0;
        log_timer = currentTime;
    }

    //Update active features
    fc->update();

    videoIndexes =fc->getVideoIndexes();
    pointCloudRender.setActiveNodes(videoIndexes);
    pointCloudRender.update();

    pair<string,int> playingVideo = fc->getPlayingVideo();
    imageManager.loadImages(playingVideo.first);
    pointCloudRender.setPlayingNode(playingVideo.second);

    //Update gui elements

    if(ofGetFrameNum()%2==0 ){
        waveform.updateSoundBuffer(samplePlayer);
    }
    waveform.update();
    featureGuiElements[0]->update(); //Update speed timer
    featureGuiElements[1]->update(); //Update search radius

    fc->updateFeatureElements();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    ofSetColor(255);

    drawFBO.begin();
    ofBackground(0);
    ofSetColor(255);
    pointCloudRender.draw();
    drawControls();

    waveform.draw();
    drawFBO.end();

    drawFBO2.begin();
    ofBackground(0);
    ofSetColor(255);

//    FXAAshader.begin();
    drawFBO.draw(0,0,ofGetWidth(), ofGetHeight());
//    FXAAshader.end();

//        glEnable(GL_POINT_SMOOTH);
//        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
//        glEnable(GL_BLEND);
//        glEnable(GL_POINT_SPRITE);
//        glEnable(GL_LINE_SMOOTH);
//        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    imageManager.draw();
    drawControlsText();

    if (DEV_MODE){
        std::stringstream strm;
        strm << "fps: " << ofGetFrameRate();
        ofDrawBitmapString(strm.str(),20, 20);

        drawDebug();
    }

    drawFBO2.end();
    drawFBO2.draw(0,0,ofGetWidth(), ofGetHeight());
}

void ofApp::drawDebug(){
    ofPushMatrix();
    ofTranslate(ofGetWidth()/3*2 -70,20);
    fc->draw();
    ofPopMatrix();

}

//Draw control gui elements
void ofApp::drawControls(){

    //Draw feature controls
    for (int i=0; i< featureGuiElements.size(); i++){
        featureGuiElements[i]->draw();
    }
}
void ofApp::drawControlsText(){

     //Draw feature controls
     for (int i=0; i< featureGuiElements.size(); i++){
         featureGuiElements[i]->drawText();
     }
 }
void ofApp::drawColors(){

    ofColor c = databaseLoader.colors[databaseLoader.getVideoIndexFromName(currentPlayingVideo)];

    int windowWidth = ofGetWidth();
    ofPushMatrix();
    ofTranslate(5*windowWidth/6, 20);
    int boxWidth = windowWidth/6;


    ofSetColor(c);
    ofDrawRectangle(0,0, boxWidth/2, boxWidth/2);
    ofSetColor(255);
    font.drawString("Palette",0,20);

    ofPopMatrix();
}

void ofApp::toggleLanguage(){
    languageIsEnglish = !languageIsEnglish;
    if (languageIsEnglish) {
        featureNames = featureNamesEn;
        durationName = durationNameEn;
        neighbourName = neighbourNameEn;
    }

    else {
        featureNames = featureNamesFr;
        durationName = durationNameFr;
        neighbourName = neighbourNameFr;
    }
    setLayout();
}
void ofApp::playRandomVideo(){
    fc->playRandomVideo();
}
void ofApp::updateOSC() {
    // hide old messages
    while( coms.receiver_playing.hasWaitingMessages() )
    {
        incomingPlayerMessageCounter++;
        ofxOscMessage m;
        coms.receiver_playing.getNextMessage( m );
        if ( m.getAddress().compare( string("/PLAYING_VIDEO") ) == 0 )
        {
            //fc->updatePlayingVideo(m.getArgAsString(0));
            playingFileDuration = m.getArgAsInt64(1);

            if (this->speedSetting ==-1){
               featureGuiElements[0]->setValue(float(playingFileDuration));
            }
            featureGuiElements[0]->reset();
        }

        else
        {
            ofLogVerbose()<<  m.getAddress();
        }
    }

    //Uncomment this when using controller simulator
    //handlePythonMessages();
    while( coms.receiver_controller.hasWaitingMessages() )
    {
        incomingControllerMessageCounter++;
        // get the next message
        ofxOscMessage m;
        coms.receiver_controller.getNextMessage( m );

        if ( m.getAddress().compare( string("/ENCODER/STEP") ) == 0 )
        {
            handleKnobInput(m);
        }

        if ( m.getAddress().compare( string("/ENCODER/SWT") ) == 0 )
        {
            int idx = m.getArgAsInt(0);
            int swt = m.getArgAsInt(1);
            ofLogVerbose(ofToString(ofGetElapsedTimef(),3)) << " SWT Message received : Index"<< idx << "value"<<swt ;
            if (swt >0) handleButtonInput(idx);
        }
        else
        {
            ofLogVerbose()<<  m.getAddress();
        }
    }
}

void ofApp::handlePythonMessages(){
    // check for waiting messages
    while( coms.receiver_python_controller.hasWaitingMessages() )
    {
        // get the next message
        ofxOscMessage m;
        coms.receiver_python_controller.getNextMessage(m );

        // check for mouse button message
        if ( m.getAddress().compare( string("/FEATURE_DIFFS") ) == 0 )
        {
            // the single argument is a string
            for (int i =0; i<23;i++){
                float diff =  m.getArgAsFloat(i);
                if (abs(diff) >0){
                    fc->registerInputActivity();

                    if (i ==0){
                        fc->incrementSpeed(diff);
                    }
                    if (i ==1){
                        fc->incrementSearchRadius(diff);

                    }

                    if (i> 1){
                        fc->incrementFeatureTarget( i-2,  diff);
                    }
                }
            }
            ofLogError(ofToString(ofGetElapsedTimef(),3)) << "FEATURE_DIFFS Message received" ;
        }

        if ( m.getAddress().compare( string("/SET_SPEED") ) == 0 )
        {
            int speed=  m.getArgAsInt(0);
            fc->registerInputActivity();

            fc->setSpeed(speed);
            ofLogError(ofToString(ofGetElapsedTimef(),3)) << "SET_SPEED Message received" ;
        }
        else
        {
            ofLogVerbose()<<  m.getAddress() << "message received ... not handled";
        }
    }

}

void ofApp::handleKnobInput(ofxOscMessage m){
    //    ofLogDebug(ofToString(ofGetElapsedTimef(),3)) << " Step Message received : " ;
    fc->registerInputActivity();

    int i = m.getArgAsInt(0);
    if (i ==1){
        int step =m.getArgAsInt(1);
        if (step ==0) step = -1;

        fc->incrementSpeed(-step);
        return;
    }

    if (i ==2){
        int step =m.getArgAsInt(1);
        if (step ==0) step = -1;
        this->fc->incrementSearchRadius(step);
        return;
    }

    i =i-3;
    float step = (float(m.getArgAsInt(1)) -0.5)*2/50.;
    fc->incrementFeatureTarget(i, step);
}

void ofApp::handleButtonInput(int index){
    //    ofLogDebug(ofToString(ofGetElapsedTimef(),3)) << " Step Message received : " ;
    fc->registerInputActivity();

    if (index ==1){
        fc->toggleSpeed();
        return;
    }

    if (index ==2){
        fc->toggleNeighbours();
        //Todo: Set number of neighbours to something else
        return;
    }

    index =index-3;
    fc->toggleFeatureTarget(index);
}


bool ofApp::vectorsAreEqual(vector<string>v1, vector<string> v2){
    if (v1.size()!= v2.size()){
        return false;
    }
    else if (!std::equal(v1.begin(), v1.begin() + v1.size(), v2.begin())){
        return false;
    }
    return true;
}

//void ofApp::audioIn(ofSoundBuffer & buffer){
//    waveform.receiveBuffer(buffer);
//}

void ofApp::receiveBuffer(ofSoundBuffer & buffer){
    waveform.receiveBuffer(buffer);
}

void ofApp::logFilename(){
    filenameLog.open("filenames.txt",  ofFile::Append);
    filenameLog << fc->getPlayingVideo().first +"\n";
    filenameLog.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key =='l'){
        toggleLanguage();
    }

    if (key =='r'){
        playRandomVideo();
    }
    if (key =='a'){
        fc->toIdleActive();
    }
    if (key ==' '){
//        logFilename();
    }
}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}
//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    if (clickIndex>0){
        if (y  > (clickY +5)){
            ofxOscMessage m;
            m.addInt32Arg(clickIndex);
            m.addInt32Arg(-1);
            clickY = y;
            handleKnobInput(m);
        }
        else if (y  < (clickY -5)){
            ofxOscMessage m;
            m.addInt32Arg(clickIndex);
            m.addInt32Arg(1);
            clickY = y;
            handleKnobInput(m);
        }
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if (button==0){
        for (int i = 0; i<clickRectangles.size(); i++){
            if (clickRectangles[i].inside(x,y)){
                clickIndex = i+1;
                clickY =y;
                ofLogError()<<i << "selected"<<endl;
                break;
            }
        }
    }

    if (button==2){
        for (int i = 0; i<clickRectangles.size(); i++){
            if (clickRectangles[i].inside(x,y)){
                handleButtonInput(i+1);
            }
        }
    }


}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    clickIndex = -1;

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    setLayout();

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
