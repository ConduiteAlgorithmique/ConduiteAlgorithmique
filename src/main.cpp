#include "ofMain.h"
#include "ofApp.h"
#include "ofVideoApp.h"
#include "ofAppGLFWWindow.h"
#include "ofxJsonSettings.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

//========================================================================
int main( ){


    //Run interface first
    Settings::get().load("settings.json");
    ofGLFWWindowSettings interfaceSettings;
    interfaceSettings.setGLVersion(2,1);
    interfaceSettings.setSize(Settings::getInt("interface_monitor_width"), Settings::getInt("interface_monitor_height"));
    interfaceSettings.monitor =Settings::getInt("interface_monitor_index");
    interfaceSettings.windowMode = Settings::getBool("windowed") ? OF_WINDOW :OF_FULLSCREEN;



    //Then run video player
    ofGLFWWindowSettings videoSettings;
    videoSettings.setGLVersion(2,1);
    videoSettings.setSize(Settings::getInt("video_monitor_width"), Settings::getInt("video_monitor_height"));
    videoSettings.monitor =Settings::getInt("video_monitor_index");
    videoSettings.windowMode = Settings::getBool("windowed") ? OF_WINDOW :OF_FULLSCREEN;

    shared_ptr<ofAppBaseWindow> videoWindow = ofCreateWindow(videoSettings);
    shared_ptr<ofVideoApp> videoApp (new ofVideoApp());
    auto t =std::dynamic_pointer_cast<ofAppBaseGLWindow> (videoWindow);

    // REALLY IMPORTANT to create the window and app before creating the second window,
    // otherwise the shaders don't work
    shared_ptr<ofAppBaseWindow> interfaceWindow = ofCreateWindow(interfaceSettings);
    shared_ptr<ofApp> interfaceApp (new ofApp());
    ofRunApp(videoWindow, videoApp);
    ofRunApp(interfaceWindow, interfaceApp);


//    glfwSetWindowFocusCallback( std::dynamic_pointer_cast<ofAppGLFWWindow> (t)->getGLFWWindow(), videoApp->window_focus_callback );
//    videoApp->setWaveformPointer(interfaceApp->getWaveform());
    ofRunMainLoop();
}
