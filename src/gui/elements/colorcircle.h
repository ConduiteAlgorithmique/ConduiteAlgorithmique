#ifndef COLORCIRCLE_H
#define COLORCIRCLE_H

#include "ofMain.h"
#include "circlefeatureguielement.h"

class ColorCircle: public CircleFeatureGuiElement
{
public:
    ColorCircle();

    void drawFill();
    void drawShell();
    void setValue(float);
    void updateFillLine();
    void setActive(bool);
    void drawTriangle();
    void setColorLimits(float, float);

    ofColor currentColor;
    float minColor, maxColor;

};

#endif // COLORCIRCLE_H
