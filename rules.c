#include <stdio.h>
#include <string.h>
#include "rules.h"
#include "features.h"
#include "database.h"

char* classify_mood(SongNode *s) {
    if (s->bpm < 80 && s->energy < 50) strcpy(s->mood, "Sad");
    else if (s->bpm < 80 && s->energy >= 50) strcpy(s->mood, "Chill");
    else if (s->bpm <= 110 && s->energy < 60) strcpy(s->mood, "Calm");
    else if (s->bpm <= 110 && s->energy >= 60) strcpy(s->mood, "Happy");
    else if (s->bpm <= 140 && s->energy < 70) strcpy(s->mood, "Romantic");
    else if (s->bpm <= 140 && s->energy >= 70) strcpy(s->mood, "Energetic");
    else if (s->bpm > 140 && s->energy < 70) strcpy(s->mood, "Tense");
    else strcpy(s->mood, "Highly Energetic");

    return s->mood;
}

void classify_all_songs() {
    SongNode *temp = head;
    if (!temp) { printf("No songs to classify.\n"); return; }

    while (temp) {
        classify_mood(temp);
        temp = temp->next;
    }
    printf("All songs classified successfully!\n");
}
