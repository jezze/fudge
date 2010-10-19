#ifndef LIB_SESSION_H
#define LIB_SESSION_H

struct vfs_node;

struct vfs_node *session_get_location();
struct vfs_node *session_get_in();
struct vfs_node *session_get_out();

#endif

