# AC_GBA_Emulator
Game Boy Emulator made by Alex Carter. A passion project and love letter 
to the Game boy series of consoles. 

don't expect it to run great on every computer.


## Build Directions
### Windows
```powershell
cd ./build
cmake .. -G "MinGW Makefiles"
make
```

Uses MinGW to compile, then run `GBA.exe`. 

Roms go in ./build/ROMS/

and Saves will be in ./build/SAVES/

### Linux
```bash
cd ./build
cmake ..
make
```

run `./GBA`. 

Roms go in ./build/ROMS/

and Saves will be in ./build/SAVES/


<img src="./res/Full_Transparent_Advance.png" alt="Icon" width="300"/>
