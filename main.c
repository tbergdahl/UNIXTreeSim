#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>            
#include <stdlib.h> 
#include <string.h>    

// you can use additional headers as needed

typedef struct node {
	char  name[64];       // node's name string
	char  type;
	struct node* child, * sibling, * parent;
} NODE;


NODE* root;
NODE* cwd;
char cwd_name[500];
char* cmd[] = { "mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "reload", "save", "quit", 0 };  // fill with list of commands
// other global variables

void mkdir(char* pathname);
int rec_rmdir(char* pathname, NODE* node, NODE* parent, NODE* prev);
void rmdir(char* pathname);
int ls(char* pathname, NODE* node);
void cd(char* name, NODE* node);
void pwd();
void creat(char* pathname);
void rm(char* target_file);
int rec_rm(char* path, NODE* node, NODE* parent, NODE* prev);
NODE* reload(FILE* infile);
void save(NODE* node, FILE* outfile, int i);
void destroyTree(NODE* node);
int find_target_path(NODE* node, const char* target_node, char* current_path, NODE* parent, int i);



void initialize() {
	root = (NODE*)malloc(sizeof(NODE));
	strcpy(root->name, "/");
	root->parent = root;
	root->sibling = NULL;
	root->child = NULL;
	root->type = 'D';
	cwd = root;
	printf("Filesystem initialized!\n");
}

int main()
{
	// other initialization as needed
	initialize();
	while (1)
	{
		printf("\nEnter Command:\n");
		char command[100], line[100], path[100];
		for (int i = 0; i < 100; i++)
			line[i] = '\0';
		fgets(line, 100, stdin);


		if (line[0] == 'c' && line[1] == 'd') //this if() handles everything to do with cd since it has three options
		{
			for (int i = 0; i < strlen(line); i++)
			{
				if (line[i] == '\n')
					line[i] = '\0';
			}
			if (strcmp(line, "cd") == 0)
			{
				cd("", cwd);
			}
			else if (strcmp(line, "cd ..") == 0)
			{
				cd("..", cwd);
			}
			else
			{
				char temp[100];
				strcpy(temp, strtok(line, " "));
				strcpy(path, strtok(NULL, "\n"));
				cd(path, cwd);
			}
		}

		else //every other instruction
		{
			strcpy(command, strtok(line, " ")); //parse the command - if no argument will return NULL
			int one_arg = 0;
			for (int i = 0; i < strlen(command); i++)
			{
				if (command[i] == '\n') //check for an argument (if there is newline character that means there was an argument)
				{
					one_arg = 1; //no arguments (name is misleading)
					command[i] = '\0';
				}
			}
			if (!one_arg)
				strcpy(path, strtok(NULL, "\n"));//get argument to function call if there is one and store it in path

			if (strcmp(command, "mkdir") == 0)
			{
				mkdir(path);
			}
			else if (strcmp(command, "rmdir") == 0)
			{
				rmdir(path);
			}
			else if (strcmp(command, "ls") == 0)
			{
				if (!ls(path, root))
				{
					printf("Pathname %s does not exist.\n", path);
				}
			}
			else if (strcmp(command, "pwd") == 0)
			{
				pwd();
			}
			else if (strcmp(command, "creat") == 0)
			{
				creat(path);
			}
			else if (strcmp(command, "rm") == 0)
			{
				rm(path);
			}
			else if (strcmp(command, "reload") == 0)
			{
				FILE* infile = fopen(path, "r");
				destroyTree(root);
				initialize();
				reload(infile);
				fclose(infile);
			}
			else if (strcmp(command, "save") == 0)
			{
				if (!one_arg)
				{
					FILE* outfile = fopen(path, "w");
					save(root, outfile, 0);
					fclose(outfile);
				}
				else
				{
					printf("Provide a pathname.\n");
				}


			}
			else if (strcmp(command, "quit") == 0)
			{
				printf("Enter Last Name:\n"); //generate file with user last name
				char ln[50] = "", prefix[50] = "fssim.";
				fgets(ln, 50, stdin);
				for (int i = 0; i < strlen(ln); i++)
				{
					if (ln[i] == '\n')
					{
						ln[i] = '\0';
					}
				}
				strcat(prefix, ln);
				strcat(prefix, ".txt");
				FILE* outfile = fopen(prefix, "w");
				save(root, outfile, 0);
				destroyTree(root);
				fclose(outfile);
				break;
			}
			else
			{
				printf("Command Not Found!\n");
			}
		}
		strcpy(path, ""); //reset path
	}
	return 0;

}

void mkdir(char* pathname)
{
	if (pathname[0] == '/')//absolute path
	{
		strcat(pathname, "/\0");
		char line[500] = "", name[50] = "";
		char* token = strtok(pathname, "/");
		NODE* p = root->child, * parent = root, * prev = NULL;
		while (token != NULL)
		{
			if (strcmp(token, "\n") != 0)
			{
				// Update 'name' with the current directory in path
				strncpy(name, token, sizeof(name));
				name[sizeof(name) - 1] = '\0';
				while (p != NULL)
				{
					if (strcmp(p->name, name) == 0 && p->type != 'F') //found a working directory with our current directory name
					{
						parent = p;
						prev = p->child; //we can go to the next level to search for next directory in path
						p = p->child;
						token = strtok(NULL, "/");
						strncpy(name, token, sizeof(name));
					}
					else if (p->type == 'F')
					{
						printf("Cannot Create New Directory Under %s. %s is of Type FILE.\n", p->name, p->name);
					}
					else //move along siblings until we find working directory that matches
					{
						prev = p;
						p = p->sibling;
					}
				}

				if (p == NULL) //once we have found the end of the path, create new node with the last directory name
				{
					p = (NODE*)malloc(sizeof(NODE));
					if (p != NULL)
					{
						if (parent->child == NULL)
							parent->child = p;
						p->child = NULL;
						strcpy(p->name, name);
						p->parent = parent;
						p->sibling = NULL;
						if (prev != NULL)
							prev->sibling = p;
						p->type = 'D';
					}
				}

			}
			token = strtok(NULL, "/"); //get next directory name
		}
	}
	else if (strcmp(pathname, "") == 0)
	{
		printf("Please Provide a Pathname\n");
		return;
	}
	else //relative path
	{
		if (strcmp(pathname, "") != 0)
		{
			NODE* p = cwd->child;
			NODE* prev = NULL; // Track the previous node in the sibling list

			if (p == 0)//insert oldest sibling - current subtree empty
			{
				p = (NODE*)malloc(sizeof(NODE));
				strcpy(p->name, pathname);
				p->type = 'D';
				p->parent = cwd;
				cwd->child = p;
				p->child = 0;
				p->sibling = 0;
			}
			else
			{
				while (p != NULL && strcmp(p->name, pathname) != 0)
				{
					prev = p;
					p = p->sibling;
				} //look through cwd for directory name

				if (p == NULL) // Node with the given name does not exist
				{
					NODE* newNode = (NODE*)malloc(sizeof(NODE));
					if (newNode)
					{
						strcpy(newNode->name, pathname);
						newNode->type = 'D';
						newNode->parent = cwd;
						newNode->sibling = 0;
						newNode->child = 0;
						prev->sibling = newNode;
					}
				}
				else
				{
					printf("DIR %s already exists!\n", pathname);
				}
			}
		}
	}

}

int rec_rmdir(char* path, NODE* node, NODE* parent, NODE* prev)
{

	if (path == NULL || *path == '\0')
	{
		return 0;
	}

	char* dir_name = strchr(path, '/'); //find pointer to first slash ('/') in string
	char* orig_path = path; //save original path in case we dont find our match
	char name[50] = "";
	if (dir_name != NULL)
	{
		strncpy(name, path, dir_name - path); //store directory name from current spot in path
		name[dir_name - path] = '\0';
		path = dir_name + 1;//move pointer to point to character after first slash
	}
	else
	{
		//we have reached the last token of string (no more slashes)
		strcpy(name, path);
		path = NULL;
	}

	if (strcmp(name, node->name) == 0)
	{
		if (path == NULL && node->type != 'F' && node->child == NULL) //last token of string and also a directory and not empty
		{
			if (node->child != NULL) //directory is empty
			{
				return -2;
			}
			if (prev == NULL) //current node is oldest sibling
			{
				parent->child = node->sibling;
			}
			else
			{
				prev->sibling = node->sibling;
			}
			free(node);
			return 1;
		}
		else if (path != NULL) //we are on the right path, but haven't reached the end, so we need to go down a level to find the next directory in path
		{
			return rec_rmdir(path, node->child, node, NULL);
		}
		else
		{
			return -1;
		}
	}
	else //check the siblings in the current directory to find the right path
	{
		path = orig_path;
		return rec_rmdir(path, node->sibling, parent, node);
	}

}



void rmdir(char* pathname)
{
	if (pathname[0] == '/')//absolute path
	{
		pathname[0] = ' ';
		for (int i = 0; i < strlen(pathname); i++)
		{
			pathname[i] = pathname[i + 1];
		}
		int status = 0;
		if ((status = rec_rmdir(pathname, root->child, NULL, NULL)) == 1) //helper function that starts at root and recursively removes target node
		{
			printf("Deleted %s Successfully!\n", pathname);
		}
		else if (status == -1)
		{
			printf("%s is of type FILE. Please use rm.\n", pathname);
		}
		else if (status == -2)
		{
			printf("Dir %s if full! Cannot Delete.\n", pathname);
		}
		else
		{
			printf("%s not in file system.\n", pathname);
		}
		return;
	}
	if (strcmp(pathname, "") != 0)//relative path
	{
		NODE* p = cwd->child, * prev = NULL;
		if (p == 0)
		{
			printf("There are no directories\n");
		}
		else
		{
			while (p != NULL && strcmp(p->name, pathname) != 0)
			{
				prev = p;
				p = p->sibling;
			}
			if (p != NULL && p->type != 'F') // directory exists
			{
				if (p->child == NULL) // directory is empty and not type file
				{
					if (cwd->child == p) // target directory is oldest child
					{
						cwd->child = p->sibling;
					}
					else if (p->sibling != NULL) // target directory is a middle child 
					{
						prev->sibling = p->sibling;
					}
					else
					{
						prev->sibling = 0;
					}
					free(p);
				}
				else
				{
					printf("Cannot Delete - Directory Is Full\n");
				}
			}
			else if (p != NULL && p->type == 'F')
			{
				printf("Not a Directory\n");
			}
			else
			{
				printf("DIR %s does not exist.\n", pathname);
			}
		}
	}
}

int ls(char* pathname, NODE* node)
{

	if (strcmp(pathname, "") == 0)//relative path
	{
		NODE* p = cwd->child;
		while (p != 0)
		{
			printf("%c %s\n", p->type, p->name);
			p = p->sibling;
		}
		printf("\n\n");
		return 1;
	}
	else //absolute path
	{
		int success = 0;
		if (node != NULL)
		{
			if (strcmp(node->name, pathname) == 0)//found target node, print children
			{
				NODE* p = node->child;
				while (p != 0)
				{
					printf("%c %s\n", p->type, p->name);
					p = p->sibling;
				}
				printf("\n\n");
			}
			else
			{
				success = ls(pathname, node->child);//check child subtree for target directory
				if (success == 0)
				{
					success = ls(pathname, node->sibling);
				}
				else
				{
					ls(pathname, node->sibling);//don't want to overwrite success if we found it in child
				}
				return success;
			}
		}
		else
		{
			return success = 0;
		}
	}
}

void cd(char* name, NODE* node)
{
	if (strcmp(name, cwd->name) == 0)
		printf("Already In %s", name);

	else if (strcmp(name, "") == 0)
		cwd = root;

	else if (strcmp(name, "..") == 0)
	{
		cwd = cwd->parent;
	}

	else
	{
		NODE* p = cwd->child;
		while (p != 0 && strcmp(p->name, name) != 0)
		{
			p = p->sibling;//advance to target directory
		}
		if (p == 0)
		{
			printf("Pathname %s does not exist in directory.\n", name);
		}
		else
		{
			if (p->type == 'F')
			{
				printf("Cannot Change to a FILE.\n");
			}
			else
			{
				cwd = p;
			}
		}
	}
}

void pwd()
{
	char abs_path[500] = "";
	if (cwd != root)
	{
		find_target_path(root, cwd->name, abs_path, cwd->parent, 2); //builds abs_path via pointer - traverses tree and adds nodes to full path bottom-up
		char temp;
		for (int i = 1, j = strlen(abs_path) - 1; j > i; i++, j--)
		{
			temp = abs_path[i];
			abs_path[i] = abs_path[j];
			abs_path[j] = temp;  //need to do some string reversal due to the nature of the recursive function find_target_path
		}
		printf("%s\n", abs_path);
	}
	else
	{
		printf("/");
	}
}


int find_target_path(NODE* node, const char* target_node, char* current_path, NODE* parent, int i)
{
	if (node == NULL)
	{
		return 0;
	}
	if (node == root)
	{
		strcat(current_path, "/");
	}

	if (strcmp(node->name, target_node) == 0 && node->parent == parent)
	{
		char temp[90] = "";
		char copy[500] = "";
		strcpy(copy, node->name);
		if (i > 1) //i is passed through to keep track of when to reverse the string so it outputs correctly. if it is a deeper level, it needs to be reversed again
			//since the path is in reverse order. First have to reverse each word, then reverse the whole path. but if we are only one level deep, it will reverse one 
			//too many times, so that's why I keep track of level.
		{
			for (int i = 0, j = strlen(copy) - 1; j > i; i++, j--)
			{
				temp[i] = copy[i];
				copy[i] = copy[j];
				copy[j] = temp[i];
			}
		}
		strcat(current_path, copy);
		return 1;
	}
	int success;

	if ((success = find_target_path(node->child, target_node, current_path, node, i)) == 1 && node != root)//success determines whether or not target directory 
		//is in current node's subtree, if it is, add node to 
		//path
	{
		char temp[90] = "";
		char copy[500] = "";
		strcpy(copy, node->name);
		strcat(copy, "/");
		if (i > 1)
		{
			for (int i = 0, j = strlen(copy) - 1; j > i; i++, j--)
			{
				temp[i] = copy[i];
				copy[i] = copy[j];
				copy[j] = temp[i];
			}
		}
		strcat(current_path, copy);
		return success;
	}
	if ((success = find_target_path(node->sibling, target_node, current_path, node->parent, i)) == 1 && node != root)//if not in current node's subtree, go to sibling
	{
		return success;
	}


}

void creat(char* pathname)//same logic as mkdir(), just inputting 'F' as type instead of 'D'
{
	if (pathname[0] == '/')//absolute path
	{
		strcat(pathname, "/\0");
		char line[500] = "", name[50] = "";
		char* token = strtok(pathname, "/");
		NODE* p = root->child, * parent = root, * prev = NULL;
		while (token != NULL)
		{
			if (strcmp(token, "\n") != 0)
			{
				// Update 'name' with the current token
				strncpy(name, token, sizeof(name));
				name[sizeof(name) - 1] = '\0'; // Null-terminate the string
				while (p != NULL)
				{
					if (strcmp(p->name, name) == 0 && p->type != 'F') //found a working directory
					{
						parent = p;
						prev = p->child;
						p = p->child;
						token = strtok(NULL, "/");
						strncpy(name, token, sizeof(name));
					}
					else if (p->type == 'F')
					{
						printf("Cannot Create New FILE Under %s. %s is of Type FILE.\n", p->name, p->name);
					}
					else //move along siblings until we find working directory
					{
						prev = p;
						p = p->sibling;
					}
				}

				if (p == NULL)
				{
					p = (NODE*)malloc(sizeof(NODE));
					if (p != NULL)
					{
						if (parent->child == NULL)
							parent->child = p;
						p->child = NULL;
						strcpy(p->name, name);
						p->parent = parent;
						p->sibling = NULL;
						if (prev != NULL)
							prev->sibling = p;
						p->type = 'F';
					}
				}

			}
			token = strtok(NULL, "/");
		}
	}
	else if (strcmp(pathname, "") == 0)
	{
		printf("Please Provide a Pathname\n");
		return;
	}
	else //relative path
	{
		if (strcmp(pathname, "") != 0)
		{
			NODE* p = cwd->child;
			NODE* prev = NULL; // Track the previous node in the sibling list

			if (p == 0)
			{
				p = (NODE*)malloc(sizeof(NODE));
				strcpy(p->name, pathname);
				p->type = 'F';
				p->parent = cwd;
				cwd->child = p;
				p->child = 0;
				p->sibling = 0;
			}
			else
			{
				while (p != NULL && strcmp(p->name, pathname) != 0)
				{
					prev = p;
					p = p->sibling;
				}

				if (p == NULL) // Node with the given name does not exist
				{
					NODE* newNode = (NODE*)malloc(sizeof(NODE));
					if (newNode)
					{
						strcpy(newNode->name, pathname);
						newNode->type = 'F';
						newNode->parent = cwd;
						newNode->sibling = 0;
						newNode->child = 0;
						prev->sibling = newNode;
					}


				}
				else
				{
					printf("DIR pathname already exists!\n");
				}
			}
		}
	}
}

int rec_rm(char* path, NODE* node, NODE* parent, NODE* prev)
{

	if (path == NULL || *path == '\0')
	{
		return 0;
	}

	char* dir_name = strchr(path, '/');
	char* orig_path = path;
	char name[50] = ""; //find pointer to first slash in string
	if (dir_name != NULL)
	{
		strncpy(name, path, dir_name - path);
		name[dir_name - path] = '\0';
		path = dir_name + 1;//move pointer to point to character after first slash
	}
	else
	{
		//we have reached the last token of string (no more slashes)
		strcpy(name, path);
		path = NULL;
	}

	if (strcmp(name, node->name) == 0)
	{
		if (path == NULL && node->type != 'F' && node->child == NULL) //last token of string and also a directory
		{
			if (prev == NULL) //current node is oldest sibling
			{
				parent->child = node->sibling;
			}
			else
			{
				prev->sibling = node->sibling;
			}
			free(node);
			return 1;
		}
	}
	else //check the siblings for the current directory to find the right path
	{
		path = orig_path;
		return rec_rm(path, node->sibling, parent, node);
	}

}


void rm(char* pathname)
{
	if (pathname[0] == '/')
	{
		pathname[0] = ' ';
		for (int i = 0; i < strlen(pathname); i++)
		{
			pathname[i] = pathname[i + 1];
		}
		int status = 0;
		if ((status = rec_rm(pathname, root->child, NULL, NULL)) == 1) //helper function that starts at root and recursively removes target node
		{
			printf("Deleted %s Successfully!\n", pathname);
		}
		else if (status == -1)
		{
			printf("%s is of type DIR. Please use rmdir.\n", pathname);
		}
		else
		{
			printf("%s not in file system.\n", pathname);
		}
		return;
	}
	if (strcmp(pathname, "") != 0)
	{
		NODE* p = cwd->child, * prev = NULL;
		if (p == 0)
		{
			printf("There are no directories\n");
		}
		else
		{
			while (p != NULL && strcmp(p->name, pathname) != 0)
			{
				prev = p;
				p = p->sibling;
			}
			if (p != NULL && strcmp(p->name, pathname) == 0 && p->type != 'D') // file exists
			{
				if (cwd->child == p) // target directory is oldest child
				{
					cwd->child = p->sibling;
				}
				else if (p->sibling != NULL) // target directory is a middle child 
				{
					prev->sibling = p->sibling;
				}
				else
				{
					prev->sibling = 0;
				}
				free(p);
			}
			else if (p != NULL && p->type == 'D')
			{
				printf("%s Is Not a file\n", pathname);
			}
			else
			{
				printf("FILE pathname does not exist.\n");
			}
		}
	}
}





NODE* reload(FILE* infile)
{
	char line[500] = "", * token, type = '\0', name[50] = "";
	int cd = 0, wordEnd = 0;
	fgets(line, 500, infile);
	while (fgets(line, 500, infile) != NULL)
	{
		type = '\0';
		strtok(line, " ");  // Tokenize the line based on space
		char* token = strtok(NULL, "/"); // Tokenize the second part based on '/'
		NODE* p = root->child, * parent = root, * prev = NULL;
		while (token != NULL)
		{
			// Get the type if it hasn't been extracted yet
			if (type == '\0') {
				type = line[0];
			}

			if (strcmp(token, "\n") != 0)
			{
				// Update 'name' with the current token
				strncpy(name, token, sizeof(name));
				name[sizeof(name) - 1] = '\0';
				while (p != NULL)
				{
					if (strcmp(p->name, name) == 0) //found a working directory
					{
						parent = p;
						prev = p->child;
						p = p->child;
						token = strtok(NULL, "/");
						strncpy(name, token, sizeof(name));
					}
					else //move along siblings until we find working directory
					{
						prev = p;
						p = p->sibling;
					}
				}

				if (p == NULL)
				{
					p = (NODE*)malloc(sizeof(NODE));
					if (p != NULL)
					{
						if (parent->child == NULL)
							parent->child = p;
						p->child = NULL;
						strcpy(p->name, name);
						p->parent = parent;
						p->sibling = NULL;
						if (prev != NULL)
							prev->sibling = p;
						p->type = type;
					}
				}

			}

			// Get the next token (if any) for further nesting
			token = strtok(NULL, "/");
		}


	}
}


void save(NODE* node, FILE* outfile, int i)
{
	if (node != 0)
	{
		char full_path[500] = "";
		find_target_path(root, node->name, full_path, node->parent, i); //fetches current node's full path
		char temp;
		if (i > 1)
		{ //reverse string since find_target_path returns the path in reverse order
			for (int i = 1, j = strlen(full_path) - 1; j > i; i++, j--)
			{
				temp = full_path[i];
				full_path[i] = full_path[j];
				full_path[j] = temp;
			}
		}
		fprintf(outfile, "%c %s/\n", node->type, full_path);
		save(node->child, outfile, ++i);
		save(node->sibling, outfile, ++i);
	}
}


void destroyTree(NODE* node)
{
	if (node != NULL)
	{
		destroyTree(node->child);
		destroyTree(node->sibling);
		free(node);
	}
}