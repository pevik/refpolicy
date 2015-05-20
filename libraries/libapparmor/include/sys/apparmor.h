/*
 * Copyright (c) 2003-2008 Novell, Inc. (All rights reserved)
 * Copyright 2009-2010 Canonical Ltd.
 *
 * The libapparmor library is licensed under the terms of the GNU
 * Lesser General Public License, version 2.1. Please see the file
 * COPYING.LGPL.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SYS_APPARMOR_H
#define _SYS_APPARMOR_H	1

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

__BEGIN_DECLS

/*
 * Class of public mediation types in the AppArmor policy db
 */

#define AA_CLASS_DBUS		32


/* Permission flags for the AA_CLASS_DBUS mediation class */
#define AA_DBUS_SEND			(1 << 1)
#define AA_DBUS_RECEIVE		 	(1 << 2)
#define AA_DBUS_EAVESDROP		(1 << 5)
#define AA_DBUS_BIND			(1 << 6)
#define AA_VALID_DBUS_PERMS		(AA_DBUS_SEND | AA_DBUS_RECEIVE | \
					 AA_DBUS_BIND | AA_DBUS_EAVESDROP)


/* Prototypes for apparmor state queries */
extern int aa_is_enabled(void);
extern int aa_find_mountpoint(char **mnt);

/* Prototypes for self directed domain transitions
 * see <http://apparmor.net>
 * Please see the change_hat(2) manpage for information.
 */

#define change_hat(X, Y) aa_change_hat((X), (Y))
extern int (change_hat)(const char *subprofile, unsigned int magic_token);
extern int aa_change_hat(const char *subprofile, unsigned long magic_token);
extern int aa_change_profile(const char *profile);
extern int aa_change_onexec(const char *profile);

extern int aa_change_hatv(const char *subprofiles[], unsigned long token);
extern int (aa_change_hat_vargs)(unsigned long token, int count, ...);

extern char *aa_splitcon(char *con, char **mode);
/* Protypes for introspecting task confinement
 * Please see the aa_getcon(2) manpage for information
 */
extern int aa_getprocattr_raw(pid_t tid, const char *attr, char *buf, int len,
			      char **mode);
extern int aa_getprocattr(pid_t tid, const char *attr, char **label,
			  char **mode);
extern int aa_gettaskcon(pid_t target, char **label, char **mode);
extern int aa_getcon(char **label, char **mode);
extern int aa_getpeercon_raw(int fd, char *buf, int *len, char **mode);
extern int aa_getpeercon(int fd, char **label, char **mode);

/* A NUL character is used to separate the query command prefix string from the
 * rest of the query string. The query command sizes intentionally include the
 * NUL-terminator in their values.
 */
#define AA_QUERY_CMD_LABEL		"label"
#define AA_QUERY_CMD_LABEL_SIZE		sizeof(AA_QUERY_CMD_LABEL)

extern int aa_query_label(uint32_t mask, char *query, size_t size, int *allow,
			  int *audit);

#define __macroarg_counter(Y...) __macroarg_count1 ( , ##Y)
#define __macroarg_count1(Y...) __macroarg_count2 (Y, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#define __macroarg_count2(_,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,n,Y...) n

/**
 * change_hat_vargs - a wrapper macro for change_hat_vargs
 * @T: the magic token
 * @X...: the parameter list of hats being passed
 *
 * The change_hat_vargs macro makes it so the caller doesn't have to
 * specify the number of hats passed as parameters to the change_hat_vargs
 * fn.
 *
 * eg.
 * change_hat_vargs(10, hat1, hat2, hat3, hat4);
 * expandes to
 * (change_hat_vargs)(10, 4, hat1, hat2, hat3, hat4);
 *
 * to call change_hat_vargs direction do
 * (change_hat_vargs)(token, nhats, hat1, hat2...)
 */
#define aa_change_hat_vargs(T, X...) \
	(aa_change_hat_vargs)(T, __macroarg_counter(X), X)

typedef struct aa_features aa_features;
int aa_features_new(aa_features **features, const char *path);
int aa_features_new_from_string(aa_features **features,
				const char *string, size_t size);
int aa_features_new_from_kernel(aa_features **features);
aa_features *aa_features_ref(aa_features *features);
void aa_features_unref(aa_features *features);

int aa_features_write_to_file(aa_features *features, const char *path);
bool aa_features_is_equal(aa_features *features1, aa_features *features2);
bool aa_features_supports(aa_features *features, const char *str);

typedef struct aa_kernel_interface aa_kernel_interface;
int aa_kernel_interface_new(aa_kernel_interface **kernel_interface,
			    aa_features *kernel_features,
			    const char *apparmorfs);
aa_kernel_interface *aa_kernel_interface_ref(aa_kernel_interface *kernel_interface);
void aa_kernel_interface_unref(aa_kernel_interface *kernel_interface);

int aa_kernel_interface_load_policy(aa_kernel_interface *kernel_interface,
				    const char *buffer, size_t size);
int aa_kernel_interface_load_policy_from_file(aa_kernel_interface *kernel_interface,
					      const char *path);
int aa_kernel_interface_load_policy_from_fd(aa_kernel_interface *kernel_interface,
					    int fd);
int aa_kernel_interface_replace_policy(aa_kernel_interface *kernel_interface,
				       const char *buffer, size_t size);
int aa_kernel_interface_replace_policy_from_file(aa_kernel_interface *kernel_interface,
						 const char *path);
int aa_kernel_interface_replace_policy_from_fd(aa_kernel_interface *kernel_interface,
					       int fd);
int aa_kernel_interface_remove_policy(aa_kernel_interface *kernel_interface,
				      const char *fqname);
int aa_kernel_interface_write_policy(int fd, const char *buffer, size_t size);

typedef struct aa_policy_cache aa_policy_cache;
int aa_policy_cache_new(aa_policy_cache **policy_cache,
			aa_features *kernel_features, const char *path,
			bool create);
aa_policy_cache *aa_policy_cache_ref(aa_policy_cache *policy_cache);
void aa_policy_cache_unref(aa_policy_cache *policy_cache);

bool aa_policy_cache_is_valid(aa_policy_cache *policy_cache);
int aa_policy_cache_create(aa_policy_cache *policy_cache);
int aa_policy_cache_remove(const char *path);
int aa_policy_cache_replace_all(aa_policy_cache *policy_cache,
				aa_kernel_interface *kernel_interface);

__END_DECLS

#endif	/* sys/apparmor.h */
