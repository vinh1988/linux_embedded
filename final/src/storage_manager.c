#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sqlite3.h>
#include "../include/shared.h"
#include "../include/sensor_data.h"

#define DB_NAME "sensor_data.db"
#define MAX_RETRY_ATTEMPTS 3
#define RETRY_DELAY 5 /* seconds */

extern shared_data_t shared_data;
extern int fifo_fd;
extern volatile int running;

/* Storage manager thread function */
void *storage_manager_thread(void *arg) {
    /* Unused parameter */
    (void)arg;

    sqlite3 *db = NULL;
    int retry_count = 0;
    int connected = 0;

    printf("Storage manager thread started\n");

    /* Main loop */
    while (running) {
        /* Try to connect to database if not connected */
        if (!connected) {
            int rc = sqlite3_open(DB_NAME, &db);
            if (rc != SQLITE_OK) {
                /* Connection failed */
                retry_count++;

                /* Log event */
                char msg[100];
                sprintf(msg, "Unable to connect to SQL server (attempt %d of %d)",
                        retry_count, MAX_RETRY_ATTEMPTS);
                write_log_event(fifo_fd, LOG_SQL_CONNECTION_FAILED, 0, 0.0, msg);

                /* Check if max retry attempts reached */
                if (retry_count >= MAX_RETRY_ATTEMPTS) {
                    printf("Failed to connect to database after %d attempts, exiting\n",
                           MAX_RETRY_ATTEMPTS);
                    running = 0;
                    break;
                }

                /* Wait before retrying */
                sleep(RETRY_DELAY);
                continue;
            }

            /* Connection successful */
            connected = 1;
            retry_count = 0;

            /* Log event */
            write_log_event(fifo_fd, LOG_SQL_CONNECTED, 0, 0.0,
                           "Connection to SQL server established");

            /* Create table if it doesn't exist */
            char *create_table_sql =
                "CREATE TABLE IF NOT EXISTS sensor_data ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "node_id INTEGER NOT NULL,"
                "temperature REAL NOT NULL,"
                "humidity REAL NOT NULL,"
                "light REAL NOT NULL,"
                "timestamp INTEGER NOT NULL"
                ");";

            char *err_msg = NULL;
            rc = sqlite3_exec(db, create_table_sql, NULL, NULL, &err_msg);
            if (rc != SQLITE_OK) {
                printf("SQL error: %s\n", err_msg);
                sqlite3_free(err_msg);
                sqlite3_close(db);
                connected = 0;
                continue;
            }

            /* Log event */
            write_log_event(fifo_fd, LOG_SQL_TABLE_CREATED, 0, 0.0,
                           "New table sensor_data created");
        }

        /* Sleep to avoid busy waiting */
        usleep(500000); /* 500ms */

        /* Lock shared data */
        pthread_mutex_lock(&shared_data.mutex);

        /* Process each sensor node's data */
        for (int i = 0; i < shared_data.count; i++) {
            sensor_data_t *sensor = &shared_data.data[i];

            /* Prepare SQL statement */
            sqlite3_stmt *stmt;
            const char *sql = "INSERT INTO sensor_data (node_id, temperature, humidity, light, timestamp) "
                             "VALUES (?, ?, ?, ?, ?);";

            int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
                connected = 0;

                /* Log event */
                write_log_event(fifo_fd, LOG_SQL_CONNECTION_LOST, 0, 0.0,
                               "Connection to SQL server lost");

                break;
            }

            /* Bind values */
            sqlite3_bind_int(stmt, 1, sensor->node_id);
            sqlite3_bind_double(stmt, 2, sensor->temperature / 10.0); /* Convert to degrees */
            sqlite3_bind_double(stmt, 3, sensor->humidity / 10.0);    /* Convert to percentage */
            sqlite3_bind_double(stmt, 4, sensor->light);
            sqlite3_bind_int64(stmt, 5, sensor->timestamp);

            /* Execute statement */
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
                connected = 0;

                /* Log event */
                write_log_event(fifo_fd, LOG_SQL_CONNECTION_LOST, 0, 0.0,
                               "Connection to SQL server lost");

                sqlite3_finalize(stmt);
                break;
            }

            /* Finalize statement */
            sqlite3_finalize(stmt);
        }

        /* Unlock shared data */
        pthread_mutex_unlock(&shared_data.mutex);

        /* Close database if connection lost */
        if (!connected && db) {
            sqlite3_close(db);
            db = NULL;
        }
    }

    /* Clean up */
    if (db) {
        sqlite3_close(db);
    }

    printf("Storage manager thread exiting\n");
    return NULL;
}
