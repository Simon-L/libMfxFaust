package.path = "./?.lua;../share/lua/5.1/?.lua;../share/lua/5.1/?/init.lua"
if (jit.os ~= "Windows") then
    package.cpath = "./?.so;../lib/lua/5.1/?.so;../lib/lua/5.1/loadall.so"
else
    package.cpath = "./?.dll;../lib/lua/5.1/?.dll;../lib/lua/5.1/loadall.dll"
end

-- local sys_stat = require "posix.sys.stat"
local inspect = require "inspect"
local socket  = require "socket"

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
lua_DspFaust* lua_newDspfaust(const char * file, char * error_msg);
void lua_startDspfaust(lua_DspFaust* dsp);
void lua_stopDspfaust(lua_DspFaust* dsp);
void lua_buildCLuaInterface(lua_DspFaust* dsp, CLuaUI* lua_struct);
float* lua_getDspMemory(lua_DspFaust* dsp);
void printVersionAndTarget();
void lua_deleteDspfaust(lua_DspFaust* dsp);
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

local MfxFaustLib = ffi.load(arg[1])
MfxFaustLib.printVersionAndTarget()

local error_msg = ffi.new("char[2048]")
local dsp = MfxFaustLib.lua_newDspfaust(arg[2], error_msg)
MfxFaustLib.lua_buildCLuaInterface(dsp, faust_ui)
print(inspect(faust_ui_tbl))
MfxFaustLib.lua_deleteDspfaust(dsp)

