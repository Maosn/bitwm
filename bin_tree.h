#ifndef BIN_TREE_HEADER
#define BIN_TREE_HEADER

#include <stdlib.h>
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#define CHILD_NUMBER(current_node) ((current_node)->parent->child[0] == ((node *) (current_node)) ? 0 : 1)
#define AS_CHILD(current_node) ((current_node)->parent->child[CHILD_NUMBER(current_node)])
#define SIBLING(current_node) ((current_node)->parent->child[!CHILD_NUMBER(current_node)])

typedef uint32_t xcb_window_t;

typedef struct node node;
typedef struct window window;
typedef struct container container;
typedef struct rectangle rectangle;

enum node_types
{
	WINDOW = 1,
	V_SPLIT_CONTAINER = 2,
	H_SPLIT_CONTAINER = 4,
};

struct node
{
	uint8_t	type;
	container *parent;
};

struct window
{
	uint8_t	type; //should always include WINDOW
	container *parent;
	xcb_window_t id;
};

struct container
{
	uint8_t	type; //should include either H_SPLIT_CONTAINER or V_SPLIT_CONTAINER
	container *parent;
	node *child[2];
	double split_ratio;
};

struct rectangle
{
	double x, y, width, height;
};

window *create_window (uint8_t	type, xcb_window_t id);
container *create_container (uint8_t	type);

node *fork_node (node *existing_node, node *new_node, uint8_t	split_type);
container *unfork_node (node *old_node);

window *adjacent_window (node *current_node, uint8_t split_type, uint8_t child_number);

void configure_tree (xcb_connection_t *connection, node *current_node, rectangle dimensions);
void kill_tree (xcb_connection_t *connection, node *current_node);

/*
node *create_tree_with_pointers (container *parent, node **pointers, int num_nodes);

void print_tree (node *current_node, int num_tabs);
*/
#endif
