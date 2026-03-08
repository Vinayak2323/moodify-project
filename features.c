// features.c
#include <stdio.h>
#include <string.h>
#include "features.h"
extern SongNode *head;


SongNode input_song_features() {
    SongNode s;
    printf("Enter song title: ");
    scanf(" %[^\n]", s.title);
    printf("Enter artist: ");
    scanf(" %[^\n]", s.artist);
    printf("Enter BPM: ");
    scanf("%d", &s.bpm);
    printf("Enter Energy (0–1): ");
    scanf("%d", &s.energy);
    printf("Enter Danceability (0–1): ");
    scanf("%d", &s.danceability);
    return s;
}
