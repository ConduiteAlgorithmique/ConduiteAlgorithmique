#include "audiowaveform.h"
#include "ofxJsonSettings.h"

AudioWaveform::AudioWaveform()
{

    Settings::get().load("settings.json");

    spectrogramOffset=0;
    soundBuffer.reserve(INTERNAL_BUFFER_LENGTH);
    drawBuffer.reserve(INTERNAL_BUFFER_LENGTH);
    toggle = false;
    for (int i =0; i<INTERNAL_BUFFER_LENGTH; i++){
        soundBuffer.push_back(0.0f);
        drawBuffer.push_back(0.f);
    }

    fft = ofxFft::create(INTERNAL_BUFFER_LENGTH, OF_FFT_WINDOW_HAMMING);
    audioBins.resize(fft->getBinSize());
    spectrogram.initialize(fft,2*ofGetWidth()/3, ofGetHeight()/3 );
    runningMax =1.;


}

void AudioWaveform::receiveBuffer(ofSoundBuffer& buffer){

    int lastChunkStart = INTERNAL_BUFFER_LENGTH -IN_AUDIO_BUFFER_LENGTH ;
    std::lock_guard<std::mutex> lock(bufferMutex);
    buffer.getChannel(rightBuffer,1 );
    //leftBuffer.addTo(rightBuffer);
    std::copy(soundBuffer.begin()+IN_AUDIO_BUFFER_LENGTH, soundBuffer.end(), soundBuffer.begin());
    std::copy(std::begin(rightBuffer.getBuffer()),std::end(rightBuffer.getBuffer()), std::begin(soundBuffer)+lastChunkStart);
}


void AudioWaveform::copyBuffer(){
    std::lock_guard<std::mutex> lock(bufferMutex);
    drawBuffer = soundBuffer;
}

void AudioWaveform::copyLeftRightBuffer(){
    std::lock_guard<std::mutex> lock(bufferMutex);
    leftDrawBuffer = leftBuffer.getBuffer();
    rightDrawBuffer = rightBuffer.getBuffer();
}

void AudioWaveform::draw2D(){
    ofPushMatrix();
//    ofTranslate(cx, cy);

    //Copy the internal buffer
    copyLeftRightBuffer();
    ofNoFill();
    ofSetLineWidth(1);
    ofDrawRectangle(0, 0, width, height);

    ofSetColor(0, 255, 0);

    ofSetLineWidth(1);

    //ofBeginShape();
    for (unsigned int i = 0; i < IN_AUDIO_BUFFER_LENGTH; i++){
        ofSetColor(0, i *255.0/IN_AUDIO_BUFFER_LENGTH,0);

        ofDrawCircle(width/2 + leftDrawBuffer[i]*2000, height/2 -rightDrawBuffer[i]*2000, 0, 1);
        //ofVertex(width/2 - leftDrawBuffer[i]*4000, height/2 -rightDrawBuffer[i]*4000);
    }
   //   ofEndShape();


    ofPopMatrix();
}

void AudioWaveform::setLayout(int x, int y, int w, int h){
    this->xOffset = x;
    this->yOffset = y;
    this->width =w;
    this->height =h;
}

void AudioWaveform::drawWaveform(int w, int h){
    ofPushStyle();
    ofNoFill();
    ofSetColor(255);

    ofBeginShape();
    int step = INTERNAL_BUFFER_LENGTH/w;
    for (unsigned int i = 0; i < w; i++){
        ofVertex(i, h/2 -drawBuffer[step*i]*h/2);
    }
    ofSetLineWidth(2);
    ofDrawRectangle(0, 1, w, h-1);
    ofEndShape();
    ofPopStyle();
}

void AudioWaveform::drawSpectrum(int w, int h){
    //std::lock_guard<std::mutex> lock(spectroMutex);

    ofPushStyle();
    ofSetColor(255);
    spectrogram.draw(3, 3,  w-4, h-4);
    ofNoFill();
    ofSetLineWidth(2);
    ofDrawRectangle(1, 1,  w-1, h-1);
    ofPopStyle();
}

void AudioWaveform::draw(){
    //Copy the internal buffer

    ofPushMatrix();

    ofTranslate(xOffset, yOffset);
    drawSpectrum(2*width/3, height);
    ofTranslate(2*width/3, yOffset);
    drawWaveform(width/3, height);

    ofPopMatrix();
}

void AudioWaveform::update(){
    copyBuffer();
    float bufferMax = 0;
    for (auto v: drawBuffer){
        bufferMax = max(v, bufferMax);
    }

    runningBufferMax = runningBufferMax*0.99 + bufferMax*0.01;
    if (bufferMax >runningBufferMax){
        runningBufferMax = bufferMax;
    }
    runningBufferMax = max(runningBufferMax, 0.05f);

    for (int  i= 0; i< drawBuffer.size(); i++){
        drawBuffer[i]/=(runningBufferMax*2);
    }

    fft->setSignal(&drawBuffer[0]);

    float* curFft = fft->getAmplitude();
    memcpy(&audioBins[0], curFft, sizeof(float) * fft->getBinSize());

    float maxValue = 0.;
    for(int i = 0; i < fft->getBinSize(); i++) {
        if(abs(audioBins[i]) > maxValue) {
            maxValue = abs(audioBins[i]);
        }
    }

    runningMax = runningMax*0.99 + maxValue*0.01;

    if (maxValue >runningMax){
        runningMax = maxValue;
    }
    runningMax = CLAMP(runningMax , 0.01, 1.);
    for(int i = 0; i < fft->getBinSize(); i++) {
        audioBins[i] /= runningMax;
    }
    spectrogram.update(audioBins);
//    ofLogError()<<"Buffer max :" << bufferMax << "\n Running buffer max" << runningBufferMax <<endl;
//    ofLogError()<<"Bins max :" << maxValue << "\n Running bins max" << runningMax <<endl;
}




