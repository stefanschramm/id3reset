/*
 * id3reset.c - id3reset - set id3 tags by directory and file names
 *
 * Copyright (C) 2010, Stefan Schramm <mail@stefanschramm.net>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <taglib/tag_c.h>

int is_nice_dir(struct dirent *d) {
	return ! (d->d_type != DT_DIR ||
		(d->d_name[0] == '.' && (d->d_name[1] == '\0' || (d->d_name[1] == '.' && d->d_name[2] == '\0')))) ? 1 : 0;
}

int is_file(struct dirent *d) {
	return (d->d_type == DT_REG);
}

void chdir_error(char *name) {
	fprintf(stderr, "Unable to chdir to %s.\n", name);
}

void opendir_error(char *name) {
	fprintf(stderr, "Unable to open %s.\n", name);
}

char *cleanup_text(char *old) {
	int len = strlen(old);
	char *new = malloc(len+1);
	int i;
	for (i = 0; i < len; i++) {
		new[i] = (old[i] >= 32 && old[i] < 126) ? old[i] : '_';
	}
	new[i] = '\0';
	return new;
}

int main(int argc, char **argv) {

	DIR *artists, *albums, *tracks;
	struct dirent *artist, *album, *track;
	char *artist_clean, *album_clean, *track_clean;
	TagLib_File *tlf;
	TagLib_Tag *tlt;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s directory\n", argv[0]);
		fprintf(stderr, "Please read the source code to be sure what this program does and if this is what you want. Otherwise it's very likely to destroy your data unintentionally.\n");
		exit(EXIT_FAILURE);
	}

	if (chdir(argv[1]) != 0) {
		chdir_error(argv[1]);
		exit(EXIT_FAILURE);
	}
	artists = opendir(".");
	if (artists == NULL) {
		opendir_error(argv[1]);
		exit(EXIT_FAILURE);
	}
	// loop thru artists
	while ((artist = readdir(artists)) != NULL) {
		if (! is_nice_dir(artist)) {
			continue;
		}
		if (chdir(artist->d_name) != 0) {
			chdir_error(artist->d_name);
			continue;
		}
		albums = opendir(".");
		if (albums == NULL) {
			opendir_error(artist->d_name);
			continue;
		}
		// loop thru albums
		while ((album = readdir(albums)) != NULL) {
			if ( ! is_nice_dir(album)) {
				continue;
			}
			if (chdir(album->d_name) != 0) {
				chdir_error(album->d_name);
				continue;
			}
			tracks = opendir(".");
			if (tracks == NULL) {
				opendir_error(album->d_name);
				continue;
			}
			// loop thru tracks
			while ((track = readdir(tracks)) != NULL) {
				if ( ! is_file(track)) {
					continue;
				}
				tlf = taglib_file_new(track->d_name);
				if (tlf == NULL) {
					fprintf(stderr, "Unable to open %s for tagging.\n", track->d_name);
					continue;
				}
				tlt = taglib_file_tag(tlf);
				if (tlt == NULL) {
					fprintf(stderr, "Unable to create tag for %s.\n", track->d_name);
					taglib_file_free(tlf);
					continue;
				}
				printf("Tagging: %s/%s/%s\n", artist->d_name, album->d_name, track->d_name);
				artist_clean = cleanup_text(artist->d_name);
				album_clean = cleanup_text(album->d_name);
				track_clean = cleanup_text(track->d_name);
				taglib_tag_set_artist(tlt, artist_clean);
				taglib_tag_set_album(tlt, album_clean);
				taglib_tag_set_title(tlt, track_clean);
				free(artist_clean);
				free(album_clean);
				free(track_clean);
				// TODO: track no.?
				taglib_file_save(tlf);
				taglib_file_free(tlf);
			}
			chdir("..");
		}
		chdir("..");
	}
	exit(EXIT_SUCCESS);
}

