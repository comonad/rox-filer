/* vi: set cindent:
 * $Id$
 *
 * ROX-Filer, filer for the ROX desktop project
 * By Thomas Leonard, <tal197@ecs.soton.ac.uk>.
 */

#ifndef _SUPPORT_H
#define _SUPPORT_H

#include <glib.h>

char *pathdup(char *path);
GString *make_path(char *dir, char *leaf);
char *our_host_name();
int spawn(char **argv);

#endif /* _SUPPORT_H */
