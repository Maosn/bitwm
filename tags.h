typedef struct tag tag;
typedef struct tag_space tag_space;

struct tag
{
	char name;
	node *target;
	tag *next_tag;
};

struct tag_space
{
	node *scope;
	tag *first_tag;
	tag_space *next_tag_space;
};

tag *find_tag (tag *tags, char name);
tag_space *find_tag_space (tag_space *tag_spaces, node *current_node);

tag *create_tag (char name);
tag_space *create_tag_space (node *scope);

/*
tag *index_tag (tag *first_tag, int index);
tag_space *index_tag_space (tag_space *first_tag_space, int index);
*/

node *find_tagged_node (tag_space *tag_spaces, node *current_node, char name);
