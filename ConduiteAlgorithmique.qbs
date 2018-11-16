import qbs
import qbs.Process
import qbs.File
import qbs.FileInfo
import qbs.TextFile
import "../../../libs/openFrameworksCompiled/project/qtcreator/ofApp.qbs" as ofApp

Project{
    property string of_root: '../../..'

    ofApp {
        name: { return FileInfo.baseName(path) }

        files: [
            "bin/data/shadersGL2/Billboard.frag",
            "bin/data/shadersGL2/Billboard.vert",
            "bin/data/shadersGL3/Billboard.frag",
            "bin/data/shadersGL3/Billboard.vert",
            "bin/data/settings.json",
            "src/gui/imagemanager.cpp",
            "src/gui/imagemanager.h",
            "src/gui/segnetcolourinspector.cpp",
            "src/gui/segnetcolourinspector.h",
            "src/gui/elements/colorcircle.h",
            "src/gui/elements/colorcircle.cpp",
            "src/gui/elements/circlefeatureguielement.h",
            "src/gui/elements/circlefeatureguielement.cpp",
            "src/gui/elements/searchradiuselement.h",
            "src/gui/elements/uielements.h",
            "src/gui/curveline.h",
            "src/gui/elements/durationElement.h",
            "src/gui/elements/tiltelement.h",
            "src/gui/physicalline.h",
            "src/gui/pointcloudrenderer.cpp",
            "src/gui/pointcloudrenderer.h",
            "src/gui/spectrogramvisualizer.h",
            "src/ofVideoApp.cpp",
            "src/ofVideoApp.h",
            "src/util/communication.h",
            "src/util/databaseloader.cpp",
            "src/util/databaseloader.h",
            "src/util/featurecontrol.cpp",
            "src/util/featurecontrol.h",
            "src/util/featuresearch.cpp",
            "src/util/featuresearch.h",
            "src/gui/audiowaveform.cpp",
            "src/gui/audiowaveform.h",
            "src/util/imageloader.cpp",
            "src/util/imageloader.h",
            "src/util/searchspace.h",
            "src/videoplayer/simplesampleplayer.cpp",
            "src/videoplayer/simplesampleplayer.h",
            "src/videoplayer/happlayermanager.cpp",
            "src/videoplayer/happlayermanager.h",
            "src/videoplayer/ofxTwoChannelFader.cpp",
            "src/videoplayer/ofxTwoChannelFader.h",
            "src/main.cpp",
            "src/ofApp.cpp",
            "src/ofApp.h",
            "src/videoplayer/videodatabaseloader.cpp",
            "src/videoplayer/videodatabaseloader.h",
        ]

        of.addons: [
            'ofxOsc',
            'ofxHDF5',
            'ofxFft',
            'ofxJsonSettings',
            'ofxJSON',
            'ofxFatLines',
            'ofxHapPlayer',
            'ofxSoundObjects'
        ]

        // additional flags for the project. the of module sets some
        // flags by default to add the core libraries, search paths...
        // this flags can be augmented through the following properties:
        of.pkgConfigs: []       // list of additional system pkgs to include
        of.includePaths: []     // include search paths
        of.cFlags: []           // flags passed to the c compiler
        of.cxxFlags: []         // flags passed to the c++ compiler
        of.linkerFlags: []      // flags passed to the linker
        of.defines: []          // defines are passed as -D to the compiler
        // and can be checked with #ifdef or #if in the code
        of.frameworks: []       // osx only, additional frameworks to link with the project

        // other flags can be set through the cpp module: http://doc.qt.io/qbs/cpp-module.html
        // eg: this will enable ccache when compiling
        //
        // cpp.compilerWrapper: 'ccache'

        Depends{
            name: "cpp"
        }

        // common rules that parse the include search paths, core libraries...
        Depends{
            name: "of"
        }

        // dependency with the OF library
        Depends{
            name: "openFrameworks"
        }
    }

    references: [FileInfo.joinPaths(of_root, "/libs/openFrameworksCompiled/project/qtcreator/openFrameworks.qbs")]
}
