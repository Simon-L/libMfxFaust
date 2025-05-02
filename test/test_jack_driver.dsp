declare options "[midi:on][nvoices:12]";
import("stdfaust.lib");

gate = button("gate");

process = os.osc(hslider("osc1",440,10,10000,1)) * en.ar(10e-3, 130e-3, gate);