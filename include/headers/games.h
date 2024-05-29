#ifndef GAMES_H
#define GAMES_H

#include "tinydir.h"
#include "globals.h"
#include <vector>
#include <string>


typedef struct {
    GB_sys system;
    std::string name;
} game;

void games_loadGames(std::vector<game>* Roms){
	tinydir_dir dir;
    tinydir_open(&dir, "./ROMS/");

    while (dir.has_next)
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if((file.name)[0] != '.'){
			uint8_t i = 0;
            game g;
            g.name = file.name;
			while (file.name[i] != '\0')
			{
				i++;
			}
			if(file.name[i-1] == 'b'){
                g.system = GB;
				Roms->push_back(g);
			}
			else if(file.name[i-1] == 'c'){
                g.system = GBC;
				Roms->push_back(g);
			}
			else if(file.name[i-1] == 'a'){
                g.system = GBA;
				Roms->push_back(g);
			}
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);

}

#endif