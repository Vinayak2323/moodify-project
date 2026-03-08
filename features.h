#ifndef FEATURES_H
#define FEATURES_H

#define MAX_NAME_LEN 50
#define MAX_TITLE_LEN 50
#define MAX_ARTIST_LEN 50
#define MAX_MOOD_LEN 25

typedef struct SongNode {   
    char title[MAX_TITLE_LEN];   
    char artist[MAX_ARTIST_LEN];
    int bpm;
    int energy;
    int danceability;
    char mood[MAX_MOOD_LEN];
    struct SongNode *next;
    struct SongNode *prev;
} SongNode;

#endif
