/*
 * $Id$
 *
 * ROX-Filer, filer for the ROX desktop project
 * By Thomas Leonard, <tal197@ecs.soton.ac.uk>.
 */

#include <glib.h>
#include <sys/stat.h>

typedef struct _FindCondition FindCondition;
typedef struct _FindInfo FindInfo;
typedef gboolean (*FindTest)(FindCondition *condition, FindInfo *info);
typedef void (*FindFree)(FindCondition *condition);

struct _FindCondition
{
	FindTest	test;
	FindFree	free;
	/* These next three depend on the first two... */
	gpointer	data1;
	gpointer	data2;
	gint		value;
};

struct _FindInfo
{
	guchar		*fullpath;
	guchar		*leaf;
	struct stat	stats;
	time_t		now;
	gboolean	prune;
};

FindCondition *find_compile(guchar *string);
gboolean find_test_condition(FindCondition *condition, FindInfo *info);
void find_condition_free(FindCondition *condition);
