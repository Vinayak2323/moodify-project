#include <stdio.h>
#include <string.h>
#include "auth.h"
#include "utils.h"

typedef struct {
    char username[50];
    char password[50];
} User;

extern char logUser[50]; 

void registerUser() {
    FILE *fp = fopen("users.txt", "a+");
    if (fp == NULL) {
        printf("Error: Could not open users database.\n");
        return;
    }
    User u, existingUser;
    int flag = 0; 

    printf("Enter new username: ");
    scanf("%s", u.username);
    printf("Enter new password: ");
    scanf("%s", u.password);

     while (fscanf(fp, "%s", existingUser.username) != EOF) {
        if (strcmp(u.username, existingUser.username) == 0 ) {
            flag = 1;
            break;
        }
    }

    if (flag) {
        printf("User already registered. Please login.\n");
    } else {
        fprintf(fp, "%s %s\n", u.username, u.password);
        printf("Registration successful.\n\n");
    }

    fclose(fp);
}

int login() {
    FILE *fp = fopen("users.txt", "r");
    char user[50], pass[50];
    User u;
    int found = 0;
    printf("Enter username: ");
    scanf("%s", user);
    changeCase(user); 
    printf("Enter password: ");
    scanf("%s", pass);
    while (fscanf(fp, "%s %s", u.username, u.password) != EOF) {
        if (strcmp(user, u.username) == 0 && strcmp(pass, u.password) == 0) {
            found = 1;
            break;
        }
    }
    fclose(fp);
    if (found){
         printf("Login successful.\n\n");
            strcpy(logUser, user); 
    }
    else printf("Login failed.\n\n");
    return found;
}