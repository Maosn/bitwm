#include <stdlib.h>
#include <stdio.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "bin_tree.h"

xcb_key_symbols_t *keysyms;

typedef struct binding
{
	xcb_keysym_t key_sym;
	xcb_keycode_t key_code;
	uint16_t modifiers;
	char *arguments;
	void (*function) (char *arguments);
} binding; 

typedef struct workspace
{
	node *top_node;
	rectangle *dimensions;
} workspace;

void exec_dmenu (char *arguments);
xcb_keycode_t key_sym_to_code (xcb_keysym_t keysym);

xcb_connection_t *connection;

int main (void)
{
	setbuf(stdout, NULL);

	connection = xcb_connect(NULL, NULL);
	const xcb_setup_t *setup = xcb_get_setup(connection);
	xcb_screen_t *screen = xcb_setup_roots_iterator(setup).data;
	keysyms = xcb_key_symbols_alloc(connection);

	rectangle *screen_dimensions = malloc(sizeof(rectangle));
	screen_dimensions->x = 0;
	screen_dimensions->y = 0;
	screen_dimensions->width = screen->width_in_pixels;
	screen_dimensions->height = screen->height_in_pixels;

	int num_bindings = 1;
	binding bindings[num_bindings];

	const uint32_t value[1] = {XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};

	xcb_change_window_attributes(connection, screen->root, XCB_CW_EVENT_MASK, value);

	bindings[0].key_sym = ' ';
	bindings[0].modifiers = XCB_MOD_MASK_CONTROL;
	bindings[0].function = exec_dmenu;


	for (int i = 0; i < num_bindings; i++)
	{
		bindings[i].key_code = key_sym_to_code(bindings[i].key_sym);
		xcb_grab_key(connection, 1, screen->root, bindings[i].modifiers, bindings[i].key_code, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	}

	xcb_flush(connection);

	int num_workspaces = 8;
	node *workspaces[num_workspaces];

	node *tree = create_tree_with_pointers(NULL, workspaces, num_workspaces);

	for (int i = 0; i < num_workspaces; i++)
		workspaces[i]->type |= WORKSPACE | LEAVE_BLANK;

	node *focus = workspaces[0];

	while (1)
	{
		xcb_generic_event_t *event = xcb_wait_for_event(connection);

		switch (event->response_type)
		{
			case XCB_KEY_PRESS:;
				xcb_key_press_event_t *key_event = (xcb_key_press_event_t *) event;
				for (int i = 0; i < num_bindings; i++)
					if (bindings[i].key_code == key_event->detail)
						bindings[i].function(bindings[i].arguments);
				break;
			case XCB_MAP_NOTIFY:;
				xcb_map_notify_event_t *map_event = (xcb_map_notify_event_t *) event;

				xcb_get_window_attributes_cookie_t attributes_cookie = xcb_get_window_attributes_unchecked(connection, map_event->window);
				xcb_get_window_attributes_reply_t *attributes_reply = xcb_get_window_attributes_reply(connection, attributes_cookie, NULL);

				if (!find_window(tree, map_event->window) && !attributes_reply->override_redirect)
				{
					if (focus->type & WORKSPACE)
					{
						int i;
						for (i = 0; i < num_workspaces && workspaces[i] != focus; i++)
							;
						window *new_window = create_window(WINDOW, map_event->window);
						workspaces[i] = fork_node(focus, (node *) new_window, V_SPLIT_CONTAINER);
						workspaces[i]->type |= WORKSPACE;
						focus = (node *) new_window;
					}
					else
					{
						window *new_window = create_window(WINDOW, map_event->window);
						fork_node(focus, (node *) new_window, V_SPLIT_CONTAINER);
						focus = (node *) new_window;
					}

					rectangle dimensions = *screen_dimensions;
					rectangle *container_dimensions = get_node_dimensions(focus, &dimensions);
					if (container_dimensions)
						configure_tree(connection, (node *) focus->parent, container_dimensions);

					xcb_flush(connection);
				}

				free(attributes_reply);

				break;
			case XCB_UNMAP_NOTIFY:;
				/*
				xcb_map_notify_event_t *unmap_event = (xcb_unmap_notify_event_t *) event;
				window *old_window;

				if (old_window = find_window(tree, unmap_event->window))
				{
					unfork_node();
				}
				*/
				break;
		}
	}
}

void exec_dmenu (char *arguments)
{
	system("exec dmenu_run");
}

xcb_keycode_t key_sym_to_code(xcb_keysym_t keysym)
{
	xcb_keycode_t *key_pointer;
	xcb_keycode_t key_code;

	key_pointer = xcb_key_symbols_get_keycode(keysyms, keysym);

	if (key_pointer == NULL)
		return 0;

	key_code = *key_pointer;
	free(key_pointer);

	return key_code;
}

