#include "games.h"

bool checkSave(char* name){
    char saveFile[300] = SAVE_DIR;
    strcat(saveFile, name);
    strcat(saveFile, ".SAV");
    if(fopen(saveFile, "r") != NULL){
        return true;
    }
    return false;
}

void loadDir(std::vector<game>* Roms, char* directory){
	tinydir_dir dir;
    tinydir_open(&dir, directory);

    while (dir.has_next)
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if((file.name)[0] != '.'){
			int i = 0;
            game g;
            g.name = file.name;
            g.has_save = false;
            if(file.is_dir){
				// Directory.
                loadDir(Roms, (std::string(directory) + file.name + "/").data());
                tinydir_next(&dir);
                continue;
			}
			while (file.name[i] != '\0')
			{
				i++;
			}
			if(file.name[i-1] == 'b' && file.name[i-2] == 'g' && file.name[i-3] == '.'){
                g.system = GB;
                g.has_save = checkSave(file.name);
                Roms->push_back(g);
			}
			else if(file.name[i-1] == 'c' && file.name[i-2] == 'b' && file.name[i-3] == 'g' && file.name[i-4] == '.'){
                g.system = GBC;
				Roms->push_back(g);
			}
			else if(file.name[i-1] == 'a' && file.name[i-2] == 'b' && file.name[i-3] == 'g' && file.name[i-4] == '.'){
                g.system = GBA;
				Roms->push_back(g);
			}
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);
}

// Load All Games from Global "GAME_DIR" Folder into Roms list
void games_loadGames(std::vector<game>* Roms){
    mkdir(GAME_DIR, 0777); // If ./ROMS/ doesn't exist yet.
    loadDir(Roms, (char*)GAME_DIR); // Recursive for file trees.
}

void games_removeSave(game* game){
    char saveFile[300] = SAVE_DIR;
    strcat(saveFile, game->name.data());
    strcat(saveFile, ".SAV");
    if(!remove(saveFile)){
        game->has_save = false;
    }

}