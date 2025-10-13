#include "games.h"

// Load All Games from Global "GAME_DIR" Folder into Roms list
void games_loadGames(std::vector<game>* Roms){
	tinydir_dir dir;
    tinydir_open(&dir, GAME_DIR);

    while (dir.has_next)
    {
        // TODO: Put limit on number of games loaded. perhaps return a error code?
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if((file.name)[0] != '.'){
			int i = 0;
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
			else if(file.is_dir){
				// Directory. 
                // TODO: Directory Trees
                printf("directory: %s\n", file.name);
			}
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);

    // TODO: Check for save files

}