package.path = "./?.lua;../share/lua/5.1/?.lua;../share/lua/5.1/?/init.lua"
if (jit.os ~= "Windows") then
    package.cpath = "./?.so;../lib/lua/5.1/?.so;../lib/lua/5.1/loadall.so"
else
    package.cpath = "./?.dll;../lib/lua/5.1/?.dll;../lib/lua/5.1/loadall.dll"
end

-- local sys_stat = require "posix.sys.stat"
local inspect = require "inspect"
local socket  = require "socket"
local app  = require "pl.app"

function sleep(sec)
    socket.select(nil, nil, sec)
end
-- 
-- 
local ffi = require "ffi"
ffi.cdef[[
typedef void lua_DspFaust;
typedef float FAUSTFLOAT;
typedef void* Soundfile;
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
} CLuaUI;
lua_DspFaust* lua_newDspfaust(const char * file, char * error_msg, int sample_rate, int buffer_size, int argc, const char* argv[]);
void lua_startDspfaust(lua_DspFaust* dsp);
void lua_stopDspfaust(lua_DspFaust* dsp);
void lua_buildCLuaInterface(lua_DspFaust* dsp, CLuaUI* lua_struct);
void printVersionAndTarget();
void lua_deleteDspfaust(lua_DspFaust* dsp);
struct rb_compute_buffers {
  uint8_t channels;
  uint16_t nframes;
  float buffers[16][2048];
};
typedef struct {
    char *buf;
    size_t len;
}
mfx_ringbuffer_data_t;

typedef struct {
    char *buf;
    volatile size_t write_ptr;
    volatile size_t read_ptr;
    size_t	size;
    size_t	size_mask;
    int	mlocked;
}
mfx_ringbuffer_t;

mfx_ringbuffer_t *mfx_ringbuffer_create(size_t sz);
void mfx_ringbuffer_free(mfx_ringbuffer_t *rb);
void mfx_ringbuffer_get_read_vector(const mfx_ringbuffer_t *rb,
                                         mfx_ringbuffer_data_t *vec);
void mfx_ringbuffer_get_write_vector(const mfx_ringbuffer_t *rb,
                                          mfx_ringbuffer_data_t *vec);
size_t mfx_ringbuffer_read(mfx_ringbuffer_t *rb, char *dest, size_t cnt);
size_t mfx_ringbuffer_peek(mfx_ringbuffer_t *rb, char *dest, size_t cnt);
void mfx_ringbuffer_read_advance(mfx_ringbuffer_t *rb, size_t cnt);
size_t mfx_ringbuffer_read_space(const mfx_ringbuffer_t *rb);
int mfx_ringbuffer_mlock(mfx_ringbuffer_t *rb);
void mfx_ringbuffer_reset(mfx_ringbuffer_t *rb);
void mfx_ringbuffer_reset_size (mfx_ringbuffer_t * rb, size_t sz);
size_t mfx_ringbuffer_write(mfx_ringbuffer_t *rb, const char *src,
                                 size_t cnt);
void mfx_ringbuffer_write_advance(mfx_ringbuffer_t *rb, size_t cnt);
size_t mfx_ringbuffer_write_space(const mfx_ringbuffer_t *rb);
void lua_setRingbuffer(lua_DspFaust* dsp, mfx_ringbuffer_t* rb);
int openDspDialog(char* path);
]]

local faust_ui = ffi.new("CLuaUI[1]")
local faust_ui_tbl = {}
faust_ui[0].openTabBox = function(label)
  print("openTabBox", ffi.string(label))
  table.insert(faust_ui_tbl, {type = "openTabBox", label = ffi.string(label)})
end
faust_ui[0].openHorizontalBox = function(label)
  print("openHorizontalBox", ffi.string(label))
  table.insert(faust_ui_tbl, {type = "openHorizontalBox", label = ffi.string(label)})
end
faust_ui[0].openVerticalBox = function(label)
  print("openVerticalBox", ffi.string(label))
  table.insert(faust_ui_tbl, {type = "openVerticalBox", label = ffi.string(label)})
end
faust_ui[0].closeBox = function()
  print("closeBox")
  table.insert(faust_ui_tbl, {type = "closeBox"})
end
faust_ui[0].addButton = function(label, zone)
  print("addButton", ffi.string(label), zone[0])
  table.insert(faust_ui_tbl, {type = "addButton", label = ffi.string(label), pointer = zone})
end
faust_ui[0].addCheckButton = function(label, zone)
  print("addCheckButton", ffi.string(label), zone[0])
  table.insert(faust_ui_tbl, {type = "addCheckButton", label = ffi.string(label), pointer = zone})
end
faust_ui[0].addHorizontalSlider = function(label, zone, init, min, max, step)
  print("addHorizontalSlider", ffi.string(label), zone[0], init, min, max, step)
  table.insert(faust_ui_tbl, {type = "addHorizontalSlider", label = ffi.string(label), pointer = zone, init = init, min = min, max = max, step = step})
end
faust_ui[0].addVerticalSlider = function(label, zone, init, min, max, step)
  print("addVerticalSlider", ffi.string(label), zone[0], init, min, max, step)
  table.insert(faust_ui_tbl, {type = "addVerticalSlider", label = ffi.string(label), pointer = zone, init = init, min = min, max = max, step = step})
end
faust_ui[0].addNumEntry = function(label, zone, init, min, max, step)
  print("addNumEntry", ffi.string(label), zone[0], init, min, max, step)
  table.insert(faust_ui_tbl, {type = "addNumEntry", label = ffi.string(label), pointer = zone, init = init, min = min, max = max, step = step})
end
faust_ui[0].addHorizontalBargraph = function(label, zone, min, max)
  print("addHorizontalBargraph", ffi.string(label), zone[0], min, max)
  table.insert(faust_ui_tbl, {type = "addHorizontalBargraph", label = ffi.string(label), pointer = zone, min = min, max = max})
end
faust_ui[0].addVerticalBargraph = function(label, zone, min, max)
  print("addVerticalBargraph", ffi.string(label), zone[0], min, max)
  table.insert(faust_ui_tbl, {type = "addVerticalBargraph", label = ffi.string(label), pointer = zone, min = min, max = max})
end
faust_ui[0].addSoundfile = function(label, soundpath)
  print("addSoundfile", ffi.string(label), ffi.string(soundpath))
  table.insert(faust_ui_tbl, {type = "addSoundfile", label = ffi.string(label), path = ffi.string(soundpath)})
end
faust_ui[0].declare = function(zone, key, val)
  print("declare", ffi.string(key), ffi.string(val))
  table.insert(faust_ui_tbl, {type = "declare", key = ffi.string(key), value = ffi.string(val)})
end

local flags, params = app.parse_args()

if (flags.lib == nil) then
  os.exit(0) -- exit without error
end

local MfxFaustLib = ffi.load(flags.lib)
MfxFaustLib.printVersionAndTarget()

local dsp_path = ffi.new("char[2048]")
MfxFaustLib.openDspDialog(dsp_path)
print('Dsp file path: ' .. ffi.string(dsp_path))
