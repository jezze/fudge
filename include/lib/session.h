#ifndef LIB_SESSION_H
#define LIB_SESSION_H

struct file_node;

struct file_node *session_get_cwd();
struct file_node *session_get_in();
struct file_node *session_get_out();

#endif

