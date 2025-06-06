/************************************************************************
 ************************************************************************
 FAUST API Architecture File
 Copyright (C) 2016 GRAME, Romain Michon, CCRMA - Stanford University
 Copyright (C) 2014-2017 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 
 This Architecture section is free software; you can redistribute it
 and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3 of
 the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; If not, see <http://www.gnu.org/licenses/>.
 
 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ************************************************************************
 ************************************************************************/

#include <cmath>
#include <cstring>
#include <iostream>
#include <string.h>
#include <stdio.h>

#include "faust/misc.h"
#include "faust/gui/DecoratorUI.h"
#include "faust/gui/JSONUIDecoder.h"
#include "faust/dsp/dsp.h"
#include "faust/dsp/dsp-adapter.h"
#include "faust/gui/meta.h"

#include "mfx-ring-buffer.h"

// we require macro declarations
#define FAUST_UIMACROS

// but we will ignore most of them
#define FAUST_ADDBUTTON(l,f)
#define FAUST_ADDCHECKBOX(l,f)
#define FAUST_ADDVERTICALSLIDER(l,f,i,a,b,s)
#define FAUST_ADDHORIZONTALSLIDER(l,f,i,a,b,s)
#define FAUST_ADDNUMENTRY(l,f,i,a,b,s)
#define FAUST_ADDVERTICALBARGRAPH(l,f,a,b)
#define FAUST_ADDHORIZONTALBARGRAPH(l,f,a,b)

// To force correct miniaudio include
#if MINIAUDIO_DRIVER
#define MINIAUDIO_IMPLEMENTATION
#include "faust/miniaudio.h"
#endif

//**************************************************************
// Soundfile handling
//**************************************************************

// Must be done before <<includeclass>> otherwise the 'Soundfile' type is not known

#if SOUNDFILE
// So that the code uses JUCE audio file loading code
#if JUCE_DRIVER
#define JUCE_64BIT 1
#elif MINIAUDIO_DRIVER
#define MINIAUDIO_READER
#endif
#include "faust/gui/SoundUI.h"
#endif

//**************************************************************
// OSC configuration (hardcoded for now...)
//**************************************************************

#define OSC_IP_ADDRESS  "192.168.1.112"
#define OSC_IN_PORT     "5510"
#define OSC_OUT_PORT    "5511"

//**************************************************************
// Intrinsic
//**************************************************************




#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>

#ifndef FAUSTCLASS 
#define FAUSTCLASS mydsp
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

struct rb_compute_buffers {
    uint8_t channels;
    uint16_t nframes;
    float buffers[16][2048];
};

class dsp_display : public decorator_dsp {

    
    public:
        mfx_ringbuffer_t *rb = nullptr;
    
        dsp_display(::dsp* dsp = nullptr):decorator_dsp(dsp)
        {
        }

        ~dsp_display()
        {
        }

        void init(int sample_rate)
        {
            decorator_dsp::init(sample_rate);
        }

        void compute(double date_usec, int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)
        {
            decorator_dsp::compute(count, inputs, outputs);
            
            if (rb)
            {
                rb_compute_buffers rb_buffer;
                rb_buffer.nframes = count;
                rb_buffer.channels = getNumOutputs();
                for (size_t i = 0; i < getNumOutputs(); i++) {
                    memcpy(rb_buffer.buffers[i], outputs[i], count * sizeof(float));
                }
                if (mfx_ringbuffer_write_space(rb) >= sizeof(rb_buffer))
                {
                    size_t written = mfx_ringbuffer_write(rb, reinterpret_cast<const char *>(&rb_buffer), sizeof(rb_buffer));
                }

            }
        }
    
};

class mydsp : public dsp {
	
 private:
	
	int fSampleRate;
	
 public:
	mydsp() {
	}
	
	void metadata(Meta* m) { 
		m->declare("compile_options", "-a api/DspFaust.cpp -lang cpp -i -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("filename", "_.dsp");
		m->declare("name", "_");
	}

	virtual int getNumInputs() {
		return 0;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
	}
	
	virtual void instanceResetUserInterface() {
	}
	
	virtual void instanceClear() {
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	
	virtual void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual mydsp* clone() {
		return new mydsp();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("_");
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			output0[i0] = FAUSTFLOAT(0);
		}
	}

};

//**************************************************************
// Polyphony
//**************************************************************

#include "faust/dsp/faust-poly-engine.h"

//**************************************************************
// Audio driver
//**************************************************************

#if COREAUDIO_DRIVER
#include "faust/audio/coreaudio-dsp.h"
#elif IOS_DRIVER
#include "faust/audio/coreaudio-ios-dsp.h"
#elif ANDROID_DRIVER
#include "faust/audio/oboe-dsp.h"
#elif ALSA_DRIVER
#include "faust/audio/alsa-dsp.h"
#elif JACK_DRIVER
#include "faust/audio/jack-dsp.h"
#elif PORTAUDIO_DRIVER
#include "faust/audio/portaudio-dsp.h"
#elif RTAUDIO_DRIVER
#include "faust/audio/rtaudio-dsp.h"
#elif MINIAUDIO_DRIVER
#include "faust/audio/miniaudio-dsp.h"
#elif OPEN_FRAMEWORK_DRIVER
#include "faust/audio/ofaudio-dsp.h"
#elif JUCE_DRIVER
#include "faust/audio/juce-dsp.h"
#elif DUMMY_DRIVER
#include "faust/audio/dummy-audio.h"
#elif TEENSY_DRIVER
#include "faust/audio/teensy-dsp.h"
#elif ESP32_DRIVER
#include "faust/audio/esp32-dsp.h"
#endif

//**************************************************************
// Interface
//**************************************************************

#if MIDICTRL
#if JACK_DRIVER
// Nothing to add since jack-dsp.h contains MIDI
#elif JUCE_DRIVER
#include "faust/midi/juce-midi.h"
#elif TEENSY_DRIVER
#include "faust/midi/teensy-midi.h"
#elif ESP32_DRIVER
#include "faust/midi/esp32-midi.h"
#else
#include "faust/midi/rt-midi.h"
#include "faust/midi/RtMidi.cpp"
#endif
#endif

#if OSCCTRL
#if JUCE_DRIVER
#include "faust/gui/JuceOSCUI.h"
#else
#include "faust/gui/OSCUI.h"
#endif
#endif

#if DYNAMIC_DSP
#include "faust/dsp/llvm-dsp.h"
#endif

#include "DspFaust.h"

using namespace std;

std::list<GUI*> GUI::fGuiList;
ztimedmap GUI::gTimedZoneMap;

static bool hasCompileOption(const string& options, const char* option)
{
    const char* sep = " ";
    string options_copy = options; // strtok modifies the string in-place
    for (char* token = strtok(&options_copy[0], sep); token; token = strtok(nullptr, sep)) {
        if (strcmp(token, option) == 0) return true;
    }
    return false;
}

DspFaust::DspFaust(bool auto_connect)
{
#if JACK_DRIVER
    // JACK has its own sample rate and buffer size
#if MIDICTRL
    fDriver = new jackaudio_midi(auto_connect);
#else
    fDriver = new jackaudio(auto_connect);
#endif
#elif JUCE_DRIVER
    // JUCE audio device has its own sample rate and buffer size
    fDriver = new juceaudio();
#elif ANDROID_DRIVER
    fDriver = new oboeaudio(-1);
#else
    printf("You are not setting 'sample_rate' and 'buffer_size', but the audio driver needs it !\n");
    throw std::bad_alloc();
#endif
    init(new mydsp(), fDriver);
}

DspFaust::DspFaust(int sample_rate, int buffer_size, bool auto_connect)
{
    fDriver = createDriver(sample_rate, buffer_size, auto_connect);
    init(new mydsp(), fDriver);
}

#if DYNAMIC_DSP
DspFaust::DspFaust(const string& dsp_content, int sample_rate, int buffer_size, int argc, const char* argv[], bool auto_connect)
{
    string error_msg;
    
    // Is dsp_content a filename ?
    fFactory = createDSPFactoryFromFile(dsp_content, argc, argv, "", error_msg, -1);
    if (!fFactory) {
        fprintf(stderr, "ERROR : %s", error_msg.c_str());
        // Is dsp_content a string ?
        fFactory = createDSPFactoryFromString("FaustDSP", dsp_content, argc, argv, "", error_msg);
        if (!fFactory) {
            fprintf(stderr, "ERROR : %s", error_msg.c_str());
            throw runtime_error(error_msg);
        }
    }
    
    dsp* dsp = fFactory->createDSPInstance();
    if (!dsp) {
        fprintf(stderr, "Cannot allocate DSP instance\n");
        deleteDSPFactory(fFactory);
        throw runtime_error("Cannot allocate DSP instance");
    }
    fDriver = createDriver(sample_rate, buffer_size, auto_connect);
    dsp = createDisplay(dsp);
    init(dsp, fDriver);
}
#endif

dsp* DspFaust::createDisplay(dsp* dsp)
{
    display = new dsp_display(dsp);
    return display;
}

audio* DspFaust::createDriver(int sample_rate, int buffer_size, bool auto_connect)
{
    audio* driver;
#if COREAUDIO_DRIVER
    driver = new coreaudio(sample_rate, buffer_size);
#elif IOS_DRIVER
    driver = new iosaudio(sample_rate, buffer_size);
#elif ANDROID_DRIVER
    // OBOE has its own and buffer size
    fprintf(stderr, "You are setting 'buffer_size' with a driver that does not need it !\n");
    driver = new oboeaudio(-1);
#elif ALSA_DRIVER
    driver = new alsaaudio(sample_rate, buffer_size);
#elif JACK_DRIVER
    // JACK has its own sample rate and buffer size
    fprintf(stderr, "You are setting 'sample_rate' and 'buffer_size' with a driver that does not need it !\n");
#if MIDICTRL
    driver = new jackaudio_midi(auto_connect);
#else
    driver = new jackaudio(auto_connect);
#endif
#elif PORTAUDIO_DRIVER
    driver = new portaudio(sample_rate, buffer_size);
#elif RTAUDIO_DRIVER
    driver = new rtaudio(sample_rate, buffer_size);
#elif MINIAUDIO_DRIVER
    driver = new miniaudio(sample_rate, buffer_size);
#elif OPEN_FRAMEWORK_DRIVER
    driver = new ofaudio(sample_rate, buffer_size);
#elif JUCE_DRIVER
    // JUCE audio device has its own sample rate and buffer size
    fprintf(stderr, "You are setting 'sample_rate' and 'buffer_size' with a driver that does not need it !\n");
    driver = new juceaudio();
#elif ESP32_DRIVER
    driver = new esp32audio(sample_rate, buffer_size);
#elif DUMMY_DRIVER
    driver = new dummyaudio(sample_rate, buffer_size);
#endif
    return driver;
}

void DspFaust::init(dsp* mono_dsp, audio* driver)
{
#if MIDICTRL
#if JACK_DRIVER
    fMidiHandler = static_cast<jackaudio_midi*>(driver);
    fMidiInterface = new MidiUI(fMidiHandler);
#elif JUCE_DRIVER
    fMidiHandler = new juce_midi();
    fMidiInterface = new MidiUI(fMidiHandler);
#elif TEENSY_DRIVER
    fMidiHandler = new teensy_midi();
    fMidiInterface = new MidiUI(fMidiHandler);
#elif ESP32_DRIVER
    fMidiHandler = new esp32_midi();
    fMidiInterface = new MidiUI(fMidiHandler);
#else
    fMidiHandler = new rt_midi();
    fMidiInterface = new MidiUI(fMidiHandler);
#endif
    fPolyEngine = new FaustPolyEngine(mono_dsp, driver, fMidiHandler);
    fPolyEngine->buildUserInterface(fMidiInterface);
#else
    fPolyEngine = new FaustPolyEngine(mono_dsp, driver);
#endif
    
#if OSCCTRL
#if JUCE_DRIVER
    fOSCInterface = new JuceOSCUI(OSC_IP_ADDRESS, atoi(OSC_IN_PORT), atoi(OSC_OUT_PORT));
#else
    const char* argv[9];
    argv[0] = "Faust";  // TODO may be should retrieve the actual name
    argv[1] = "-xmit";
    argv[2] = "1";      // TODO retrieve that from command line or somewhere
    argv[3] = "-desthost";
    argv[4] = OSC_IP_ADDRESS;   // TODO same
    argv[5] = "-port";
    argv[6] = OSC_IN_PORT;      // TODO same
    argv[7] = "-outport";
    argv[8] = OSC_OUT_PORT;     // TODO same
    /*
     Deactivated for now (sometimes crashing)
     argv[9] = "-bundle";
     argv[10] = "1";             // TODO same
     */
    fOSCInterface = new OSCUI("Faust", 9, (char**)argv); // TODO fix name
#endif
    fPolyEngine->buildUserInterface(fOSCInterface);
#endif
    
    // Retrieving DSP object 'compile_options'
    struct MyMeta : public Meta
    {
        string fCompileOptions;
        void declare(const char* key, const char* value)
        {
            if (strcmp(key, "compile_options") == 0) fCompileOptions = value;
        }
        MyMeta(){}
    };
    
    MyMeta meta;
    mono_dsp->metadata(&meta);
    bool is_double = hasCompileOption(meta.fCompileOptions, "-double");
    
#if SOUNDFILE
#if JUCE_DRIVER
    auto file = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory().getParentDirectory().getChildFile("Resources");
    fSoundInterface = new SoundUI(file.getFullPathName().toStdString(), -1, nullptr, is_double);
#else
    // Use bundle path
    fSoundInterface = new SoundUI(SoundUI::getBinaryPath(), -1, nullptr, is_double);
#endif
    fPolyEngine->buildUserInterface(fSoundInterface);
#endif
}

DspFaust::~DspFaust()
{
#if OSCCTRL
    delete fOSCInterface;
#endif
#if SOUNDFILE
    delete fSoundInterface;
#endif
    delete fPolyEngine;
#if DYNAMIC_DSP
    deleteDSPFactory(static_cast<llvm_dsp_factory*>(fFactory));
#endif
#if MIDICTRL
    delete fMidiInterface;  // after deleting fPolyEngine;
#if JACK_DRIVER
    // JACK has its own MIDI interface, don't delete fMidiHandler
#else
    delete fMidiHandler;    // after deleting fMidiInterface;
#endif

#endif
    delete fDriver;
}

bool DspFaust::start()
{
#if OSCCTRL
    fOSCInterface->run();
#endif
#if MIDICTRL
    if (!fMidiInterface->run()) {
        fprintf(stderr, "MIDI run error...\n");
    }
#endif
    return fPolyEngine->start();
}

void DspFaust::stop()
{
#if OSCCTRL
    fOSCInterface->stop();
#endif
#if MIDICTRL
    fMidiInterface->stop();
#endif
    fPolyEngine->stop();
}

bool DspFaust::configureOSC(int xmit, int inport, int outport, int errport, const char* address)
{
#if OSCCTRL
#if JUCE_DRIVER
    // Nothing for now
    return false;
#else
    if (isRunning()) {
        return false;
    } else {
        oscfaust::OSCControler::gXmit = xmit;
        fOSCInterface->setUDPPort(inport);
        fOSCInterface->setUDPOut(outport);
        fOSCInterface->setUDPErr(errport);
        fOSCInterface->setDestAddress(address);
        return true;
    }
#endif
#else
    return false;
#endif
}

bool DspFaust::isOSCOn()
{
#if OSCCTRL
    return true;
#else
    return false;
#endif
}

bool DspFaust::isRunning()
{
    return fPolyEngine->isRunning();
}

uintptr_t DspFaust::keyOn(int pitch, int velocity)
{
    return (uintptr_t)fPolyEngine->keyOn(pitch, velocity);
}

int DspFaust::keyOff(int pitch)
{
    return fPolyEngine->keyOff(pitch);
}

uintptr_t DspFaust::newVoice()
{
    return (uintptr_t)fPolyEngine->newVoice();
}

int DspFaust::deleteVoice(uintptr_t voice)
{
    return fPolyEngine->deleteVoice(voice);
}

void DspFaust::allNotesOff(bool hard)
{
    fPolyEngine->allNotesOff(hard);
}

void DspFaust::propagateMidi(int count, double time, int type, int channel, int data1, int data2)
{
    fPolyEngine->propagateMidi(count, time, type, channel, data1, data2);
}

const char* DspFaust::getJSONUI()
{
    return fPolyEngine->getJSONUI();
}

const char* DspFaust::getJSONMeta()
{
    return fPolyEngine->getJSONMeta();
}

void DspFaust::buildUserInterface(UI* ui_interface)
{
    fPolyEngine->buildUserInterface(ui_interface);
}

int DspFaust::getParamsCount()
{
    return fPolyEngine->getParamsCount();
}

void DspFaust::setParamValue(const char* address, float value)
{
    fPolyEngine->setParamValue(address, value);
}

void DspFaust::setParamValue(int id, float value)
{
    fPolyEngine->setParamValue(id, value);
}

float DspFaust::getParamValue(const char* address)
{
    return fPolyEngine->getParamValue(address);
}

float DspFaust::getParamValue(int id)
{
    return fPolyEngine->getParamValue(id);
}

void DspFaust::setVoiceParamValue(const char* address, uintptr_t voice, float value)
{
    fPolyEngine->setVoiceParamValue(address, voice, value);
}

void DspFaust::setVoiceParamValue(int id, uintptr_t voice, float value)
{
    fPolyEngine->setVoiceParamValue(id, voice, value);
}

float DspFaust::getVoiceParamValue(const char* address, uintptr_t voice)
{
    return fPolyEngine->getVoiceParamValue(address, voice);
}

float DspFaust::getVoiceParamValue(int id, uintptr_t voice)
{
    return fPolyEngine->getVoiceParamValue(id, voice);
}

const char* DspFaust::getParamAddress(int id)
{
    return fPolyEngine->getParamAddress(id);
}

const char* DspFaust::getVoiceParamAddress(int id, uintptr_t voice)
{
    return fPolyEngine->getVoiceParamAddress(id, voice);
}

float DspFaust::getParamMin(const char* address)
{
    return fPolyEngine->getParamMin(address);
}

float DspFaust::getParamMin(int id)
{
    return fPolyEngine->getParamMin(id);
}

float DspFaust::getParamMax(const char* address)
{
    return fPolyEngine->getParamMax(address);
}

float DspFaust::getParamMax(int id)
{
    return fPolyEngine->getParamMax(id);
}

float DspFaust::getParamInit(const char* address)
{
    return fPolyEngine->getParamInit(address);
}

float DspFaust::getParamInit(int id)
{
    return fPolyEngine->getParamInit(id);
}

const char* DspFaust::getMetadata(const char* address, const char* key)
{
    return fPolyEngine->getMetadata(address, key);
}

const char* DspFaust::getMetadata(int id, const char* key)
{
    return fPolyEngine->getMetadata(id, key);
}

void DspFaust::propagateAcc(int acc, float v)
{
    fPolyEngine->propagateAcc(acc, v);
}

void DspFaust::setAccConverter(int p, int acc, int curve, float amin, float amid, float amax)
{
    fPolyEngine->setAccConverter(p, acc, curve, amin, amid, amax);
}

void DspFaust::propagateGyr(int acc, float v)
{
    fPolyEngine->propagateGyr(acc, v);
}

void DspFaust::setGyrConverter(int p, int gyr, int curve, float amin, float amid, float amax)
{
    fPolyEngine->setGyrConverter(p, gyr, curve, amin, amid, amax);
}

float DspFaust::getCPULoad()
{
    return fPolyEngine->getCPULoad();
}

int DspFaust::getScreenColor()
{
    return fPolyEngine->getScreenColor();
}

#ifdef BUILD
#include <unistd.h>

int main(int argc, char* argv[])
{
    try {
#ifdef DYNAMIC_DSP
        if (argc == 1) {
            printf("./dynamic-api <foo.dsp> \n");
            exit(-1);
        }
        DspFaust* dsp = new DspFaust(argv[1], 44100, 512);
#else
        DspFaust* dsp = new DspFaust(44100, 512);
#endif
        dsp->start();
        printf("Type 'q' to quit\n");
        char c;
        while ((c = getchar()) && (c != 'q')) { usleep(100000); }
        dsp->stop();
        delete dsp;
    } catch (...) {
        fprintf(stderr, "Cannot allocate or start DspFaust\n");
    }
}

#endif

extern "C"
{
  typedef struct {
    void (*openTabBox)(const char* label);
    void (*openHorizontalBox)(const char* label);
    void (*openVerticalBox)(const char* label);
    void (*closeBox)();
    void (*addButton)(const char* label, FAUSTFLOAT* zone);
    void (*addCheckButton)(const char* label, FAUSTFLOAT* zone);
    void (*addVerticalSlider)(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step);
    void (*addHorizontalSlider)(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step);
    void (*addNumEntry)(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step);
    void (*addHorizontalBargraph)(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max);
    void (*addVerticalBargraph)(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max);
    void (*addSoundfile)(const char* label, const char* soundpath, Soundfile** sf_zone);
    void (*declare)(FAUSTFLOAT* zone, const char* key, const char* val);  
  }  CLuaUI;
  class LuaUI : public GenericUI
  {    
  public:
    CLuaUI* lua_struct;
    LuaUI(CLuaUI* lua_struct) : lua_struct(lua_struct) {}
    virtual ~LuaUI() {}        
    virtual void openTabBox(const char* label) { if (lua_struct->openTabBox != NULL) lua_struct->openTabBox(label); }
    virtual void openHorizontalBox(const char* label) { if (lua_struct->openHorizontalBox != NULL) lua_struct->openHorizontalBox(label); }
    virtual void openVerticalBox(const char* label) { if (lua_struct->openVerticalBox != NULL) lua_struct->openVerticalBox(label); }
    virtual void closeBox() { if (lua_struct->closeBox != NULL) lua_struct->closeBox(); }        
    virtual void addButton(const char* label, FAUSTFLOAT* zone) { if (lua_struct->addButton != NULL) lua_struct->addButton(label, zone); }
    virtual void addCheckButton(const char* label, FAUSTFLOAT* zone) { if (lua_struct->addCheckButton != NULL) lua_struct->addCheckButton(label, zone); }
    virtual void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) { if (lua_struct->addVerticalSlider != NULL) lua_struct->addVerticalSlider(label, zone, init, min, max, step); }
    virtual void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) { if (lua_struct->addHorizontalSlider != NULL) lua_struct->addHorizontalSlider(label, zone, init, min, max, step); }
    virtual void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) { if (lua_struct->addNumEntry != NULL) lua_struct->addNumEntry(label, zone, init, min, max, step); }    
    virtual void addHorizontalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) { if (lua_struct->addHorizontalBargraph != NULL) lua_struct->addHorizontalBargraph(label, zone, min, max); }
    virtual void addVerticalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) { if (lua_struct->addVerticalBargraph != NULL) lua_struct->addVerticalBargraph(label, zone, min, max); }    
    virtual void addSoundfile(const char* label, const char* soundpath, Soundfile** sf_zone) { if (lua_struct->addSoundfile != NULL) lua_struct->addSoundfile(label, soundpath, sf_zone); }    
    virtual void declare(FAUSTFLOAT* zone, const char* key, const char* val) { if (lua_struct->declare != NULL) lua_struct->declare(zone, key, val); }    
  };
}

struct MfxDspFaust : DspFaust
{ 
  int check = 42;
  MfxDspFaust (const char * file, int sample_rate, int buffer_size, int argc, const char* argv[]) : DspFaust(string(file), sample_rate, buffer_size, argc, argv, false)
  {
    std::cout << "MfxDspFaust()" << '\n';
  }
  ~MfxDspFaust()
  {
    std::cout << "~MfxDspFaust()" << '\n';
  }
  
  void buildCLuaInterface(CLuaUI* lua_struct)
  {
    LuaUI lua(lua_struct);
    buildUserInterface(&lua);
  }  
};

extern "C"
{
typedef void lua_DspFaust;
lua_DspFaust* lua_newDspfaust(const char * file, char * error_msg, int sample_rate, int buffer_size, int argc, const char* argv[]);
void lua_startDspfaust(lua_DspFaust* dsp);
void lua_stopDspfaust(lua_DspFaust* dsp);
void lua_buildCLuaInterface(lua_DspFaust* dsp, CLuaUI* lua_struct);
float* lua_getDspMemory(lua_DspFaust* dsp);
void lua_setRingbuffer(lua_DspFaust* dsp, mfx_ringbuffer_t* rb);
void lua_deleteDspfaust(lua_DspFaust* dsp);
void printVersionAndTarget();

void printVersionAndTarget()
{
  std::cout << "Libfaust version : " << getCLibFaustVersion () << std::endl;
  std::cout << "getDSPMachineTarget " << getDSPMachineTarget() << std::endl;
#if JACK_DRIVER
  std::cout << "Driver: JACK" << std::endl;
#elif RTAUDIO_DRIVER
  std::cout << "Driver: RtAudio" << std::endl;
#endif
}

lua_DspFaust* lua_newDspfaust(const char * file, char * error_msg, int sample_rate, int buffer_size, int argc, const char* argv[])
{
    printf("Loading file: %s\n", file);
    try {
      MfxDspFaust* dsp = new MfxDspFaust(file, sample_rate, buffer_size, argc, argv);
      error_msg[0] = '\0';
      return static_cast<lua_DspFaust*>(dsp);
    } catch (runtime_error& e) {
      strcpy(error_msg, e.what());
      return static_cast<lua_DspFaust*>(nullptr);
    }
}

void lua_startDspfaust(lua_DspFaust* dsp) 
{
    static_cast<MfxDspFaust*>(dsp)->start();
}

void lua_deleteDspfaust(lua_DspFaust* dsp)
{
    auto _dsp = static_cast<MfxDspFaust*>(dsp);
    delete _dsp;
}

void lua_stopDspfaust(lua_DspFaust* dsp)
{
    auto _dsp = static_cast<MfxDspFaust*>(dsp);
    _dsp->stop();
    delete _dsp;
    dsp = nullptr;
}

void lua_buildCLuaInterface(lua_DspFaust* dsp, CLuaUI* lua_struct)
{
  static_cast<MfxDspFaust*>(dsp)->buildCLuaInterface(lua_struct);
}

void lua_setRingbuffer(lua_DspFaust* dsp, mfx_ringbuffer_t* rb)
{
  auto _dsp = static_cast<MfxDspFaust*>(dsp);
  if (_dsp)
  {
    if (_dsp->display)
    {
        if (rb)
        {
            _dsp->display->rb = rb;
        }
    }
  }
}

}