/*
 * GNU GPLv3 or later
 * Copyright (C) 2025 Lukas Schmid
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <libgen.h>
#include <linux/limits.h>
#include <errno.h>
#include <sys/mount.h>

void printHelp(char *programName) {
    fprintf(stderr, "MakeRW v1.1 ("__DATE__" "__TIME__")\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage: %s [-uv] [-t DIR] DIRECTORY\n", programName);
    fprintf(stderr, "\n");
    fprintf(stderr, "Make DIRECTORY writeable by copying it to a writeable location\n");
    fprintf(stderr, "and binding it to the original location\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "        -t DIR  Use DIR instead of /tmp as the writable directory\n");
    fprintf(stderr, "        -u      Unmount the directory\n");
    fprintf(stderr, "        -v      Verbose\n");
}

int main(int argc, char *argv[]) {
    int opt;
    bool verbose = false;
    bool unmount = false;
    char *tmpDir = "/tmp";
    char directory[PATH_MAX];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printHelp(argv[0]);
            return 0;
        }
    }

    while ((opt = getopt(argc, argv, "t:uv")) != -1) {
        switch (opt) {
            case 't':
                tmpDir = optarg;
                break;
            case 'u':
                unmount = true;
                break;
            case 'v':
                verbose = true;
                break;
            default:
                printHelp(argv[0]);
                return 1;
        }
    }

    if (optind >= argc) {
        printHelp(argv[0]);
        return 1;
    }

    if (realpath(argv[optind], directory) == NULL) {
        fprintf(stderr, "%s: Failed to resolve path %s: %s\n", argv[0], argv[optind], strerror(errno));
        return 1;
    }

    // Check if the directory exists
    if (access(directory, F_OK) != 0) {
        fprintf(stderr, "%s: %s: No such file or directory\n", argv[0], directory);
        return 1;
    }

    // Unmount the directory if requested
    if (unmount) {
        if (umount2(directory, MNT_DETACH) != 0) {
            fprintf(stderr, "%s: Failed to unmount %s: %s\n", argv[0], directory, strerror(errno));
            return 1;
        }

        if (verbose)
            printf("unmounted '%s'\n", directory);

        return 0;
    }

    // Check if the directory is already mounted
    FILE *mountsFile = fopen("/proc/mounts", "r");
    if (mountsFile == NULL) {
        fprintf(stderr, "%s: /proc/mounts: %s\n", argv[0], strerror(errno));
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), mountsFile)) {
        char mountPoint[256];
        sscanf(line, "%*s %255s", mountPoint);
        if (strcmp(mountPoint, directory) == 0) {
            fprintf(stderr, "%s: %s: already mounted\n", argv[0], directory);
            fclose(mountsFile);
            return 1;
        }
    }

    fclose(mountsFile);

    // Create a new folder in tmpDir
    char template[PATH_MAX];
    snprintf(template, sizeof(template), "%s/%s-XXXXXX", tmpDir, basename(directory));
    char *newFolder = mkdtemp(template);
    if (newFolder == NULL) {
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        return 1;
    }

    // Delete the new folder in order for cp to create it
    if (rmdir(newFolder) != 0) {
        fprintf(stderr, "%s: Failed to rmdir %s: %s\n", argv[0], newFolder, strerror(errno));
        return 1;
    }

    // Copy the directory to the new folder
    char *copyCommand = malloc(strlen(directory) + strlen(newFolder) + 16);
    sprintf(copyCommand, "cp -a %s %s", directory, newFolder);
    if (system(copyCommand) != 0) {
        fprintf(stderr, "%s: Failed to copy %s to %s\n", argv[0], directory, newFolder);
        free(copyCommand);
        return 1;
    }
    free(copyCommand);

    // Bind the new folder to the original directory
    if (mount(newFolder, directory, NULL, MS_BIND, NULL) != 0) {
        fprintf(stderr, "%s: Failed to bind %s to %s: %s\n", argv[0], newFolder, directory, strerror(errno));
        return 1;
    }

    // Print success message
    if (verbose)
        printf("'%s' -> '%s'\n", newFolder, directory);

    return 0;
}
