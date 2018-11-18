/**
 *  ofxSoundObject.cpp
 *
 *  Created by Marek Bereza on 10/08/2013.
 */
#include "ofxTwoChannelFader.h"
//----------------------------------------------------
ofxTwoChannelFader::ofxTwoChannelFader(){
    masterVolume = 1.0f;
    masterVol.set("Master Vol", 1, 0, 1);
    masterVol.addListener(this, &ofxTwoChannelFader::masterVolChanged);
    current_channel =1;
    fade = false;
    same_input = false;
    channels.resize(2);
    channelVolume.resize(2);
    channelVolume[0] =1.;
    channelVolume[1] =1.;

}

void ofxTwoChannelFader::create_fades(int buffer_size, int num_channels){
    fadeIn.resize(buffer_size*num_channels, 0);
    fadeOut.resize(buffer_size*num_channels, 0);
    fadeInSolo.resize(buffer_size*num_channels, 1);


    int half_buffer =buffer_size/2;
    int index =0;
    for (int i =0; i <half_buffer; i++){
        for (int c = 0; c<num_channels; c++){
            fadeOut[index] = 1.0 -float(i)/half_buffer;
            fadeIn[half_buffer*num_channels +index] = float(i)/half_buffer;
            fadeInSolo[index] =  1.0 -float(i)/half_buffer;
            index++;
        }
    }
}

//----------------------------------------------------
void ofxTwoChannelFader::masterVolChanged(float& f) {
    mutex.lock();
    masterVolume = masterVol;
    mutex.unlock();
}

//----------------------------------------------------
ofxTwoChannelFader::~ofxTwoChannelFader(){
    channels.clear();
    channelVolume.clear();
}
//----------------------------------------------------
ofxSoundObject* ofxTwoChannelFader::getChannelSource(int channelNumber){
    if (channelNumber < channels.size()) {
        return channels[channelNumber];
    }else{
        return nullptr;
    }
}
//----------------------------------------------------
void ofxTwoChannelFader::disconnectInput(ofxSoundPlayerObject * input){
    for (int i =0; i<channels.size(); i++) {
        if (input == channels[i]) {
            channels.erase(channels.begin() + i);
            channelVolume.erase(channelVolume.begin() + i);
            break;
        }
    }
}

void  ofxTwoChannelFader::fadeTo(ofxSoundPlayerObject *obj){
    fade = true;
    same_input = false;
    for (int i =0; i<channels.size(); i++) {
        if (obj == channels[i]) {
            // ofLogNotice("ofxSoundMixer::setInput") << " already connected" << endl;
            // To fix popping on 2 videos need to add a check if i is the other channel and then set same_input accordingly
            same_input= true;
            if (i ==current_channel){
                same_same = true;
            }
            current_channel = i;
            return;
        }
    }
    //Swap the current channel index
    current_channel = 1- current_channel;
    channels[current_channel] = obj;
    channels[current_channel]->play();

}

//----------------------------------------------------
void ofxTwoChannelFader::setInput(ofxSoundObject *obj){
    fadeTo(dynamic_cast<ofxSoundPlayerObject*>( obj));
}
//----------------------------------------------------
int ofxTwoChannelFader::getNumChannels(){
    return channels.size();
}
//----------------------------------------------------
void ofxTwoChannelFader::setMasterVolume(float vol){
    mutex.lock();
    masterVolume = vol;
    mutex.unlock();
}
//----------------------------------------------------
float ofxTwoChannelFader::getMasterVolume(){
    return masterVolume;
}

//----------------------------------------------------
bool ofxTwoChannelFader::isConnectedTo(ofxSoundPlayerObject& obj){
    for (int i =0; i<channels.size(); i++) {
        if (&obj == channels[i]) {
            return true;
        }
    }
    return false;
}
//----------------------------------------------------
void ofxTwoChannelFader::setChannelVolume(int channelNumber, float vol){
    if (channelNumber < channelVolume.size()) {
        channelVolume[channelNumber] = vol;
    }
}
//----------------------------------------------------
float ofxTwoChannelFader::getChannelVolume(int channelNumber){
    if (channelNumber < channelVolume.size()) {
        return channelVolume[channelNumber];
    }
    return 0;
}

void ofxTwoChannelFader::copyLiveAudio(ofSoundBuffer &output){
    if (copyLock.try_lock()){
        ofSoundBuffer tb;
        output.getChannel(tb,0);
        audioCopyBuffer.append(tb);
        copyLock.unlock();
    }
}

void ofxTwoChannelFader::getAudio(ofSoundBuffer &input){
    if (copyLock.try_lock()){
        audioCopyBuffer.copyTo(input, audioCopyBuffer.size(),1,0);
        audioCopyBuffer.clear();
        copyLock.unlock();
    }
}

//----------------------------------------------------
// this pulls the audio through from earlier links in the chain and sums up the total output
void ofxTwoChannelFader::audioOut(ofSoundBuffer &output) {
    int last_channel = 1-current_channel;

    if (channels[current_channel] == NULL) {
        return;
    }

    ofSoundBuffer currentBuffer;
    currentBuffer.resize(output.size());
    currentBuffer.setNumChannels(output.getNumChannels());
    currentBuffer.setSampleRate(output.getSampleRate());
    dynamic_cast<ofxSoundObject*>(channels[current_channel])->audioOut(currentBuffer);

    //Case 1 : just reading audio, no need to crossfade
    if (!fade){
        for (int j = 0; j < currentBuffer.size(); j++) {
            output.getBuffer()[j] += currentBuffer.getBuffer()[j];
        }
    }

    //Handle fade
    else {
        fade = false;
        //Case 2 : have to do a crossfade on the same audio source
        if (same_input)
        {
            if (same_same){
                channels[current_channel]->play();

                ofSoundBuffer tempBuffer;
                tempBuffer.resize(output.size());
                tempBuffer.setNumChannels(output.getNumChannels());
                tempBuffer.setSampleRate(output.getSampleRate());
                dynamic_cast<ofxSoundObject*>(channels[current_channel])->audioOut(tempBuffer);

                //combine the channels with fade-in_fadeout boyos
                for (int j = 0; j < tempBuffer.size(); j++) {
                    output.getBuffer()[j] += tempBuffer.getBuffer()[j] * fadeIn[j] +currentBuffer.getBuffer()[j]*fadeOut[j];
    //                output.getBuffer()[j] +=  fadeIn[j] +fadeOut[j];

                }
                same_same = false;
            }
            else{
                channels[current_channel]->play();

                ofSoundBuffer tempBuffer;
                tempBuffer.resize(output.size());
                tempBuffer.setNumChannels(output.getNumChannels());
                tempBuffer.setSampleRate(output.getSampleRate());
                dynamic_cast<ofxSoundObject*>(channels[current_channel])->audioOut(tempBuffer);

                //combine the channels with fade-in_fadeout boyos
                for (int j = 0; j < tempBuffer.size(); j++) {
                    output.getBuffer()[j] += tempBuffer.getBuffer()[j] * fadeIn[j] +currentBuffer.getBuffer()[j]*fadeOut[j];
    //                output.getBuffer()[j] +=  fadeIn[j] +fadeOut[j];

                }

            }

            same_input = false;
        }

        //Case 3 : no last channel
        else if (channels[last_channel] == NULL){
            for (int j = 0; j < currentBuffer.size(); j++) {
                output.getBuffer()[j] += currentBuffer.getBuffer()[j]*fadeIn[j];
            }
        }

        //Case 4 : Both channels present
        else{
            ofSoundBuffer tempBuffer;
            tempBuffer.resize(output.size());
            tempBuffer.setNumChannels(output.getNumChannels());
            tempBuffer.setSampleRate(output.getSampleRate());
            dynamic_cast<ofxSoundObject*>(channels[last_channel])->audioOut(tempBuffer);
            //combine the channels with fade-in_fadeout boyos
            for (int j = 0; j < tempBuffer.size(); j++) {
                output.getBuffer()[j] += tempBuffer.getBuffer()[j]*fadeOut[j] +currentBuffer.getBuffer()[j]*fadeIn[j];
            }
            channels[last_channel]->stop();
        }
    }
    copyLiveAudio(output);

}

//----------------------------------------------------
