# southpole-vcvrack

![All](./doc/sp-all-2018-02-06.png)

Southpole VCV Rack Modules 0.5.2

A personal collection of modules I've always wanted for my workflow.

Some of the modules are simply reskins of the existing Audible Instruments /
Arable Instruments VCV Rack versions of Mutable Instruments eurorack modules.

- The skins are inspired by hardware micro versions of various MI modules.
- Some skins are additonal modes of firmware with appropriate labels for knobs and ports to make usage easier.
- Some skins do not exist yet in hardware.
- Some modules did not yet exist in VCV Rack versions.
- Some modules are new.
- Compiled agaist parasites firmware where available (Required a few fixes which is why it is checked in here)

[Mutable Instruments](https://mutable-instruments.net/)

[Audible Instruments](https://github.com/VCVRack/AudibleInstruments/)

[ArableInstruments](https://github.com/adbrant/ArableInstruments/)

## Building

Compile Rack from source, following the instructions at https://github.com/VCVRack/Rack.

After checking out in the `plugins/` directory, get external dependencies with

	git submodule update --init --recursive

Build main modules

	make dist
	
Build parasite based modules (only Smoke, for now not included in binary release)

	make clean
	make -f Makefile.parasites dist
	cp -r dist/Southpole-parasites .. #assuming .. is your plugins directory

## Modules copied from Audible Instruments / Arable Instruments

### Cornrows - Macro Oscillator

- Based on [Braids](https://mutable-instruments.net/modules/braids), [Manual](https://mutable-instruments.net/modules/braids/manual/)

### Splash - Tidal Modulator / Lambs - Wavetable Oscillator

- Based on [Tides](https://mutable-instruments.net/modules/tides), [Manual](https://mutable-instruments.net/modules/tides/manual/)

- Based on [Sheep](https://mutable-instruments.net/modules/tides/firmware/) (Tides alternative firmware)

- Uses eurorack firmware, no parasites interface yet

### Humo - Texture Synthesizer
Based on [Clouds](https://mutable-instruments.net/modules/clouds), [Manual](https://mutable-instruments.net/modules/clouds/manual/)

- Humo: eurorack firmware, extra skins for additional modes (see image above)
- Smoke: parasites firmware (Neil) - need to compile yourself for now

### Annuli - Resonator
- Based on [Rings](https://mutable-instruments.net/modules/rings), [Manual](https://mutable-instruments.net/modules/rings/manual/)

### Bandana - Quad VC-polarizer
- Based on [Blinds](https://mutable-instruments.net/modules/blinds), [Manual](https://mutable-instruments.net/modules/blinds/manual/)

### Balaclava - Quad VCA
- Based on [Veils](https://mutable-instruments.net/modules/veils), [Manual](https://mutable-instruments.net/modules/veils/manual/)

## New modules

### Etagère - EQ Filter
Inspired by [Shelves](https://mutable-instruments.net/modules/shelves), [Manual](https://mutable-instruments.net/modules/shelves/manual/)

- Does not fully reproduce all characteristics of Shelves!
- Biquad filters copied from RJModules Filters

### But - Manual A/B Buss 

- Inspired by [DJ Steevio's modular method](https://www.youtube.com/watch?v=x6hJa2lRRgM)
- Either input A or B go to the output and the A/B busses
- Two summed identical outputs for A and B buss are provided.

### Abr - Manual A/B Switch 

- Inverse of But, either input A or B goes to output
- Sums of inputs set to A or B provided

### SNS - Euclidean Sequencer

- I know. but this one looks cool and has a few additional features:
- Up to 32 steps
- Accents: nested euclidean sequence on main sequence
- Rotate sequence and accents
- Up to 32 pads (silent steps)
- All inputs scaled to each other in what i think is a sensible way - great for LFO modulation of inputs

### Piste - Drum Processor

- One-stop module to turn oscillators into drums / percussion, or process existing drums to taste
- Inspired by [Bastl Skis](http://www.bastl-instruments.com/modular/skis/)
- Input gain + eq + overdrive because sometimes basic processing is all that's needed
- 2 independent decay envelopes (eg. body and accent) the sum of which is applied to input signal via internal VCA
- 2nd envelope is scaled to 1st
- Triggers are scalable
- Mute input / trigger veto intended for choke groups (eg. open hihat mutes closed hihat)

### Fuse - Synchronized one-shot triggers

- Intended to build a "queue next pattern" system like on groove boxes / drum machines
- Helps fire synchronized events in 4/4 based music if you can't count like me
- Can be used to launch triggers (or beat-long gates) ONCE at a given beat on a 16 step counter
- Arm triggers/gates manually or via inputs
- For example, clock with bars, trigger to mute the kick for exactly 4 bars, trigger 4 bar later to drop

### Wriggle - Spring Model

- Follows input with a given stiffness and damping of the spring
- Mass normalized to one
- Output can be scaled/offset
- Designed to implement "Woggle CV" (works best at LFO rates)

### Sssh - Noise / S+H

- 4x noise / S+H - inspired by the lowest section from [Kinks](https://mutable-instruments.net/modules/kinks/)
- Triggers are normalled from top to bottom
- Inputs are normalled to noise
- Noises are four calls to generator (not a copy)

### Snake

- Internal buss to bridge large distances or generally tidy up the patch
- First connection to input locks input (green led - red on all other instances)
- Works best with slow signal. In case of parallel processin check difference between original and bussed signal by means of inverting summer and scope

### Gnome - Synth Voice

- An all-in-one module inspired by [MFB Nanozwerg](http://mfberlin.de/en/modules/nanozwerg_pro_e/)
- Because sometimes you just need the basics and want to get started fast
- The submodules are mostly copied from the Fundamental modules with some omissions and additions:
- VCO: analog part of Fundamental VCO, morphing
- Sub-OSC 1/2 Oct / Noise
- LFO from Fundamental LFO, morphing + S+H
- ADSR from Fundamental ADSR
- Biquad VCF

### Blanks

- Nothing special right, but watch this space
- 42HP useful if you want to recreate a 84HP row common in hardware