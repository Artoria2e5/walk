// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>

static const char *const JUST_CURRENT_DIRECTORY[] = {".", NULL};

// Like readdir(3), but resets errno(3) before the call to make it easy to
// check.
static struct dirent *readdir2(DIR *const dirp)
{
	errno = 0;
	return readdir(dirp);
}

// Like realloc(3), but exits the binary in an out-of-memory situation.
static void *realloc2(void *ptr, const size_t size)
{
	if (!(ptr = realloc(ptr, size))) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}
	return ptr;
}

static void strcpy3(char *dest, const char *s1, const char *s2, const char *s3)
{
	stpcpy(stpcpy(stpcpy(dest, s1), s2), s3);
}

// Walks the directory named dirname, printing the names of all files it
// contains (but not the name of the directory itself). Returns 2 if dirname is
// not a directory and 1 if another error occurs.
static int walk(const char dirname[])
{
	DIR *const dir = opendir(dirname);
	if (!dir) {
		if (errno != ENOTDIR) {
			perror(dirname);
			return 1;
		}
		return 2;
	}
	int r = 0;
	char *filename = NULL;
	for (const struct dirent *f = readdir2(dir); f; f = readdir2(dir)) {
		if (strcmp(f->d_name, ".") == 0
				|| strcmp(f->d_name, "..") == 0)
			continue;
		filename = realloc2(filename,
			strlen(dirname) + 1 + strlen(f->d_name) + 1);
		strcpy3(filename, dirname, "/", f->d_name);
		// TODO(bbaren): Emulate Plan 9's cleanname(3).
		puts(filename);
		// Walk the file if we can successfully open it as a directory.
		// Don't worry about it if it's not one (walk(filename) == 2).
		if ((f->d_type == DT_DIR || f->d_type == DT_UNKNOWN)
				&& walk(filename) == 1)
			r = 1;
	}
	if (errno) {	// from readdir
		perror(dirname);
		r = 1;
	}
	free(filename);
	if (closedir(dir)) {
		perror(dirname);
		r = 1;
	}
	return r;
}

int main(const int argc, const char *const argv[])
{
	int r = 0;
	const char *const *const dirs =
		argc == 1 ? JUST_CURRENT_DIRECTORY : argv + 1;
	for (int i = 0; dirs[i]; ++i) {
		puts(dirs[i]);
		r |= walk(dirs[i]);
	}
	return r;
}