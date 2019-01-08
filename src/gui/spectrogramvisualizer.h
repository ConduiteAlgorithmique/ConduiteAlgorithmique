#ifndef SPECTROGRAMVISUALIZER_H
#define SPECTROGRAMVISUALIZER_H
#include "ofMain.h"
#include "ofxFft.h"
#include "ofxJsonSettings.h"
#include "ofxColorMap.h"

class SpectrogramVisualizer{
public:
    SpectrogramVisualizer(){}
    ~SpectrogramVisualizer(){}

    void initialize(ofxFft* fft, int width, int height){
        Settings::get().load("settings.json");
        bool expIndex = Settings::getFloat("log_spectral_axis");
        string cmapStr = Settings::getString("colormap");
        cmap.setMapFromName(cmapStr);

        image.allocate(height, width, OF_IMAGE_GRAYSCALE);
        colorImage.allocate(height, width, OF_IMAGE_COLOR);
        image.setColor(ofColor::black);
        int n = (int) image.getHeight();
        int maxBin = fft->getBinFromFrequency(8000);
        int spectrogramWidth = (int) image.getWidth();

        float binstep = maxBin/float(n);
        for(int i = 0; i < n; i++) {
    //        js.push_back((n - i - 1) * spectrogramWidth +  spectrogramWidth-1);
            js.push_back((n-1)*spectrogramWidth+ i);
//            js.push_back(i);
            if (expIndex){
                int expi = powFreq(float(i)/n +0.5)*i;
                binIndexes.push_back(expi*binstep);
            }
            else{
                binIndexes.push_back(i*binstep);
            }
        }
    }

    void draw(int xPos, int yPos, int width, int height){
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(xPos,height+yPos);
        ofRotateDeg(-90);
//        ofDrawRectangle(xPos, yPos, height, width);
        colorImage.draw(0, 0, height, width);
        ofPopStyle();
        ofPopMatrix();
    }

    void update(vector<float> audioBins){
        shift();
        int n = (int) image.getWidth();

        for(int i = 0; i < n; i++) {
            image.setColor(js[i] , (unsigned char) (255. * audioBins[binIndexes[i]]));
        }
        //Test gradient
//        ofPixels& p = image.getPixels();
//         for(size_t x = 0; x < image.getWidth(); x++) {
//             for(size_t y = 0; y < image.getHeight(); y++) {
//                 int value = 255*(float)x/image.getWidth();
//                 p[(int)(x+y*image.getWidth())] = value;
//             }
//         }
        image.update();
        cmap.apply(image,colorImage);
    }

private:
    ofImage image;
    ofImage colorImage;

    vector<int> js;
    vector<int> binIndexes;
    ofxColorMap cmap;


    void shift(){
        int spectrogramWidth = (int) image.getWidth();
        int spectrogramHeight= (int) image.getHeight();
        auto pixels = image.getPixels();
        auto pixelref = pixels.getData();
        memmove(pixelref, pixelref+spectrogramWidth, spectrogramWidth*spectrogramHeight - spectrogramWidth);
        image.setFromPixels(pixels);
    }


    float powFreq(float i) {
        return powf(i, 3);
    }
};

#endif // SPECTROGRAMVISUALIZER_H
