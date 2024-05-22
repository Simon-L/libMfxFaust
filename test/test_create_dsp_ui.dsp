import("stdfaust.lib");

process = attach(13.12,
  os.lf_saw(4) : hbargraph("foobarbaz",0,1));