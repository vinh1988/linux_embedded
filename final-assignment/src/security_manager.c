#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "../include/shared.h"

/* Global variables */
extern int fifo_fd;
extern volatile int running;

/* Local variables */
static pthread_mutex_t blacklist_mutex = PTHREAD_MUTEX_INITIALIZER;
static char **blacklisted_ips = NULL;
static int blacklist_count = 0;
static int blacklist_capacity = 0;

/* Forward declarations */
static int add_to_blacklist(const char *ip_address);
static int load_blacklist();
static int save_blacklist();

/* Initialize the security manager */
int init_security_manager() {
    /* Load blacklist from file */
    if (load_blacklist() != 0) {
        printf("Failed to load blacklist, starting with empty list\n");
    }

    return 0;
}

/* Clean up the security manager */
void cleanup_security_manager() {
    /* Save blacklist to file */
    save_blacklist();

    /* Free blacklist memory */
    pthread_mutex_lock(&blacklist_mutex);

    if (blacklisted_ips) {
        for (int i = 0; i < blacklist_count; i++) {
            if (blacklisted_ips[i]) {
                free(blacklisted_ips[i]);
            }
        }
        free(blacklisted_ips);
        blacklisted_ips = NULL;
    }

    blacklist_count = 0;
    blacklist_capacity = 0;

    pthread_mutex_unlock(&blacklist_mutex);

    /* Destroy mutex */
    pthread_mutex_destroy(&blacklist_mutex);
}

/* Check if an IP is blacklisted */
bool is_ip_blacklisted(const char *ip_address) {
    if (!ip_address) return false;

    pthread_mutex_lock(&blacklist_mutex);

    bool result = false;
    for (int i = 0; i < blacklist_count; i++) {
        if (blacklisted_ips[i] && strcmp(blacklisted_ips[i], ip_address) == 0) {
            result = true;
            break;
        }
    }

    pthread_mutex_unlock(&blacklist_mutex);

    return result;
}

/* Add an IP to the blacklist */
void add_ip_to_blacklist(const char *ip_address) {
    if (!ip_address) return;

    /* Check if already blacklisted */
    if (is_ip_blacklisted(ip_address)) {
        return;
    }

    /* Add to blacklist */
    if (add_to_blacklist(ip_address) == 0) {
        /* Log event */
        char msg[100];
        sprintf(msg, "IP %s added to blacklist", ip_address);
        write_log_event(fifo_fd, LOG_SECURITY_VIOLATION, 0, 0.0, msg);

        /* Save blacklist to file */
        save_blacklist();
    }
}

/* Maximum allowed authentication failures before blacklisting */
#define MAX_AUTH_FAILURES 3

/* Authenticate a connection */
bool authenticate_connection(connection_session_t *session, const char *username, const char *password) {
    if (!session || !username || !password) return false;

    /* In a real implementation, this would check credentials against a database */
    /* For simplicity, we'll accept any non-empty username and password */
    bool authenticated = (strlen(username) > 0 && strlen(password) > 0);

    /* Update session authentication state */
    pthread_mutex_lock(&session->mutex);

    if (authenticated) {
        session->auth_state = AUTH_STATE_SUCCESS;
        strncpy(session->username, username, MAX_USERNAME_LENGTH - 1);
        session->username[MAX_USERNAME_LENGTH - 1] = '\0';
        /* Reset auth failures on successful authentication */
        session->auth_failures = 0;
    } else {
        session->auth_state = AUTH_STATE_FAILED;
        /* Increment auth failures counter */
        session->auth_failures++;
    }

    /* Check if we need to blacklist this IP */
    uint32_t failures = session->auth_failures;
    char ip_copy[INET_ADDRSTRLEN];
    strncpy(ip_copy, session->ip_str, INET_ADDRSTRLEN);
    ip_copy[INET_ADDRSTRLEN - 1] = '\0';

    pthread_mutex_unlock(&session->mutex);

    /* Log authentication result */
    char msg[100];
    if (authenticated) {
        sprintf(msg, "Authentication successful for user '%s' from %s:%d",
                username, session->ip_str, session->port);
        write_log_event(fifo_fd, LOG_AUTH_SUCCESS, 0, 0.0, msg);
    } else {
        sprintf(msg, "Authentication failed for user '%s' from %s:%d (Failures: %u/%u)",
                username, session->ip_str, session->port, failures, MAX_AUTH_FAILURES);
        write_log_event(fifo_fd, LOG_AUTH_FAILED, 0, 0.0, msg);

        /* After multiple failures, blacklist the IP */
        if (failures >= MAX_AUTH_FAILURES) {
            sprintf(msg, "IP %s blacklisted after %u authentication failures",
                    ip_copy, failures);
            write_log_event(fifo_fd, LOG_SECURITY_VIOLATION, 0, 0.0, msg);
            add_ip_to_blacklist(ip_copy);
        }
    }

    return authenticated;
}

/* Add an IP to the blacklist (internal function) */
static int add_to_blacklist(const char *ip_address) {
    pthread_mutex_lock(&blacklist_mutex);

    /* Check if we need to allocate or resize the array */
    if (!blacklisted_ips || blacklist_count >= blacklist_capacity) {
        int new_capacity = blacklist_capacity == 0 ? 10 : blacklist_capacity * 2;
        char **new_ips = (char **)realloc(blacklisted_ips, new_capacity * sizeof(char *));

        if (!new_ips) {
            pthread_mutex_unlock(&blacklist_mutex);
            return -1;
        }

        blacklisted_ips = new_ips;
        blacklist_capacity = new_capacity;
    }

    /* Add the IP to the list */
    blacklisted_ips[blacklist_count] = strdup(ip_address);
    if (!blacklisted_ips[blacklist_count]) {
        pthread_mutex_unlock(&blacklist_mutex);
        return -1;
    }

    blacklist_count++;

    pthread_mutex_unlock(&blacklist_mutex);
    return 0;
}

/* Load blacklist from file */
static int load_blacklist() {
    FILE *file = fopen("blacklist.txt", "r");
    if (!file) {
        return -1;
    }

    char line[INET_ADDRSTRLEN + 1];

    pthread_mutex_lock(&blacklist_mutex);

    /* Free existing blacklist */
    if (blacklisted_ips) {
        for (int i = 0; i < blacklist_count; i++) {
            if (blacklisted_ips[i]) {
                free(blacklisted_ips[i]);
            }
        }
        free(blacklisted_ips);
        blacklisted_ips = NULL;
    }

    blacklist_count = 0;
    blacklist_capacity = 10;

    /* Allocate initial array */
    blacklisted_ips = (char **)malloc(blacklist_capacity * sizeof(char *));
    if (!blacklisted_ips) {
        pthread_mutex_unlock(&blacklist_mutex);
        fclose(file);
        return -1;
    }

    /* Read IPs from file */
    while (fgets(line, sizeof(line), file)) {
        /* Remove newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        /* Skip empty lines */
        if (strlen(line) == 0) {
            continue;
        }

        /* Check if we need to resize the array */
        if (blacklist_count >= blacklist_capacity) {
            int new_capacity = blacklist_capacity * 2;
            char **new_ips = (char **)realloc(blacklisted_ips, new_capacity * sizeof(char *));

            if (!new_ips) {
                pthread_mutex_unlock(&blacklist_mutex);
                fclose(file);
                return -1;
            }

            blacklisted_ips = new_ips;
            blacklist_capacity = new_capacity;
        }

        /* Add the IP to the list */
        blacklisted_ips[blacklist_count] = strdup(line);
        if (!blacklisted_ips[blacklist_count]) {
            pthread_mutex_unlock(&blacklist_mutex);
            fclose(file);
            return -1;
        }

        blacklist_count++;
    }

    pthread_mutex_unlock(&blacklist_mutex);
    fclose(file);

    printf("Loaded %d blacklisted IPs\n", blacklist_count);
    return 0;
}

/* Save blacklist to file */
static int save_blacklist() {
    FILE *file = fopen("blacklist.txt", "w");
    if (!file) {
        return -1;
    }

    pthread_mutex_lock(&blacklist_mutex);

    for (int i = 0; i < blacklist_count; i++) {
        if (blacklisted_ips[i]) {
            fprintf(file, "%s\n", blacklisted_ips[i]);
        }
    }

    pthread_mutex_unlock(&blacklist_mutex);

    fclose(file);
    return 0;
}
