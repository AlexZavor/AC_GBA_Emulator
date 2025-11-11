#include "game.h"
#include "fcntl.h"
#include "unistd.h"
#include "stdio.h"
#ifdef WIN32 //For creating save folder
    #include "windows.h"
#else
    #include "sys/stat.h" 
#endif

static bool checkSave(char* name){
    char saveFile[300];
    sprintf(saveFile, "%s%s.SAV", SAVE_DIR, name);
    int fd = open(saveFile, O_RDONLY);
    if(fd > 0){
        close(fd);
        return true;
    }
    return false;
}

void loadDir(game* game_data, game_holder* Roms, const char* directory){
	tinydir_dir dir;
    tinydir_open(&dir, directory);

    while (dir.has_next)
    {
        if(Roms->length > ROMS_MAX){break;}
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if((file.name)[0] != '.'){
			int i = 0;
            if(file.is_dir){
				// Directory.
                char next_dir[512];
                sprintf(next_dir, "%s%s/", directory, file.name);
                loadDir(game_data, Roms, next_dir);
                tinydir_next(&dir);
                continue;
			}
			while (file.name[i] != '\0')
			{
				i++;
			}
            game* g = &game_data[Roms->length];
            g->name = (char*)malloc(strlen(file.name)+1);
            g->location = (char*)malloc(512);
            strcpy(g->name, file.name);
            strcpy(g->location, directory);
			if(file.name[i-1] == 'b' && file.name[i-2] == 'g' && file.name[i-3] == '.'){
                g->system = GB;
                g->has_save = checkSave(file.name);
                Roms->games[Roms->length] = g;
                Roms->length++;
			}
			else if(file.name[i-1] == 'c' && file.name[i-2] == 'b' && file.name[i-3] == 'g' && file.name[i-4] == '.'){
                g->system = GBC;
                g->has_save = checkSave(file.name);
                Roms->games[Roms->length] = g;
                Roms->length++;
			}
			else if(file.name[i-1] == 'a' && file.name[i-2] == 'b' && file.name[i-3] == 'g' && file.name[i-4] == '.'){
                g->system = GBA;
                g->has_save = checkSave(file.name);
                Roms->games[Roms->length] = g;
                Roms->length++;
			}
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);
}

// Load All Games from Global "GAME_DIR" Folder into Roms list
void game_loadGames(game_holder* Roms){
    // If Rom and Save dir doesn't exist yet.
    #ifdef WIN32
    CreateDirectory(GAME_DIR, NULL);
    CreateDirectory(SAVE_DIR, NULL);
    #else
    mkdir(GAME_DIR, 0777); 
    mkdir(SAVE_DIR, 0777);
    #endif

    // data arena for games (doesn't really get freed. but thats probably fine)
    game* game_data = (game*)calloc(256, sizeof(game));
    Roms->length = 0;

    loadDir(game_data, Roms, GAME_DIR); // Recursive for file trees.
}

void game_removeSave(game* game){
    char saveFile[300] = SAVE_DIR;
    strcat(saveFile, game->name);
    strcat(saveFile, ".SAV");
    if(!remove(saveFile)){
        game->has_save = false;
    }

}