#include "colorcircle.h"

ColorCircle::ColorCircle():CircleFeatureGuiElement()
{
    setColorLimits(0.,360.);
    setRange(360);
}

void ColorCircle::setValue(float v){
//    currentColor = ofColor::fromHsb(v*256,200,200);

    currentValue = v;
}


void ColorCircle::setActive(bool v){
    CircleFeatureGuiElement::setActive(v);
}

void ColorCircle::drawTriangle(){
    ofPushMatrix();
    ofPushStyle();
    translateToCenter();
    //Draw control triangle
    if (active && triangle){
        ofPushMatrix();
        ofFill();
        int target_rot = this->targetValue*(rotationRange);
        ofRotateDeg(rotationMin +target_rot);
        ofTranslate(0, circleOuterRadius+4);
        ofVec2f t1(0,0);
        ofVec2f t2(5,10);
        ofVec2f t3(-5,10);
        ofDrawTriangle(t1, t2,t3);
        ofPopMatrix();
    }
    ofPopStyle();
    ofPopMatrix();
}

void ColorCircle::updateFillLine(){
    ofPushStyle();
    ofSetLineWidth(fillWidth);
    vector<double> widths;
    vector<ofFloatColor> colors;
    ofFloatColor c(1.);

    fillArc.clear();
    fillArc.arc(ofVec3f(0,0,0), fillRadius, fillRadius, rotationMin, rotationMax, rotationRange);
    fillArc.rotateDeg(90, ofVec3f(0,0,1));

    for(int i = 0; i< fillArc.getVertices().size(); i++){
        float h = ofMap(i, 0,fillArc.getVertices().size(),minColor,maxColor );
        c = ofFloatColor::fromHsb(h/255.,0.8,0.8,float(fillColor)/255.);
        colors.push_back(c);
    }

    fillLine.clear();
    fillLine.setFromPolyline(fillArc, colors);
    ofPopStyle();

}

void ColorCircle::setColorLimits(float min, float max){
    minColor = min;
    maxColor = max*255./360.;
}

void ColorCircle::drawShell(){
    ofPushMatrix();
    ofPushStyle();

    translateToCenter();

    //Draw outer circle
    ofSetColor(fillColor);
    ofSetLineWidth(2);

    exteriorPath.draw();

    //Draw interior circle
    interiorPath.draw();

    //Draw weight circle
    ofFill();
    float radius = (circleInnerRadius-10)*currentWeight;

    ofSetColor(weightCircleColor);
    ofDrawCircle(0,0,0,radius);

    ofPopStyle();
    ofPopMatrix();
}

void ColorCircle::drawFill(){
    //Move to draw position and draw name
    ofPushMatrix();
    ofPushStyle();
    translateToCenter();
    fillLine.draw();
    ofRotateDeg(this->rotationMin);

    ofRotateDeg(fillOffset);
    int current_rot = this->currentValue*(rotationRange -fillOffset*2);
    ofSetLineWidth(2);
    ofSetColor(fillColor);
    ofRotateDeg(fillOffset);
    ofRotateDeg(current_rot);
    ofDrawLine(0,circleInnerRadius,0,circleOuterRadius);

    ofPopStyle();
    ofPopMatrix();
}


