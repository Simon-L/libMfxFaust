#include <stdio.h>
#include <unistd.h>
typedef void lua_DspFaust;
lua_DspFaust* lua_newDspfaust(const char * file, char * error_msg, int sample_rate, int buffer_size, int argc, const char* argv[]);
void lua_startDspfaust(lua_DspFaust* dsp);
void lua_stopDspfaust(lua_DspFaust* dsp);
// void lua_buildCLuaInterface(lua_DspFaust* dsp, CLuaUI* lua_struct);
float* lua_getDspMemory(lua_DspFaust* dsp);
// void lua_setRingbuffer(lua_DspFaust* dsp, mfx_ringbuffer_t* rb);
void lua_deleteDspfaust(lua_DspFaust* dsp);
void printVersionAndTarget();
int openDspDialog(char* path);

int main(int argc, char* argv[])
{
    printVersionAndTarget();
    char error_msg[2048];
    const char* faust_argv[2] = {
        "-I",
        "/usr/local/share/faust"
    };
    lua_DspFaust* dsp = lua_newDspfaust(argv[1], error_msg, 48000, 512, 2, faust_argv);
    lua_startDspfaust(dsp);
    getchar();
    sleep(2);
    lua_stopDspfaust(dsp);
    sleep(2);
    dsp = lua_newDspfaust(argv[1], error_msg, 48000, 512, 2, faust_argv);
    lua_startDspfaust(dsp);
    sleep(2);
    // getchar();
    lua_stopDspfaust(dsp);

    // char path[2048];
    // if (!openDspDialog(path)) {
    //     printf("Result: %s\n", path);
    // }
    return 0;
}