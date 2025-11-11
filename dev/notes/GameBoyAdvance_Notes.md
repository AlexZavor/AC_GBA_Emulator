# GBA

### Preface

The Game Boy Advance is my favorite console ever. That is the purpose of this project. I can't say to be an expert or have any more than anyone else to create a better emulator than anyone else. But I want to learn more.

If you share this desire to learn and somehow find your way to this document. I'm impressed. I don't expect anyone to come here. but I will make it regardless. and maybe it will be worth something.

Making an emulator is an interesting problem. hardware documentations isn't usually public, so the best you can usually do is find the programming details and work your way back from there. I'll list all of the resources I use Here for your own reference and mine. Documents found from across the web. test roms I used and their source code. Whatever I use.

And lastly. I wish you and my future self good luck. It's not an easy task, or a very useful one. writing code on your own is becoming less useful as AI takes over. and there are already incredibly well crafted emulators out there already. but believe me, it's worth it to learn and fail rather than have things given to you.

### Resources

<u>GBATEK</u> &emsp; Helpful guide detailing everything needed to program the GBA
- Online: https://problemkaputt.de/gbatek.htm
- MD file: [link](../resources/gba.md)

<u>GBA Programming Manual</u> &emsp; Very similar to GBATEK but official document from nintendo 
- Online: [PDF](https://cdn.preterhuman.net/texts/gaming_and_diversion/Gameboy%20Advance%20Programming%20Manual%20v1.1.pdf)

<u>GBA ARM7 Processor Manual</u> &emsp; Technical manual for the CPU in the GBA from ARM's website
- Online: [PDF](https://documentation-service.arm.com/static/5e8e1323fd977155116a3129?token=)

### Background

[GBA Wikipedia Page](https://en.wikipedia.org/wiki/Game_Boy_Advance)

The Game Boy Advance is a 32-bit handheld game console made by Nintendo in 2001. it has a full color screen and 10 buttons for input. there were different versions released. including the SP with a clamshell design and a backlit screen. It uses an ARM7TDMI processor to read in cartridges and inputs on the controller to play games on the screen.

I think the Game Boy Advance (GBA) is the last of it's kind, preceding the DS line of handhelds that introduced 3d gaming to the handheld realm. It's the last handheld device that only had 2d pixel graphics.

In some ways, the GBA is simpler than its monochrome sibling. Technological advancements allowed more simple solutions to problems such as a small memory range (32 bit address space instead of 16) and compact instructions (16 and 32 bit instructions allow for decode logic rather than individual opcodes)

## Programming

I'll be writing in C. pure and simple. but hopefully I'll be writing this in a way that can be used for any programming language. I just like low level programming.

Following the classic Architecture, I'll start with fetch, decode and execute. To fetch, we need some way to read memory, so that's a good place to start. Along with that we need to figure out where to start code, so it's a good time to read about the cartridge and what it gives for a start location and information.

Then we can put together the CPU, what registers it needs, and start programming the decode and execute phase.

Eventually we'll get stuck in a loop waiting for something else and we can look at what register it's waiting on and figure out what it wants. if it's a line count, or an interrupt or what. and just add support for the hardware that we need as we need it!

### Memory
