#ifndef FILE_IO_H
#define FILE_IO_H

#include "features.h"
#include "database.h"

extern SongNode *head;

// Updated function prototypes (no filename argument)
void load_songs();
void save_songs();
void print_songs();

#endif
