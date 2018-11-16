#ifndef AUDIOWAVEFORM_H
#define AUDIOWAVEFORM_H

#include "ofMain.h"
#include <mutex>
#include "ofxFft.h"
#include "spectrogramvisualizer.h"

#define IN_AUDIO_BUFFER_LENGTH 1024
#define INTERNAL_BUFFER_LENGTH IN_AUDIO_BUFFER_LENGTH*4
class AudioWaveform
{
public:
   AudioWaveform();
   void receiveBuffer(ofSoundBuffer& buffer);
   void setLayout(int x, int y, int w, int h);
   void draw();
   void update();
   void draw2D();
   void drawSpectrum(int w, int h);
   void drawWaveform(int w, int h);
   void shiftSpectrogram();

private:

   int xOffset, yOffset, width, height;
   void copyBuffer();
   void copyLeftRightBuffer();
   float powFreq(float i);
   SpectrogramVisualizer spectrogram;

   std::vector<float> soundBuffer;
   std::vector<float> drawBuffer;

   std::vector<float> leftDrawBuffer;
   std::vector<float> rightDrawBuffer;

   std::mutex bufferMutex;
   ofMutex spectroMutex;
   bool toggle;
   ofSoundBuffer leftBuffer;
   ofSoundBuffer rightBuffer;

   ofxFft* fft;
   vector<float> drawBins, middleBins, audioBins;

   float runningBufferMax;

   float runningMax;

   int spectrogramOffset;

};

#endif // AUDIOWAVEFORM_H
