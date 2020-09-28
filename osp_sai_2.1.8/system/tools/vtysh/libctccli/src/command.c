/* Command interpreter routine for virtual terminal [aka TeletYpe]
   Copyright (C) 1997, 98, 99 Kunihiro Ishiguro

This file is part of GNU Zebra.
 
GNU Zebra is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2, or (at your
option) any later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "ctc_cli.h"
#include "zebra.h"

#define MAX_OPTIONAL_CMD_NUM 100 /* max optional cmd number in {} */

typedef enum best_match_type_s
{
    cmd_exact_match = 0,
    cmd_partly_match,
    cmd_extend_match,
    cmd_imcomplete_match,
}best_match_type_t;

/* Command vector which includes some level of command lists. Normally
   each daemon maintains each own vtysh_cmdvec. */
vector vtysh_cmdvec;
int vtysh_cmd_debug = 0;
int vtysh_cmd_arg_debug = 0;
struct desc ** vtysh_matched_desc_ptr  = NULL;

/* Host information structure. */
struct host host;

best_match_type_t cmd_best_match_check(vector vline, struct desc** vtysh_matched_desc_ptr, int if_describe)
{
    int i =0;
    char* command = NULL;
    char* str = NULL;
    struct desc* matched_desc = NULL;
    int max_index = vector_max   (vline);

    if(if_describe)
    {
        max_index--;
    }

    for(i=0; i<max_index; i++)
    {
        command = vector_slot(vline, i);
        matched_desc = vtysh_matched_desc_ptr[i];
        if(command && command[0]>='a' && command[0]<='z') /* keyword format*/
        {
            str = matched_desc->cmd;
            if(CMD_VARIABLE(str))
            {
                return cmd_extend_match; /* extend match */
            }

            if (strncmp (command, str, strlen (command)) == 0)
            {
                if (strcmp (command, str) == 0) /* exact match */
                { 
                    continue;
                }
                else
                {
                    return cmd_partly_match; /* partly match */
                }
            }
        }
    }

    return cmd_exact_match; /* exact match */
}

char* strdup_ctc(char* str)
{
  char* new_str = malloc(strlen(str)+1);
  if(new_str)
    memcpy(new_str, str, strlen(str)+1);

  return new_str;
}
#if 0
/* Utility function to concatenate argv argument into a single string
   with inserting ' ' character between each argument.  */
char *
argv_concat (char **argv, int argc, int shift)
{
  int i;
  int len;
  int index;
  char *str;

  str = NULL;
  index = 0;

  for (i = shift; i < argc; i++)
    {
      len = strlen (argv[i]);

      if (i == shift)
	{
	  str = XSTRDUP (MTYPE_VECTOR, argv[i]);
	  index = len;
	}
      else
	{
	  str = XREALLOC (MTYPE_TMP, str, (index + len + 2));
	  str[index++] = ' ';
	  memcpy (str + index, argv[i], len);
	  index += len;
	  str[index] = '\0';
	}
    }
  return str;
}
#endif

/* Install top node of command vector. */
void
ctc_install_node (struct cmd_node *node, 
	      int (*func) (struct vty *))
{
  vtysh_vector_set_index (vtysh_cmdvec, node->node, node);
  node->func = func;
  node->cmd_vector = vtysh_vector_init (VECTOR_MIN_SIZE);
  if(!node->cmd_vector)
  {
    printf("System error: no memory for install node!\n\r");
  }
}

/* Compare two command's string.  Used in sort_node (). */
int
cmp_node (const void *p, const void *q)
{
  struct cmd_element *a = *(struct cmd_element **)p;
  struct cmd_element *b = *(struct cmd_element **)q;

  return strcmp (a->string, b->string);
}

int
cmp_desc (const void *p, const void *q)
{
  struct desc *a = *(struct desc **)p;
  struct desc *b = *(struct desc **)q;

  return strcmp (a->cmd, b->cmd);
}

/* Sort each node's command element according to command string. */
void
ctc_sort_node ()
{
  int i;
 /* int j;*/
  struct cmd_node *cnode;
  /*vector descvec;*/
  /*struct cmd_element *cmd_element;*/

  for (i = 0; i < vector_max (vtysh_cmdvec); i++) 
    if ((cnode = vector_slot (vtysh_cmdvec, i)) != NULL)
      {	
	vector cmd_vector = cnode->cmd_vector;
	qsort (cmd_vector->index, cmd_vector->max, sizeof (void *), cmp_node);
/*
	for (j = 0; j < vector_max (cmd_vector); j++)
	  if ((cmd_element = vector_slot (cmd_vector, j)) != NULL)
	    {
	      descvec = vector_slot (cmd_element->strvec,
				     vector_max (cmd_element->strvec) - 1);
	      qsort (descvec->index, descvec->max, sizeof (void *), cmp_desc);
	    }
*/
      }
}

/* Breaking up string into each command piece. I assume given
   character is separated by a space character. Return value is a
   vector which includes char ** data element. */
vector
cmd_make_strvec (char *string)
{
  char *cp, *start, *token;
  int strlen;
  vector strvec;
  
  if (string == NULL)
    return NULL;
  
  cp = string;

  /* Skip white spaces. */
  while (isspace ((int) *cp) && *cp != '\0')
    cp++;

  /* Return if there is only white spaces */
  if (*cp == '\0')
    return NULL;

  if (*cp == '!' || *cp == '#')
    return NULL;

  /* Prepare return vector. */
  strvec = vtysh_vector_init (VECTOR_MIN_SIZE);
  if(!strvec)
  {
    return NULL;
  }

  /* Copy each command piece and set into vector. */
  while (1) 
    {
      start = cp;
      while (!(isspace ((int) *cp) || *cp == '\r' || *cp == '\n') &&
	     *cp != '\0')
	cp++;
      strlen = cp - start;
      token = XMALLOC (MTYPE_VECTOR, strlen + 1);
      memcpy (token, start, strlen);
      *(token + strlen) = '\0';
      vtysh_vector_set (strvec, token);

      while ((isspace ((int) *cp) || *cp == '\n' || *cp == '\r') &&
	     *cp != '\0')
	cp++;

      if (*cp == '\0')
	return strvec;
    }
}

/* Free allocated string vector. */
void
cmd_free_strvec (vector v)
{
  int i;
  char *cp;

  if (!v)
    return;

  for (i = 0; i < vector_max (v); i++)
    if ((cp = vector_slot (v, i)) != NULL)
      XFREE (MTYPE_VECTOR, cp);

  vtysh_vector_free (v);
}

/* Fetch next description.  Used in cmd_make_descvec(). */
char *
cmd_desc_str (char **string)
{
  char *cp, *start, *token;
  int strlen;
  
  cp = *string;

  if (cp == NULL)
    return NULL;

  /* Skip white spaces. */
  while (isspace ((int) *cp) && *cp != '\0')
    cp++;

  /* Return if there is only white spaces */
  if (*cp == '\0')
    return NULL;

  start = cp;

  while (!(*cp == '\r' || *cp == '\n') && *cp != '\0')
    cp++;

  strlen = cp - start;
  token = XMALLOC (MTYPE_VECTOR, strlen + 1);
  memcpy (token, start, strlen);
  *(token + strlen) = '\0';

  *string = cp;

  return token;
}

char* vtysh_cmd_parse_token(char** cp, cmd_token_type *token_type)
{
    char *sp = NULL;
    char *token = NULL;
    int len = 0;
    int need_while = 1;

    if (**cp == '\0')
    {
        *token_type =  cmd_token_unknown;       
        return NULL;
    }

    while (**cp != '\0' && need_while)
    {
        switch(**cp)
        {
            case ' ':
                (*cp)++;;
                break;
            case '{':
                (*cp)++;
                *token_type = cmd_token_cbrace_open;
                return NULL;
            case '(':
                (*cp)++;
                *token_type = cmd_token_paren_open;
                return NULL;
            case '|':
                (*cp)++;
                *token_type = cmd_token_separator;
                return NULL;
            case ')':
                (*cp)++;
                *token_type = cmd_token_paren_close;
                return NULL;              
            case '}':
                (*cp)++;
                *token_type = cmd_token_cbrace_close;
                return NULL;
            case '\n':                
                (*cp)++;
                break;
            case '\r':
                (*cp)++;
                break;
            default:
                need_while = 0;
                break;
        }
    }

    sp = *cp;
    while (! (**cp==' ' || **cp == '\r' || **cp == '\n' || **cp == ')' || **cp == '(' ||  **cp == '{' || **cp == '}' || **cp == '|') && **cp != '\0')
        (*cp)++;
    len = *cp - sp;
    
    if(len)
    {
        token = XMALLOC (MTYPE_VECTOR, len + 1);
        memcpy (token, sp, len);
        *(token + len) = '\0';
        if(CMD_VARIABLE(token))
        {
            *token_type = cmd_token_var;
        }
        else
        {
            *token_type = cmd_token_keyword;
        }
        return token;
    }

    *token_type = cmd_token_unknown;
    return NULL;   
}

vector vtysh_cmd_make_cli_tree(struct desc *tmp_desc, char **descstr, vector parent, int* dp_index, int depth)
{
    cmd_token_type token_type = 0;

    char *token = NULL;
    vector cur_vec = NULL;
    vector pending_vec = NULL;
    vector sub_parent_vec = NULL;
    struct desc *desc = NULL;
    int flag = 0;
    vector p = NULL;

    while (*(tmp_desc->str) != '\0')
    {
        token = vtysh_cmd_parse_token(&tmp_desc->str, &token_type); 

        switch(token_type)
        {
            case cmd_token_paren_open:
                cur_vec = vtysh_vector_init (VECTOR_MIN_SIZE);
                cur_vec->is_desc =0;
                cur_vec->is_multiple = 0;
                cur_vec->direction = 0;    
                if(flag) /* '(' after '|', finish previous building */
                {
                    flag++;
                    if(flag==2) /* flag==2 first keyword or VAR after seperator */
                    {
                        pending_vec = cur_vec;
                    }
                    else if(flag==3) /* 2 words are after seperator, current and pending vectors belong to sub_parent_vec */
                    {
                        sub_parent_vec = vtysh_vector_init (VECTOR_MIN_SIZE);
                        vtysh_vector_set(sub_parent_vec, pending_vec);
                        vtysh_vector_set(sub_parent_vec, cur_vec);
                        vtysh_vector_set(parent, sub_parent_vec);
                    }
                    else/* 2 more words are after seperator */
                    {
                        vtysh_vector_set(sub_parent_vec, cur_vec); /* all more vectors belong to sub_parent_vec */
                    }
                } 
                else
                {
                    vtysh_vector_set(parent, cur_vec);
                }               
                vtysh_cmd_make_cli_tree(tmp_desc, descstr, cur_vec, dp_index, depth+1);
                cur_vec = NULL;              
                break;
            case cmd_token_cbrace_open:
                cur_vec = vtysh_vector_init (VECTOR_MIN_SIZE);
                cur_vec->is_desc =0;
                cur_vec->is_multiple = 1; /* this is difference for {} and (), other codes are same */
                cur_vec->direction = 0;
                if(flag)
                {
                    flag++;
                    if(flag==2)
                    {
                        pending_vec = cur_vec;
                    }
                    else if(flag==3)
                    {
                        sub_parent_vec = vtysh_vector_init (VECTOR_MIN_SIZE);
                        vtysh_vector_set(sub_parent_vec, pending_vec);
                        vtysh_vector_set(sub_parent_vec, cur_vec);
                        vtysh_vector_set(parent, sub_parent_vec);
                    }
                    else
                    {
                        vtysh_vector_set(sub_parent_vec, cur_vec);
                    }
                } 
                else
                {
                    vtysh_vector_set(parent, cur_vec);
                } 
                vtysh_cmd_make_cli_tree(tmp_desc, descstr, cur_vec, dp_index, depth+1);
                cur_vec = NULL;
                break;
            case cmd_token_paren_close:
            case cmd_token_cbrace_close:
                if(flag==2) /* flag==2 first keyword after seperator, only one keyword  */
                {
                    vtysh_vector_set(parent, pending_vec);
                }       
                flag = 0;         
                return parent;
                break;
            case cmd_token_separator: 
                if(!parent->direction && (vtysh_vector_count(parent)>1)) /* if current parent is tranverse and has more than 2 vector, make it a sub parent*/
                {
                    p = vtysh_vector_copy(parent);
                    memset(parent->index, 0, sizeof(void*)*parent->max);
                    vector_reset(parent);
                    vtysh_vector_set(parent, p);
                }
                parent->direction = 1;
                if(flag == 2) /* new seperator starts, finish previous */
                {
                    vtysh_vector_set(parent, pending_vec);
                }
                flag = 1; /*flag=1, new seperator starts*/              
                cur_vec = NULL;
                break;
            case cmd_token_keyword:
                if(!cur_vec)
                {
                    cur_vec = vtysh_vector_init (VECTOR_MIN_SIZE);
                    cur_vec->direction = 0;
                    cur_vec->is_multiple = 0;
                    cur_vec->is_desc = 1;

                    if(flag)
                    {
                        flag++;
                        if(flag==2) /* flag==2 first keyword or VAR after seperator */
                        {
                            pending_vec = cur_vec;
                        }
                        else if(flag==3) /* flag==3 seconds keyword or VAR after seperator */
                        {
                            sub_parent_vec = vtysh_vector_init (VECTOR_MIN_SIZE);
                            vtysh_vector_set(sub_parent_vec, pending_vec);
                            vtysh_vector_set(sub_parent_vec, cur_vec);
                            vtysh_vector_set(parent, sub_parent_vec);
                        }
                        else /* flag>3, more keywords */
                        {
                            vtysh_vector_set(sub_parent_vec, cur_vec);
                        }
                    } 
                    else
                    {
                        vtysh_vector_set(parent, cur_vec);
                    }
                } 
                desc = XCALLOC (MTYPE_DESC, sizeof (struct desc));
                desc->cmd = token;
                desc->str = descstr[*dp_index];
                if(depth>0)
                {
                    desc->is_arg = 1;
                }
                else
                {
                    desc->is_arg = 0;
                }
                (*dp_index)++;
                 
                vtysh_vector_set(cur_vec, desc);
                break;
            case cmd_token_var:
                if(!cur_vec)
                {
                    cur_vec = vtysh_vector_init (VECTOR_MIN_SIZE);
                    cur_vec->direction = 0;
                    cur_vec->is_multiple = 0;
                    cur_vec->is_desc = 1;

                    if(flag) /* deal with seperator */
                    {
                        flag++;
                        if(flag==2) /* flag==2 first keyword or VAR after seperator */
                        {
                            pending_vec = cur_vec;
                        }
                        else if(flag==3) /* flag==3 seconds keyword or VAR after seperator */
                        {
                            sub_parent_vec = vtysh_vector_init (VECTOR_MIN_SIZE);
                            vtysh_vector_set(sub_parent_vec, pending_vec);
                            vtysh_vector_set(sub_parent_vec, cur_vec);
                            vtysh_vector_set(parent, sub_parent_vec);
                        }
                        else /* flag>3, more keywords or VAR */
                        {
                            vtysh_vector_set(sub_parent_vec, cur_vec);
                        }
                    } 
                    else
                    {
                        vtysh_vector_set(parent, cur_vec); 
                    }
                }  
                desc = XCALLOC (MTYPE_DESC, sizeof (struct desc));
                desc->cmd = token;
                desc->str = descstr[*dp_index];
                desc->is_arg = 1;
                (*dp_index)++;
          
                vtysh_vector_set(cur_vec, desc);
                break;
            default:
                break;
        }
    }    
    
    return parent;
}

vector
cmd_make_descvec (char *string, char **descstr)
{
    vector all_vec = NULL;
    int dp_index = 0;
    struct desc tmp_desc;

    tmp_desc.str = string;

    if (string == NULL)
        return NULL;

    all_vec = vtysh_vector_init (VECTOR_MIN_SIZE);
    all_vec->is_desc = 0;
    all_vec->direction = 0;
    all_vec->is_multiple = 0;

    return vtysh_cmd_make_cli_tree(&tmp_desc, descstr, all_vec, &dp_index, 0);
}

void vtysh_cmd_dump_vector_tree(vector all_vec, int depth)
{
    vector cur_vec = NULL;
    int i = 0;
    int j = 0;
    int space = 0;
    struct desc *desc;

    for(i=0; i<vector_max(all_vec); i++)
    {
        cur_vec = vector_slot(all_vec, i);
        for(space=0; space<depth; space++)
        {
            printf("  ");
        }
        printf("%d:", i);
        if(cur_vec->direction)
        {
            printf("V:");
        }
        else
        {
            printf("T:");
        }
        if(cur_vec->is_desc)
        {
            printf("s:");
        }
        else
        {
            printf("v:");
        }
        printf("\n\r");

        if(cur_vec->is_desc)
        {
            for(space=0; space<depth; space++)
            {
                printf("  ");
            }
            for(j=0; j<vector_max(cur_vec); j++)
            {
                desc = vector_slot(cur_vec, j);
                /*printf("  %s [%s] ", desc->cmd, desc->str);*/
                printf("  %s ", desc->cmd);
            }
            printf("\n\r");
        }
        else
        {
            vtysh_cmd_dump_vector_tree(cur_vec, depth+1);
        }
    }    
}

/* Count mandantory string vector size.  This is to determine inputed
   command has enough command length. */
int
cmd_cmdsize (vector strvec)
{
    int i;
    int size = 0;
    vector descvec;
    
    for (i = 0; i < vector_max (strvec); i++)
    {
        descvec = vector_slot (strvec, i);
    }
    
    return size;
}

/* Return prompt character of specified node. */
char *
cmd_prompt (enum node_type node)
{
  struct cmd_node *cnode;

  cnode = vector_slot (vtysh_cmdvec, node);
  return cnode->prompt;
}

struct desc* vtysh_cmd_get_desc(vector strvec, int index, int depth)
{
    vector descvec;

    if(index>= vector_max (strvec))
    {
        return NULL;
    }

    descvec = vector_slot (strvec, index);
    if(!descvec)
    {
        return NULL;
    }
    if(depth>=vector_max(descvec))
    {
        return NULL;
    }

    return vector_slot (descvec, depth);
}



/* Install a command into a node. */
void
install_element (enum node_type ntype, struct cmd_element *cmd)
{
    struct cmd_node *cnode;
    cnode = vector_slot (vtysh_cmdvec, ntype);

    if (cnode == NULL) 
    {
        fprintf (stderr, "Command node %d doesn't exist, please check it\n",
        ntype);
        exit (1);
    }

    vtysh_vector_set (cnode->cmd_vector, cmd);

    cmd->strvec = cmd_make_descvec (cmd->string, cmd->doc);
    cmd->cmdsize = cmd_cmdsize (cmd->strvec);

    if (vtysh_cmd_debug)
    {
        printf("cmdsize=%d for cmd: %s\n\r", cmd->cmdsize, cmd->string);
        if(cmd->strvec->direction)
        {
            printf("Parent V\n\r");
        }
        else
        {
            printf("Parent T\n\r");
        }
        vtysh_cmd_dump_vector_tree(cmd->strvec, 0);
    }
}

static unsigned char itoa64[] =	
"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void
to64(char *s, long v, int n)
{
  while (--n >= 0) 
    {
      *s++ = itoa64[v&0x3f];
      v >>= 6;
    }
}

/* Utility function for getting command vector. */
vector
cmd_node_vector (vector v, enum node_type ntype)
{
  struct cmd_node *cnode = vector_slot (v, ntype);
  return cnode->cmd_vector;
}

/* Filter command vector by symbol */
int
cmd_filter_by_symbol (char *command, char *symbol)
{
  int i, lim;

  if (strcmp (symbol, "IPV4_ADDRESS") == 0)
    {
      i = 0;
      lim = strlen (command);
      while (i < lim)
	{
	  if (! (isdigit ((int) command[i]) || command[i] == '.' || command[i] == '/'))
	    return 1;
	  i++;
	}
      return 0;
    }
  if (strcmp (symbol, "STRING") == 0)
    {
      i = 0;
      lim = strlen (command);
      while (i < lim)
	{
	  if (! (isalpha ((int) command[i]) || command[i] == '_' || command[i] == '-'))
	    return 1;
	  i++;
	}
      return 0;
    }
  if (strcmp (symbol, "IFNAME") == 0)
    {
      i = 0;
      lim = strlen (command);
      while (i < lim)
	{
	  if (! isalnum ((int) command[i]))
	    return 1;
	  i++;
	}
      return 0;
    }
  return 0;
}

/* Completion match types. */
enum match_type 
{
  no_match,
  extend_match,
  ipv4_prefix_match,
  ipv4_match,
  ipv6_prefix_match,
  ipv6_match,
  range_match,
  vararg_match,
  partly_match,
  exact_match,
  option_match,
  incomplete_cmd, 
};

enum match_type
cmd_ipv4_match (char *str)
{
  char *sp;
  int dots = 0, nums = 0;
  char buf[4];

  if (str == NULL)
    return partly_match;

  for (;;)
    {
      memset (buf, 0, sizeof (buf));
      sp = str;
      while (*str != '\0')
	{
	  if (*str == '.')
	    {
	      if (dots >= 3)
		return no_match;

	      if (*(str + 1) == '.')
		return no_match;

	      if (*(str + 1) == '\0')
		return partly_match;

	      dots++;
	      break;
	    }
	  if (!isdigit ((int) *str))
	    return no_match;

	  str++;
	}

      if (str - sp > 3)
	return no_match;

      strncpy (buf, sp, str - sp);
      if (atoi (buf) > 255)
	return no_match;

      nums++;

      if (*str == '\0')
	break;

      str++;
    }

  if (nums < 4)
    return partly_match;

  return exact_match;
}

enum match_type
cmd_ipv4_prefix_match (char *str)
{
  char *sp;
  int dots = 0;
  char buf[4];

  if (str == NULL)
    return partly_match;

  for (;;)
    {
      memset (buf, 0, sizeof (buf));
      sp = str;
      while (*str != '\0' && *str != '/')
	{
	  if (*str == '.')
	    {
	      if (dots == 3)
		return no_match;

	      if (*(str + 1) == '.' || *(str + 1) == '/')
		return no_match;

	      if (*(str + 1) == '\0')
		return partly_match;

	      dots++;
	      break;
	    }

	  if (!isdigit ((int) *str))
	    return no_match;

	  str++;
	}

      if (str - sp > 3)
	return no_match;

      strncpy (buf, sp, str - sp);
      if (atoi (buf) > 255)
	return no_match;

      if (dots == 3)
	{
	  if (*str == '/')
	    {
	      if (*(str + 1) == '\0')
		return partly_match;

	      str++;
	      break;
	    }
	  else if (*str == '\0')
	    return partly_match;
	}

      if (*str == '\0')
	return partly_match;

      str++;
    }

  sp = str;
  while (*str != '\0')
    {
      if (!isdigit ((int) *str))
	return no_match;

      str++;
    }

  if (atoi (sp) > 32)
    return no_match;

  return exact_match;
}

#define IPV6_ADDR_STR		"0123456789abcdefABCDEF:.%"
#define IPV6_PREFIX_STR		"0123456789abcdefABCDEF:.%/"
#define STATE_START		1
#define STATE_COLON		2
#define STATE_DOUBLE		3
#define STATE_ADDR		4
#define STATE_DOT               5
#define STATE_SLASH		6
#define STATE_MASK		7

enum match_type
cmd_ipv6_match (char *str)
{
  int state = STATE_START;
  int colons = 0, nums = 0, double_colon = 0;
  char *sp = NULL;

  if (str == NULL)
    return partly_match;

  if (strspn (str, IPV6_ADDR_STR) != strlen (str))
    return no_match;

  while (*str != '\0')
    {
      switch (state)
	{
	case STATE_START:
	  if (*str == ':')
	    {
	      if (*(str + 1) != ':' && *(str + 1) != '\0')
		return no_match;
     	      colons--;
	      state = STATE_COLON;
	    }
	  else
	    {
	      sp = str;
	      state = STATE_ADDR;
	    }

	  continue;
	case STATE_COLON:
	  colons++;
	  if (*(str + 1) == ':')
	    state = STATE_DOUBLE;
	  else
	    {
	      sp = str + 1;
	      state = STATE_ADDR;
	    }
	  break;
	case STATE_DOUBLE:
	  if (double_colon)
	    return no_match;

	  if (*(str + 1) == ':')
	    return no_match;
	  else
	    {
	      if (*(str + 1) != '\0')
		colons++;
	      sp = str + 1;
	      state = STATE_ADDR;
	    }

	  double_colon++;
	  nums++;
	  break;
	case STATE_ADDR:
	  if (*(str + 1) == ':' || *(str + 1) == '\0')
	    {
	      if (str - sp > 3)
		return no_match;

	      nums++;
	      state = STATE_COLON;
	    }
	  if (*(str + 1) == '.')
	    state = STATE_DOT;
	  break;
	case STATE_DOT:
	  state = STATE_ADDR;
	  break;
	default:
	  break;
	}

      if (nums > 8)
	return no_match;

      if (colons > 7)
	return no_match;

      str++;
    }

#if 0
  if (nums < 11)
    return partly_match;
#endif /* 0 */

  return exact_match;
}

enum match_type
cmd_ipv6_prefix_match (char *str)
{
  int state = STATE_START;
  int colons = 0, nums = 0, double_colon = 0;
  int mask;
  char *sp = NULL;
  char *endptr = NULL;

  if (str == NULL)
    return partly_match;

  if (strspn (str, IPV6_PREFIX_STR) != strlen (str))
    return no_match;

  while (*str != '\0' && state != STATE_MASK)
    {
      switch (state)
	{
	case STATE_START:
	  if (*str == ':')
	    {
	      if (*(str + 1) != ':' && *(str + 1) != '\0')
		return no_match;
	      colons--;
	      state = STATE_COLON;
	    }
	  else
	    {
	      sp = str;
	      state = STATE_ADDR;
	    }

	  continue;
	case STATE_COLON:
	  colons++;
	  if (*(str + 1) == '/')
	    return no_match;
	  else if (*(str + 1) == ':')
	    state = STATE_DOUBLE;
	  else
	    {
	      sp = str + 1;
	      state = STATE_ADDR;
	    }
	  break;
	case STATE_DOUBLE:
	  if (double_colon)
	    return no_match;

	  if (*(str + 1) == ':')
	    return no_match;
	  else
	    {
	      if (*(str + 1) != '\0' && *(str + 1) != '/')
		colons++;
	      sp = str + 1;

	      if (*(str + 1) == '/')
		state = STATE_SLASH;
	      else
		state = STATE_ADDR;
	    }

	  double_colon++;
	  nums += 1;
	  break;
	case STATE_ADDR:
	  if (*(str + 1) == ':' || *(str + 1) == '.'
	      || *(str + 1) == '\0' || *(str + 1) == '/')
	    {
	      if (str - sp > 3)
		return no_match;

	      for (; sp <= str; sp++)
		if (*sp == '/')
		  return no_match;

	      nums++;

	      if (*(str + 1) == ':')
		state = STATE_COLON;
	      else if (*(str + 1) == '.')
		state = STATE_DOT;
	      else if (*(str + 1) == '/')
		state = STATE_SLASH;
	    }
	  break;
	case STATE_DOT:
	  state = STATE_ADDR;
	  break;
	case STATE_SLASH:
	  if (*(str + 1) == '\0')
	    return partly_match;

	  state = STATE_MASK;
	  break;
	default:
	  break;
	}

      if (nums > 11)
	return no_match;

      if (colons > 7)
	return no_match;

      str++;
    }

  if (state < STATE_MASK)
    return partly_match;

  mask = strtol (str, &endptr, 10);
  if (*endptr != '\0')
    return no_match;

  if (mask < 0 || mask > 128)
    return no_match;
  
/* I don't know why mask < 13 makes command match partly.
   Forgive me to make this comments. I Want to set static default route
   because of lack of function to originate default in ospf6d; sorry
       yasu
  if (mask < 13)
    return partly_match;
*/

  return exact_match;
}

#define DECIMAL_STRLEN_MAX 10

int
cmd_range_match (char *range, char *str)
{
  char *p;
  char buf[DECIMAL_STRLEN_MAX + 1];
  char *endptr = NULL;
  unsigned long min, max, val;

  if (str == NULL)
    return 1;

  val = strtoul (str, &endptr, 10);
  if (*endptr != '\0')
    return 0;

  range++;
  p = strchr (range, '-');
  if (p == NULL)
    return 0;
  if (p - range > DECIMAL_STRLEN_MAX)
    return 0;
  strncpy (buf, range, p - range);
  buf[p - range] = '\0';
  min = strtoul (buf, &endptr, 10);
  if (*endptr != '\0')
    return 0;

  range = p + 1;
  p = strchr (range, '>');
  if (p == NULL)
    return 0;
  if (p - range > DECIMAL_STRLEN_MAX)
    return 0;
  strncpy (buf, range, p - range);
  buf[p - range] = '\0';
  max = strtoul (buf, &endptr, 10);
  if (*endptr != '\0')
    return 0;

  if (val < min || val > max)
    return 0;

  return 1;
}


/* Filter vector by command character with index. */
enum match_type
cmd_filter_by_string (char *command, vector v, int index)
{
  int i;
  char *str;
  struct cmd_element *cmd_element;
  enum match_type match_type;
  vector descvec;
  struct desc *desc;
  
  match_type = no_match;

  /* If command and cmd_element string does not match set NULL to vector */
  for (i = 0; i < vector_max (v); i++) 
    if ((cmd_element = vector_slot (v, i)) != NULL)
      {
	/* If given index is bigger than max string vector of command,
           set NULL*/
	if (index >= vector_max (cmd_element->strvec))
	  vector_slot (v, i) = NULL;
	else 
	  {
	    int j;
	    int matched = 0;

	    descvec = vector_slot (cmd_element->strvec, index);

	    for (j = 0; j < vector_max (descvec); j++)
	      {
		desc = vector_slot (descvec, j);
		str = desc->cmd;

		if (CMD_VARARG (str))
		  {
		    if (match_type < vararg_match)
		      match_type = vararg_match;
		    matched++;
		  }
		else if (CMD_RANGE (str))
		  {
		    if (cmd_range_match (str, command))
		      {
			if (match_type < range_match)
			  match_type = range_match;
			matched++;
		      }
		  }
		else if (CMD_IPV6 (str))
		  {
		    if (cmd_ipv6_match (command) == exact_match)
		      {
			if (match_type < ipv6_match)
			  match_type = ipv6_match;
			matched++;
		      }
		  }
		else if (CMD_IPV6_PREFIX (str))
		  {
		    if (cmd_ipv6_prefix_match (command) == exact_match)
		      {
			if (match_type < ipv6_prefix_match)
			  match_type = ipv6_prefix_match;
			matched++;
		      }
		  }
		else if (CMD_IPV4 (str))
		  {
		    if (cmd_ipv4_match (command) == exact_match)
		      {
			if (match_type < ipv4_match)
			  match_type = ipv4_match;
			matched++;
		      }
		  }
		else if (CMD_IPV4_PREFIX (str))
		  {
		    if (cmd_ipv4_prefix_match (command) == exact_match)
		      {
			if (match_type < ipv4_prefix_match)
			  match_type = ipv4_prefix_match;
			matched++;
		      }
		  }
		else if (CMD_OPTION (str) || CMD_VARIABLE (str))
		  {
		    if (match_type < extend_match)
		      match_type = extend_match;
		    matched++;
		  }
		else
		  {		  
		    if (strcmp (command, str) == 0)
		      {
			match_type = exact_match;
			matched++;
		      }
		  }
	      }
	    if (! matched)
	      vector_slot (v, i) = NULL;
	  }
      }
  return match_type;
}

/* Check ambiguous match */
int
vtysh_is_cmd_ambiguous (char *command, vector v, int index, enum match_type type)
{
  int i;
  int j;
  char *str = NULL;
  struct cmd_element *cmd_element;
  char *matched = NULL;
  vector descvec;
  struct desc *desc;
  
  for (i = 0; i < vector_max (v); i++) 
    if ((cmd_element = vector_slot (v, i)) != NULL)
      {
	int match = 0;
	descvec = vector_slot (cmd_element->strvec, index);

	for (j = 0; j < vector_max (descvec); j++)
	  {
	    enum match_type ret;

	    desc = vector_slot (descvec, j);
	    str = desc->cmd;
           if(!str)
            continue;

	    switch (type)
	      {
	      case exact_match:
		if (! (CMD_OPTION (str) || CMD_VARIABLE (str))
		    && strcmp (command, str) == 0)
		  match++;
		break;
	      case partly_match:
		if (! (CMD_OPTION (str) || CMD_VARIABLE (str))
		    && strncmp (command, str, strlen (command)) == 0)
		  {
		    if (matched && strcmp (matched, str) != 0)
		      return 1;	/* There is ambiguous match. */
		    else
		      matched = str;
		    match++;
		  }
		break;
	      case range_match:
		if (cmd_range_match (str, command))
		  {
		    if (matched && strcmp (matched, str) != 0)
		      return 1;
		    else
		      matched = str;
		    match++;
		  }
		break;
 	      case ipv6_match:
		if (CMD_IPV6 (str))
		  match++;
		break;
	      case ipv6_prefix_match:
		if ((ret = cmd_ipv6_prefix_match (command)) != no_match)
		  {
		    if (ret == partly_match)
		      return 2; /* There is incomplete match. */

		    match++;
		  }
		break;
	      case ipv4_match:
		if (CMD_IPV4 (str))
		  match++;
		break;
	      case ipv4_prefix_match:
		if ((ret = cmd_ipv4_prefix_match (command)) != no_match)
		  {
		    if (ret == partly_match)
		      return 2; /* There is incomplete match. */

		    match++;
		  }
		break;
	      case extend_match:
		if (CMD_OPTION (str) || CMD_VARIABLE (str))
		  match++;
		break;
             case option_match:
                 match++;
               break;
	      case no_match:
	      default:
		break;
	      }
	  } /* for */
	if (! match)
       {
	  vector_slot (v, i) = NULL;
          if(vtysh_cmd_debug) 
              printf("vector %d filtered by vtysh_is_cmd_ambiguous\n\r", i);
        }
      }
  return 0;
}

/* If src matches dst return dst string, otherwise return NULL */
char *
cmd_entry_function (char *src, char *dst)
{
  /* Skip variable arguments. */
  if (CMD_OPTION (dst) || CMD_VARIABLE (dst) || CMD_VARARG (dst) ||
      CMD_IPV4 (dst) || CMD_IPV4_PREFIX (dst) || CMD_RANGE (dst))
    return NULL;

  /* In case of 'command \t', given src is NULL string. */
  if (src == NULL)
    return dst;

  /* Matched with input string. */
  if (strncmp (src, dst, strlen (src)) == 0)
    return dst;

  return NULL;
}

/* If src matches dst return dst string, otherwise return NULL */
/* This version will return the dst string always if it is
   CMD_VARIABLE for '?' key processing */
char *
cmd_entry_function_desc (char *src, char *dst)
{
  if (CMD_VARARG (dst))
    return dst;

  if (CMD_RANGE (dst))
    {
      if (cmd_range_match (dst, src))
	return dst;
      else
	return NULL;
    }

  if (CMD_IPV6 (dst))
    {
      if (cmd_ipv6_match (src))
	return dst;
      else
	return NULL;
    }

  if (CMD_IPV6_PREFIX (dst))
    {
      if (cmd_ipv6_prefix_match (src))
	return dst;
      else
	return NULL;
    }

  if (CMD_IPV4 (dst))
    {
      if (cmd_ipv4_match (src))
	return dst;
      else
	return NULL;
    }

  if (CMD_IPV4_PREFIX (dst))
    {
      if (cmd_ipv4_prefix_match (src))
	return dst;
      else
	return NULL;
    }

  /* Optional or variable commands always match on '?' */
  if (CMD_OPTION (dst) || CMD_VARIABLE (dst))
    return dst;

  /* In case of 'command \t', given src is NULL string. */
  if (src == NULL)
    return dst;

  if (strncmp (src, dst, strlen (src)) == 0)
    return dst;
  else
    return NULL;
}

/* Check same string element existence.  If it isn't there return
    1. */
int
cmd_unique_string (vector v, char *str)
{
  int i;
  char *match;

  for (i = 0; i < vector_max (v); i++)
    if ((match = vector_slot (v, i)) != NULL)
      if (strcmp (match, str) == 0)
	return 0;
  return 1;
}

/* Compare string to description vector.  If there is same string
   return 1 else return 0. */
int
vtysh_desc_unique_string (vector v, char *str)
{
    int i;
    struct desc *desc;

    for (i = 0; i < vector_max (v); i++)
    {
        if ((desc = vector_slot (v, i)) != NULL)
        {
            if (strcmp (desc->cmd, str) == 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

#define INIT_MATCHVEC_SIZE 10
#define VECTOR_SET \
if(if_describe) \
{ \
    if (! vtysh_desc_unique_string (matchvec, string)) \
    { \
        vtysh_vector_set (matchvec, desc); \
    } \
} \
else \
{ \
    if (cmd_unique_string (matchvec, string)) \
    { \
        vtysh_vector_set (matchvec, XSTRDUP (MTYPE_TMP, string)); \
    } \
}

enum match_type cmd_string_match(char* str, char* command)
{
    enum match_type match_type = no_match;

    if (CMD_VARARG (str))
    {
        match_type = vararg_match;
    }
    else if (CMD_RANGE (str))
    {
        if (cmd_range_match (str, command))
        {
            match_type = range_match;
        }
    }
    else if (CMD_IPV6 (str))
    {
        if (cmd_ipv6_match (command))
        {
            match_type = ipv6_match;
        }
    }
    else if (CMD_IPV6_PREFIX (str))
    {
        if (cmd_ipv6_prefix_match (command))
        {
            match_type = ipv6_prefix_match;
        }
    }
    else if (CMD_IPV4 (str))
    {
        if (cmd_ipv4_match (command))
        {
            match_type = ipv4_match;
        }
    }
    else if (CMD_IPV4_PREFIX (str))
    {
        if (cmd_ipv4_prefix_match (command))
        {
            match_type = ipv4_prefix_match;
        }
    }
    else
    if (CMD_OPTION (str) || CMD_VARIABLE (str))
    {
            match_type = extend_match;
    }
    else if (strncmp (command, str, strlen (command)) == 0)
    {
        if (strcmp (command, str) == 0)
        { 
            match_type = exact_match;
        }
        else
        {
            match_type = partly_match;
        }
    }
    return match_type;             
}

enum match_type 
cmd_filter_command_tree(vector str_vec, vector vline, int *index, struct desc** vtysh_matched_desc_ptr, int depth, int* if_exact_match)
{
    int j = 0;
    char *str = NULL;
    enum match_type match_type = no_match;
    vector cur_vec = NULL;
    struct desc *desc = NULL;
    char *command = NULL;
    int old_index = 0;
    int k = 0;
    int no_option = 0;
 
    while(*index<vector_max(vline))
    {
        command = vector_slot (vline, *index);
        if(!command)
        {
            return option_match;
        }

        if(str_vec->is_desc)
        {
            if(str_vec->direction ==0) /* Tranverse */
            {
                for(j=0; j<vector_max(str_vec); j++)
                {
                    desc = vector_slot (str_vec, j);
                    str =  desc->cmd;
                    if((match_type = cmd_string_match(str, command)) == no_match)
                    {
                        return no_match;
                    }
                    else /* matched */
                    {
                        vtysh_matched_desc_ptr[*index] = desc;
                        (*index)++;
                        if(*index<vector_max(vline))
                        {
                            command = vector_slot (vline, *index);
                            if(!command) /* next is null */
                            {
                                return option_match;
                            }
                        }
                        else
                        {
                            j++;
                            break;
                        }
                    }
                }

                if(j <vector_max(str_vec))
                {
                    return incomplete_cmd;
                }

                return match_type;
            }
            else /* vertical */
            {
                for(j=0; j<vector_max(str_vec); j++)
                {
                    desc = vector_slot (str_vec, j);
                    str =  desc->cmd;
                    if((match_type = cmd_string_match(str, command)) == no_match)
                    {
                        continue;
                    }
                    else
                    {
                        vtysh_matched_desc_ptr[*index] = desc;
                        (*index)++;
                        break;
                    }
                }

                if(match_type == no_match )
                {
                    if(vector_max(str_vec)>1)
                    {
                        return no_match;
                    }
                    else /* if vetical vector and only has one element, it is optional */
                    {
                        return option_match;
                    }
                }
                else 
                {
                    return match_type;
                }
            }
        }
        else /* shall go to next level's vector */
        {
            if(str_vec->direction ==0) /* Tranverse */
            {
                for(j=0; j<vector_max(str_vec); j++)
                {
                    cur_vec = vector_slot (str_vec, j);
                    if(cur_vec->direction && vector_max(cur_vec)==1) /* optinal vector */
                    {
                        while(!cur_vec->is_desc)
                        {
                            cur_vec = vector_slot (cur_vec, 0);
                        }
                        desc = vector_slot (cur_vec, 0);
                        command = vector_slot (vline, *index);
                        if(command && CMD_VARIABLE(desc->cmd) && !CMD_NUMBER(command) && !CMD_VARIABLE(command)) /* skip if input is keyword but desc is VAR */
                        {
                            if(vtysh_cmd_debug)
                            {
                                printf("\n\rLine: %d, index=%d,  skip if input is keyword but desc is VAR", __LINE__, *index);
                            }
                            continue;
                        }
                    }
                    cur_vec = vector_slot (str_vec, j); /* retry to get the current vector */
                    if((match_type=cmd_filter_command_tree(cur_vec, vline, index, vtysh_matched_desc_ptr, depth+1, if_exact_match)) == no_match)
                    {
                        return no_match;
                    }/* else, matched, index will be increased and go on next match */
                    /* else, matched, index will be increased and go on next match */
                    if(*index>=vector_max(vline))
                    {
                        j++;
                        if(vtysh_cmd_debug)
                        {
                            printf("\n\rLine: %d, index=%d, j=%d: reach the end of input word", __LINE__, *index, j);
                        }
                        break;
                    }
                }

                no_option = 0;
                for(k=j; k<vector_max(str_vec); k++) /* check if all the left cmds in the tranverse list can be skipped */
                {
                    cur_vec = vector_slot (str_vec, k);
                    if(!cur_vec->direction || vector_max(cur_vec)>1) /* optional vector shall be vertical and has one cmd*/
                    {
                        no_option = 1;
                        break;
                    }
                }
                if((j <vector_max(str_vec) )&& no_option)
                {
                    return incomplete_cmd;
                }

                /* too many input words */
                if(depth==0 && *index!=vector_max(vline) && (command = vector_slot (vline, *index)))
                {
                    if(vtysh_cmd_debug)
                    {
                        printf("\n\rLine: %d, index=%d,  too more cmds", __LINE__, *index);
                    }
                    return no_match;
                }

                return match_type;
            }
            else /* Vertical */
            {
                int cbrace_matched = 0;
                int cbrace_try_result = 0;
                if(str_vec->is_multiple) /* cbrace: {a1 |a2 }*/
                {
                    char match_j[MAX_OPTIONAL_CMD_NUM] = {0};
                    if(vtysh_cmd_debug)
                    {
                        printf("\r\nLine %d: *index: %d, entering cbrace checking", __LINE__, *index);
                    }
                    do
                    {
                        cbrace_try_result = 0;
                        for(j=0; j<vector_max(str_vec); j++)
                        {
                            if(j>=MAX_OPTIONAL_CMD_NUM)
                            {
                                printf("\n\rLine: %d, index=%d,  too many optional cmds", __LINE__, *index);
                                break;
                            }
                            if(!match_j[j])
                            {
                                cur_vec = vector_slot (str_vec, j);
                                if((match_type=cmd_filter_command_tree(cur_vec, vline, index, vtysh_matched_desc_ptr, depth+1, if_exact_match)) == no_match)
                                {
                                    continue;
                                }
                                else
                                {
                                    match_j[j] = 1;
                                    cbrace_matched++;
                                    cbrace_try_result++;
                                    break;
                                }
                            }
                        }                        
                    } while(cbrace_try_result); /* if match none, shall exit loop */

                    if(cbrace_matched) /* {} only need match at least one command */
                    {
                        if(vtysh_cmd_debug)
                        {
                            printf("\r\ncbrace_matched: Line %d: *index: %d, command: %s, j: %d", __LINE__, *index, command, j);
                        }
                        return option_match;
                    }
                    else
                    {
                        if(vtysh_cmd_debug)
                        {
                            printf("\r\nNone cbrace matched in Line %d: *index: %d, command: %s, j: %d", __LINE__, *index, command, j);
                        }
                    }
                }
                else /* paren: (a1 |a2 ) */
                {
                    int matched_j = -1;
                    enum match_type previous_match_type = no_match;
                    old_index = *index;
                    for(j=0; j<vector_max(str_vec); j++) /* try to get best match in the paren list */
                    {
                        cur_vec = vector_slot (str_vec, j);
                        *index = old_index;
                        if(!cur_vec->is_desc)
                        {
                             match_type = cmd_filter_command_tree(cur_vec, vline, index, vtysh_matched_desc_ptr, depth+1, if_exact_match); 
                        }
                        else
                        {
                            desc = vector_slot (cur_vec, 0);
                            str =  desc->cmd;
                            command = vector_slot (vline, *index);
                            match_type = cmd_string_match(str, command);
                        }
                        if(match_type>previous_match_type)
                        {
                            previous_match_type = match_type;
                            matched_j = j;
                        }
                    }

                    if(previous_match_type != no_match) /* found best match */
                    {
                        cur_vec = vector_slot (str_vec, matched_j);
                        *index = old_index;
                        match_type=cmd_filter_command_tree(cur_vec, vline, index, vtysh_matched_desc_ptr, depth+1, if_exact_match); 
                        if(vtysh_cmd_debug)
                        {
                            printf("\r\nLine %d: *index: %d, Found best match %d, returned type: %d",  __LINE__, *index, matched_j, match_type);
                        }
                        return match_type;
                    }
                    else /* no match */
                    {
                        if(vector_max(str_vec)>1)
                        {
                            return no_match;
                        }
                        else  /* if vertical vector only has one element, it is optional */
                        {
                            return option_match;
                        }
                    }
                }

                return match_type;
            }
        }
    } /* while */

    return match_type;
}

enum match_type
cmd_filter_by_completion (vector strvec, vector vline, struct desc** vtysh_matched_desc_ptr, int *if_exact_match)
{
    int index = 0;
    return cmd_filter_command_tree(strvec, vline, &index, vtysh_matched_desc_ptr, 0, if_exact_match);
}

static struct desc desc_cr = { "<cr>", "" };
/*returns: 0 no match; 1 matched but not last word, continue searching; 2 match and last word, finish searching */
int cmd_describe_cmd_tree(vector vline, int* index, vector str_vec, vector matchvec, int if_describe, int depth)
{
    int j = 0;
    int ret = 0;
    char *str = NULL;
    enum match_type match_type = no_match;
    vector cur_vec = NULL;
    struct desc *desc = NULL;
    char* command = NULL;
    char *string = NULL;
    int old_index  = 0;
      
    while(*index<vector_max(vline))
    {
        command = vector_slot (vline, *index);

        if(str_vec->is_desc)
        {
            if(str_vec->direction ==0) /* Tranverse */
            {
                int if_exact_match = 0;
                for(j=0; j<vector_max(str_vec); j++)
                {
                    command = vector_slot (vline, *index);
                    desc = vector_slot (str_vec, j);
                    str =  desc->cmd;

                    if(command) /* not null command */
                    {
                        if((match_type = cmd_string_match(str, command)) == no_match)
                        {
                            return 0;
                        }
                        else /* matched */
                        {
                            if(*index == (vector_max(vline)-1)) /* command is last string*/
                            {
                                string = cmd_entry_function_desc (command, desc->cmd);
                                if (string)
                                {
                                    VECTOR_SET;
                                }
                                if(vtysh_cmd_debug)
                                {
                                    printf("\r\nLine %d:depth: %d, *index: %d, string: %s, j: %d", __LINE__, depth, *index, string, j);
                                }
                                return 2; /* not null, last word match */
                            }
                            else /* not null, not last word */
                            {
                                (*index)++;
                                command = vector_slot (vline, *index);      
                                if(vtysh_cmd_debug)
                                {
                                    printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d", __LINE__, depth, *index, command, j);
                                }
                                if(partly_match != match_type)
                                {
                                    if_exact_match= 1; /* exact match */
                                }
                            }
                        }
                    }
                    else /* command is null, always the last word */
                    {
                        string = cmd_entry_function_desc (command, desc->cmd);
                        if (string)
                        {
                            VECTOR_SET;
                        }
                        if(vtysh_cmd_debug)
                        {
                            printf("\r\nLine %d:depth: %d, *index: %d, string: %s, j: %d", __LINE__, depth, *index, string, j);
                        }                        
                        return 2;
                    }
                }
                if(vtysh_cmd_debug)
                {
                    printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d", __LINE__, depth, *index, command, j);
                }

                return 1;
            }
            else /* vertical */
            {
                command = vector_slot (vline, *index);
                if(command) /* not null */
                {
                    if(*index == (vector_max(vline)-1)) /* command is last string */
                    {
                        for(j=0; j<vector_max(str_vec); j++)
                        {
                            desc = vector_slot (str_vec, j);
                            str =  desc->cmd;
                            if((match_type = cmd_string_match(str, command)) != no_match)
                            {
                                string = cmd_entry_function_desc (command, desc->cmd);
                                if (string)
                                {
                                    VECTOR_SET;
                                } 
                                if(vtysh_cmd_debug)
                                {
                                    printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d", __LINE__, depth, *index, command, j);
                                }
                                return 2; /* shall match only one */
                            }
                        } /* for j */
                    }
                    else /* command not last word */
                    {
                        for(j=0; j<vector_max(str_vec); j++)
                        {
                            desc = vector_slot (str_vec, j);
                            str =  desc->cmd;
                            if((match_type = cmd_string_match(str, command)) != no_match)
                            {   
                                if(vtysh_cmd_debug)
                                {
                                    printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d", __LINE__, depth, *index, command, j);
                                }
                                return 1; /* shall match only one */
                            }
                        } /* for j */
                    }
                }
                else /*  last string, null command */
                {
                    for(j=0; j<vector_max(str_vec); j++)
                    {
                        desc = vector_slot (str_vec, j);
                        str =  desc->cmd;
                        string = cmd_entry_function_desc (command, desc->cmd);
                        if (string)
                        {
                            VECTOR_SET;
                        }
                    } /* for j */
                    if(vtysh_cmd_debug)
                    {
                        printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d", __LINE__, depth, *index, command, j);
                    }
                    return 2;
                }

                if(vtysh_cmd_debug)
                {
                    printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d", __LINE__, depth, *index, command, j);
                }

                return 0;
            }
        }
        else /* shall go to next level's vector */
        {
            if(str_vec->direction ==0) /* Tranverse */
            {
                for(j=0; j<vector_max(str_vec); j++)
                {
                    cur_vec = vector_slot (str_vec, j);
                    if(cur_vec->direction && vector_max(cur_vec)==1) /* optinal vector */
                    {
                        while(!cur_vec->is_desc)
                        {
                            cur_vec = vector_slot (cur_vec, 0);
                        }
                        desc = vector_slot (cur_vec, 0);
                        command = vector_slot (vline, *index);
                        if(command && CMD_VARIABLE(desc->cmd) && !CMD_NUMBER(command) && !CMD_VARIABLE(command)) /* skip if input is keyword but desc is VAR */
                        {
                            continue;
                        }
                    }

                    cur_vec = vector_slot (str_vec, j);/* retry to get the current vector */
                    old_index = *index;
                    ret = cmd_describe_cmd_tree(vline, index, cur_vec, matchvec, if_describe, depth+1);
                    if(ret==2)
                    {
                        if(cur_vec->direction && vector_max(cur_vec)==1 && (old_index == *index)) /* optional vector */
                        {/*(old_index == *index) means index is not increased in the optional vector */
                            continue;
                        }
                        if(vtysh_cmd_debug)
                        {
                            printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d", __LINE__, depth, *index, command, j);
                        }                    
                        return 2;
                    }
                    
                    if(ret ==3)
                    {
                        return 3;
                    }

                    if(ret==0)
                    {
                        if(vtysh_cmd_debug)
                        {
                            printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d ", __LINE__, depth, *index, command, j);
                        }                        
                        return 0;
                    }
                }

                if(!depth && (j==vector_max(str_vec)) && ((command = vector_slot (vline, *index)) == NULL) ) /* all tranverse vector has been searched */
                {
                    if(vtysh_cmd_debug)
                    {
                        printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d ", __LINE__, depth, *index, command, j);
                    }  
                    string = "<cr>";

                    if(if_describe)
                    {
                        if(!vtysh_desc_unique_string (matchvec, string))
                            vtysh_vector_set (matchvec, &desc_cr);
                    }
                    else
                    {
                        if(cmd_unique_string(matchvec, string))
                            vtysh_vector_set(matchvec, XSTRDUP (MTYPE_TMP, desc_cr.cmd));
                    }
                }
                if(vtysh_cmd_debug)
                {
                    printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d", __LINE__, depth, *index, command, j);
                } 
                return 1;
            }
            else /* Vertical */
            {
                if(str_vec->is_multiple) /* {a1|a2} */
                {
                    char match_j[100] = {0};
                    int  cbrace_try_result = 0;
                    int cbrace_matched = 0;
                    do
                    {
                        cbrace_try_result = 0;
                        for(j=0; j<vector_max(str_vec); j++)
                        {
                            cur_vec = vector_slot (str_vec, j);
                            command = vector_slot (vline, *index);
                            if(!command) /* it's time to match NULL */
                            {
                                break;
                            }

                            if(!match_j[j]) /* find those not searched */
                            {
                                if(*index==(vector_max(vline)-1))/* last word */
                                {
                                    ret = cmd_describe_cmd_tree(vline, index, cur_vec, matchvec, if_describe, depth+1);                            
                                    if(ret)
                                    {
                                        if(vtysh_cmd_debug)
                                        {
                                            printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d, ret: %d", __LINE__, depth, *index, command, j, ret);
                                        }    
                                        return ret;
                                    }
                                }
                                else /* not last word */
                                {
                                    old_index = *index;
                                    ret =cmd_describe_cmd_tree(vline, index, cur_vec, matchvec, if_describe, depth+1);
                                    if(ret)
                                    {
                                        match_j[j] = 1; /* matched */
                                        cbrace_matched++;
                                        cbrace_try_result++;
                                        command = vector_slot (vline, *index);
                                        if((!command || vector_max(cur_vec)>1)&& (ret ==2)) /* "a1 A1" format in one of the list */
                                        {
                                            return 3;
                                        }
                                    }
                                    else
                                    {
                                        if(*index >old_index) /* inner "a1 A1" format but no match */
                                        {
                                            return 0;
                                        }
                                    }
                                }
                            }
                        }                        
                    } while(cbrace_try_result); /* if match none, shall exit loop */

                    if(!command)
                    {
                        for(j=0; j<vector_max(str_vec); j++)
                        {
                            cur_vec = vector_slot (str_vec, j);
                            if(!match_j[j])
                            {
                                ret =cmd_describe_cmd_tree(vline, index, cur_vec, matchvec, if_describe, depth+1);
                            }
                        }  
                    }

                    if(vtysh_cmd_debug)
                    {
                        printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d, ret: %d", __LINE__, depth, *index, command, j, ret);
                    } 
                    if(cbrace_matched)
                    {
                        return 1;
                    }
                    else
                    {
                        return ret;
                    }
                } /* end of {} */
                else /* (a1|a2) */
                {
                    if(*index!=(vector_max(vline)-1)) /* not last word */
                    {
                        int matched_j = -1;
                        enum match_type previous_match_type = no_match;
                        old_index = *index;
                        for(j=0; j<vector_max(str_vec); j++)
                        {
                            *index = old_index;
                            cur_vec = vector_slot (str_vec, j);
                            if(!cur_vec->is_desc)
                            {
                                match_type = cmd_describe_cmd_tree(vline, index, cur_vec, matchvec, if_describe, depth+1);
                            }
                            else
                            {
                                desc = vector_slot (cur_vec, 0);
                                str =  desc->cmd;
                                command = vector_slot (vline, *index);
                                match_type = cmd_string_match(str, command);
                            }
                            if(match_type>previous_match_type)
                            {
                                previous_match_type = match_type;
                                matched_j = j;
                            }
                        }
                        if(previous_match_type != no_match) /* found best match*/
                        {
                            cur_vec = vector_slot (str_vec, matched_j);
                            *index = old_index;
                            ret = cmd_describe_cmd_tree(vline, index, cur_vec, matchvec, if_describe, depth+1);
                        }
                        else /* all list not matched*/
                        {
                            ret = 0;
                        }
                    }
                    else /*last word, can be null */
                    {
                        int if_matched = 0;
                        for(j=0; j<vector_max(str_vec); j++)
                        {
                            cur_vec = vector_slot (str_vec, j);
                            ret = cmd_describe_cmd_tree(vline, index, cur_vec, matchvec, if_describe, depth+1);
                            if(ret)
                            {
                                if_matched = ret;
                            }  
                        }
                        ret = if_matched;
                    }

                    if(ret==0 && vector_max(str_vec) == 1) /* optional vector, can skill matching NULL command */
                    {
                        return 1;
                    }
                    if(vtysh_cmd_debug)
                    {
                        printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d, ret: %d", __LINE__, depth, *index, command, j, ret);
                    }

                    return ret;
                }
            }
        }
    } /* while */
    if(vtysh_cmd_debug)
    {
        printf("\r\nLine %d:depth: %d, *index: %d, command: %s, j: %d \n\r", __LINE__, depth, *index, command, j);
    }
    return ret;
}


vector cmd_describe_complete_cmd(vector vline, vector cmd_vector, vector matchvec, int if_describe)
{
    int i = 0;
    int index = 0;
    struct cmd_element *cmd_element = NULL;

    for (i = 0; i < vector_max (cmd_vector); i++)
    {
        index = 0;
        if ((cmd_element = vector_slot (cmd_vector, i)) != NULL)
        {
            cmd_describe_cmd_tree(vline, &index, cmd_element->strvec, matchvec, if_describe, 0);
        }
    }

    return matchvec;
}

/* '?' describe command support. */
vector
cmd_describe_command (vector vline, struct vty *vty, int *status)
{
    int i;
    int if_exact_match = 0;

    vector cmd_vector;
    vector matchvec;
    enum match_type match;
    struct cmd_element *cmd_element = NULL;

    int best_match_type = 0;
    unsigned short matched_count[3] = {0};
    char* partly_match_element = NULL;
    char* extend_match_element = NULL;

    /* Make copy vector of current node's command vector. */
    cmd_vector = vtysh_vector_copy (cmd_node_vector (vtysh_cmdvec, vty->node));
    if(!cmd_vector)
    {
        *status = CMD_SYS_ERROR;
        return NULL;
    }

    /* Prepare match vector */
    matchvec = vtysh_vector_init (INIT_MATCHVEC_SIZE);

    partly_match_element = (char*) malloc(sizeof(char)*MAX_ELEMENT_NUM);
    extend_match_element = (char*) malloc(sizeof(char)*MAX_ELEMENT_NUM);

    if(!partly_match_element || !extend_match_element)
    {
        printf("\n\rError: no memory!!");
    }
    memset(partly_match_element, 0, sizeof(char)*MAX_ELEMENT_NUM);
    memset(extend_match_element, 0, sizeof(char)*MAX_ELEMENT_NUM);

    /* filter command elements */
    if(vector_slot(vline, 0)!=NULL)
    {
        for(i=0; i<vector_max(cmd_vector); i++)
        {
            match = 0;
            if_exact_match = 1;
            cmd_element = vector_slot (cmd_vector, i);
            if(cmd_element)
            {
                match = cmd_filter_by_completion(cmd_element->strvec, vline, vtysh_matched_desc_ptr, &if_exact_match);
                if (! match)
                {
                    vector_slot (cmd_vector, i) = NULL;
                    /*
                    if(vtysh_cmd_debug)
                    {
                        printf("cmd element %d filtered \n\r", i);
                    }*/
                }
                else /* matched, save the exact match element*/
                {
                    best_match_type = cmd_best_match_check(vline, vtysh_matched_desc_ptr, 1);
                    matched_count[best_match_type]++;
                    if(best_match_type == cmd_partly_match)
                    {
                        partly_match_element[i] = 1;
                        extend_match_element[i] = 0;
                    }
                    else if(best_match_type == cmd_extend_match)
                    {
                        extend_match_element[i] = 1;
                        partly_match_element[i] = 0;
                    }
                    else
                    {
                        extend_match_element[i] = 0;
                        partly_match_element[i] = 0;
                    }
                    if(vtysh_cmd_debug)
                    {
                        printf("cmd element %d best matched %d: %s \n\r", i, best_match_type, cmd_element->string);
                    }
                }
            }
        }/* for cmd filtering */
    }

    if(matched_count[cmd_exact_match]) /* found exact match, filter all partly and extend match elements */
    {
        for(i=0; i<vector_max(cmd_vector); i++)
        {
            if(extend_match_element[i] || partly_match_element[i]) /* filter all other elements */
            {
                vector_slot (cmd_vector, i) = NULL;
                if(vtysh_cmd_debug)
                {
                    printf("cmd element %d filterd for not exact match \n\r", i);
                }
            }
        }
    }
    else if(matched_count[cmd_partly_match]) /* found partly match, filter all extend match elements */
    {
        for(i=0; i<vector_max(cmd_vector); i++)
        {
            if(extend_match_element[i]) /* filter all other elements */
            {
                vector_slot (cmd_vector, i) = NULL;
                if(vtysh_cmd_debug)
                {
                    printf("cmd element %d filterd for not exact match \n\r", i);
                }
            }
        }
    }

    free(partly_match_element);
    free(extend_match_element);

    /* make desc vector */
    matchvec = cmd_describe_complete_cmd(vline, cmd_vector, matchvec, 1);

    vtysh_vector_free (cmd_vector);

    if (vector_slot (matchvec, 0) == NULL)
    {
        vtysh_vector_free (matchvec);
        *status= CMD_ERR_NO_MATCH;
    }
    else
    {
        *status = CMD_SUCCESS;
    }

    return matchvec;
}


/* Check LCD of matched command. */
int
cmd_lcd (char **matched)
{
  int i;
  int j;
  int lcd = -1;
  char *s1, *s2;
  char c1, c2;

  if (matched[0] == NULL || matched[1] == NULL)
    return 0;

  for (i = 1; matched[i] != NULL; i++)
    {
      s1 = matched[i - 1];
      s2 = matched[i];

      for (j = 0; (c1 = s1[j]) && (c2 = s2[j]); j++)
	if (c1 != c2)
	  break;

      if (lcd < 0)
	lcd = j;
      else
	{
	  if (lcd > j)
	    lcd = j;
	}
    }
  return lcd;
}

/* Command line completion support. */
char **
cmd_complete_command (vector vline, struct vty *vty, int *status)
{
    int i = 0;
    int if_exact_match = 0;
    int index = vector_max (vline) -1;
    int lcd = 0;
    vector cmd_vector = NULL;
    vector matchvec = NULL;
    struct cmd_element *cmd_element = NULL;
    enum match_type match = 0;
    char **match_str = NULL;

    int best_match_type = 0;
    unsigned short matched_count[3] = {0};
    char* partly_match_element = NULL;
    char* extend_match_element = NULL;

    if(vector_slot(vline, 0)==NULL)
    {
        *status = CMD_ERR_NOTHING_TODO;
        return match_str;
    }

    partly_match_element = (char*) malloc(sizeof(char)*MAX_ELEMENT_NUM);
    extend_match_element = (char*) malloc(sizeof(char)*MAX_ELEMENT_NUM);
    if( !partly_match_element || !extend_match_element)
    {
        printf("Error: no memory!!\n\r");
        return NULL;
    }

    /* Make copy of command elements. */
    cmd_vector = vtysh_vector_copy (cmd_node_vector (vtysh_cmdvec, vty->node));
    if(!cmd_vector)
    {
        printf("Error: no memory!!\n\r");
        return NULL;
    }

    /* filter command elements */

    for(i=0; i<vector_max(cmd_vector); i++)
    {
        match = 0;
        cmd_element = vector_slot (cmd_vector, i);
        if_exact_match = 1;
        if(cmd_element)
        {
            match = cmd_filter_by_completion(cmd_element->strvec, vline, vtysh_matched_desc_ptr, &if_exact_match);
            if (! match)
            {
                vector_slot (cmd_vector, i) = NULL;
                if(vtysh_cmd_debug)
                {
                    printf("cmd element %d filtered \n\r", i);
                }
            }
            else
            {
                best_match_type = cmd_best_match_check(vline, vtysh_matched_desc_ptr, 1);
                matched_count[best_match_type]++;
                if(best_match_type == cmd_partly_match)
                {
                    partly_match_element[i] = 1;
                    extend_match_element[i] = 0;
                }
                else if(best_match_type == cmd_extend_match)
                {
                    extend_match_element[i] = 1;
                    partly_match_element[i] = 0;
                }
                else
                {
                    extend_match_element[i] = 0;
                    partly_match_element[i] = 0;
                }

                if(vtysh_cmd_debug)
                {
                    printf("cmd element %d best match %d: %s \n\r", i, best_match_type, cmd_element->string);
                }
            }
        }
    }/* for cmd filtering */

    if(matched_count[cmd_exact_match]) /* found exact match, filter all partly and extend match elements */
    {
        for(i=0; i<vector_max(cmd_vector); i++)
        {
            if(extend_match_element[i] || partly_match_element[i]) /* filter all other elements */
            {
                vector_slot (cmd_vector, i) = NULL;
                if(vtysh_cmd_debug)
                {
                    printf("cmd element %d filterd for not exact match \n\r", i);
                }
            }
        }
    }
    else if(matched_count[cmd_partly_match]) /* found partly match, filter all extend match elements */
    {
        for(i=0; i<vector_max(cmd_vector); i++)
        {
            if(extend_match_element[i]) /* filter all other elements */
            {
                vector_slot (cmd_vector, i) = NULL;
                if(vtysh_cmd_debug)
                {
                    printf("cmd element %d filterd for not exact match \n\r", i);
                }
            }
        }
    }

    free(partly_match_element);
    free(extend_match_element);

    /* Prepare match vector. */
    matchvec = vtysh_vector_init (INIT_MATCHVEC_SIZE);
    if(!matchvec)
    {
        *status = CMD_WARNING;
        return NULL;
    }
    matchvec = cmd_describe_complete_cmd(vline, cmd_vector, matchvec, 0);

    /* We don't need cmd_vector any more. */
    vtysh_vector_free (cmd_vector);

    /* No matched command */
    if (vector_slot (matchvec, 0) == NULL)
    {
        vtysh_vector_free (matchvec);

        /* In case of 'command \t' pattern.  Do you need '?' command at
         the end of the line. */
        if (vector_slot (vline, index) == '\0')
            *status = CMD_ERR_NOTHING_TODO;
        else
            *status = CMD_ERR_NO_MATCH;
        return NULL;
    }

    /* Only one matched */
    if (vector_slot (matchvec, 1) == NULL)
    {
        match_str = (char **) matchvec->index;
        vtysh_vector_only_wrapper_free (matchvec);
        if( (strcmp(match_str[0], "<cr>") ==0) || CMD_VARIABLE(match_str[0]) ) /* if only cr or VAR matched, dont show it*/
        {
            XFREE (MTYPE_TMP, match_str);
            *status = CMD_ERR_NOTHING_TODO;
            return NULL;
        }
        *status = CMD_COMPLETE_FULL_MATCH;
        return match_str;
    }

    /* Make it sure last element is NULL. */
    vtysh_vector_set (matchvec, NULL);

    /* Check LCD of matched strings. */
    if (vector_slot (vline, index) != NULL)
    {
        lcd = cmd_lcd ((char **) matchvec->index);

        if (lcd)
        {
            int len = strlen (vector_slot (vline, index));

            if (len < lcd)
            {
                char *lcdstr;

                lcdstr = XMALLOC (MTYPE_VECTOR, lcd + 1);
                memcpy (lcdstr, matchvec->index[0], lcd);
                lcdstr[lcd] = '\0';

                /* match_str = (char **) &lcdstr; */

                /* Free matchvec. */
                for (i = 0; i < vector_max (matchvec); i++)
                {
                    if (vector_slot (matchvec, i))
                    XFREE (MTYPE_TMP, vector_slot (matchvec, i));
                }
                vtysh_vector_free (matchvec);

                /* Make new matchvec. */
                matchvec = vtysh_vector_init (INIT_MATCHVEC_SIZE);
                vtysh_vector_set (matchvec, lcdstr);
                match_str = (char **) matchvec->index;
                vtysh_vector_only_wrapper_free (matchvec);

                *status = CMD_COMPLETE_MATCH;
                return match_str;
            }
        }
    }

    match_str = (char **) matchvec->index;
    vtysh_vector_only_wrapper_free (matchvec);
    *status = CMD_COMPLETE_LIST_MATCH;
    return match_str;

}

enum match_type cmd_is_cmd_incomplete(vector str_vec, vector vline, struct desc** vtysh_matched_desc_ptr, int *if_exact_match)
{
    int index = 0;
    enum match_type match = 0;
    match = cmd_filter_command_tree(str_vec, vline, &index, vtysh_matched_desc_ptr, 0, if_exact_match);

    return match;
}



/* Execute command by argument vline vector. */
int
cmd_execute_command_vtysh_client (vector vline, struct vty *vty, struct cmd_element **cmd)
{
    int i = 0;
    int if_exact_match = 0;
    int best_match_type = 0;
    vector cmd_vector = NULL;
    struct cmd_element *cmd_element = NULL;
    struct cmd_element *matched_element = NULL;
    unsigned short matched_count[4] = {0};
    int matched_index[4] = {0};
    int argc;
    char *argv[CMD_ARGC_MAX];

    enum match_type match = 0;

    /* Make copy of command elements. */
    cmd_vector = vtysh_vector_copy (cmd_node_vector (vtysh_cmdvec, vty->node));
    if(!cmd_vector)
    {
        return CMD_SYS_ERROR;
    }

    /* filter command elements */
    for(i=0; i<vector_max(cmd_vector); i++)
    {
        match = 0;
        cmd_element = vector_slot (cmd_vector, i);
        if(cmd_element)
        {
            match = cmd_filter_by_completion(cmd_element->strvec, vline, vtysh_matched_desc_ptr, &if_exact_match);
            if (! match)
            {
                vector_slot (cmd_vector, i) = NULL;
                if(vtysh_cmd_debug)
                {
                    printf("cmd: %d: filtered \n\r", i);
                }
            }
            else
            {
                if(vtysh_cmd_debug)
                {
                    printf("cmd: %d matched type: %d: %s \n\r", i, match, cmd_element->string);
                }
                if(incomplete_cmd == cmd_is_cmd_incomplete(cmd_element->strvec, vline, vtysh_matched_desc_ptr, &if_exact_match))
                {
                    matched_count[cmd_imcomplete_match]++;
                }
                else
                {
                    best_match_type = cmd_best_match_check(vline, vtysh_matched_desc_ptr, 0);
                    matched_index[best_match_type] = i;
                    matched_count[best_match_type]++;
                }
            }
        }
    }
    
    if(!matched_count[cmd_exact_match]  &&!matched_count[cmd_partly_match] 
        &&!matched_count[cmd_extend_match] &&! matched_count[cmd_imcomplete_match])
    {
        vtysh_vector_free (cmd_vector);
        return CMD_ERR_NO_MATCH;
    }

    if(matched_count[cmd_imcomplete_match] &&!matched_count[cmd_exact_match]  &&!matched_count[cmd_partly_match] 
        &&!matched_count[cmd_extend_match] )
    {
        vtysh_vector_free (cmd_vector);
        return CMD_ERR_INCOMPLETE;
    }

    if(matched_count[cmd_exact_match]>1 || matched_count[cmd_partly_match] >1 || matched_count[cmd_extend_match] >1) /* exact match found, can be 1 or more */
    {
        vtysh_vector_free (cmd_vector);
        return CMD_ERR_AMBIGUOUS;
    }

    if(matched_count[cmd_exact_match]) /* single match */
    {
        matched_element =  vector_slot (cmd_vector, matched_index[cmd_exact_match]); 
    }
    else if(matched_count[cmd_partly_match])
    {
        matched_element =  vector_slot (cmd_vector, matched_index[cmd_partly_match]); 
    }
    else
    {
        matched_element =  vector_slot (cmd_vector, matched_index[cmd_extend_match]); 
    }

    vtysh_vector_free (cmd_vector);

    /*retry to get new desc */
    cmd_is_cmd_incomplete(matched_element->strvec, vline, vtysh_matched_desc_ptr, &if_exact_match);

    /* Argument treatment */
    argc = 0;
    for (i = 0; i < vector_max (vline); i++)
    {
        struct desc *desc = vtysh_matched_desc_ptr [i];
        if(desc->is_arg)
        {
            if(!CMD_VARIABLE(desc->cmd)) /* keywords, use origina, user input can be partiall */
            {
                char* cp = vector_slot (vline, i);
                if(cp)
                {
                    cp = XREALLOC (MTYPE_VECTOR, cp, strlen(desc->cmd)+1);
                    vector_slot (vline, i) = cp; /* cp changed,  must be freed*/
                    memcpy(cp, desc->cmd, strlen(desc->cmd));
                    cp[strlen(desc->cmd)] = '\0';
                }  
            }
            argv[argc++] = vector_slot (vline, i);
        }

        if (argc >= CMD_ARGC_MAX)
        {
            return CMD_ERR_EXEED_ARGC_MAX;
        }
    }

    /* For vtysh execution. */
    if (cmd)
        *cmd = matched_element;

    if(vtysh_cmd_arg_debug)
    {
        printf("argc=%d, argv= \n\r", argc);
        for(i=0; i<argc; i++)
        {
            printf("%s ", argv[i]);
        }
        printf("\n\r");
    }

    if (matched_element && matched_element->daemon)
    {
        return CMD_SUCCESS_DAEMON;
    }

    /* Execute matched command. */
    return (*matched_element->func) (matched_element, vty, argc, argv);
}

/* Execute command by argument vline vector. */
int
cmd_execute_command (vector vline, struct vty *vty, struct cmd_element **cmd)
{
    int i = 0;
    int if_exact_match = 0;
    int best_match_type = 0;
    vector cmd_vector = NULL;
    struct cmd_element *cmd_element = NULL;
    struct cmd_element *matched_element = NULL;
    unsigned short matched_count[4] = {0};
    int matched_index[4] = {0};
    int argc;
    char *argv[CMD_ARGC_MAX];

    enum match_type match = 0;

    /* Make copy of command elements. */
/* modified by cuixl for support vtysh in gb, 2013-04-13 */
/* in vtysh client, we don't care cli mode, just send cli string to vtysh server*/
/* in vtysh server, we interate sdk/cmodel/debug mode */
#ifdef _CENTEC_
    int node = 0;
    for (node = 0; node < vtysh_cmdvec->max; node++ )
    {
        cmd_element = NULL;
        matched_element = NULL;
        if (!vector_slot(vtysh_cmdvec, node))
            continue;
        
        cmd_vector = vtysh_vector_copy (cmd_node_vector (vtysh_cmdvec, node));

        if (!cmd_vector)
            continue;
#else    
    cmd_vector = vtysh_vector_copy (cmd_node_vector (vtysh_cmdvec, vty->node));
    if(!cmd_vector)
    {
        return CMD_SYS_ERROR;
    }
#endif /*_CENTEC_*/

    /* filter command elements */
    for(i=0; i<vector_max(cmd_vector); i++)
    {
        match = 0;
        cmd_element = vector_slot (cmd_vector, i);
        if(cmd_element)
        {
            match = cmd_filter_by_completion(cmd_element->strvec, vline, vtysh_matched_desc_ptr, &if_exact_match);
            if (! match)
            {
                vector_slot (cmd_vector, i) = NULL;
                if(vtysh_cmd_debug)
                {
                    printf("cmd: %d: filtered \n\r", i);
                }
            }
            else
            {
                if(vtysh_cmd_debug)
                {
                    printf("cmd: %d matched type: %d: %s \n\r", i, match, cmd_element->string);
                }
                if(incomplete_cmd == cmd_is_cmd_incomplete(cmd_element->strvec, vline, vtysh_matched_desc_ptr, &if_exact_match))
                {
                    matched_count[cmd_imcomplete_match]++;
                }
                else
                {
                    best_match_type = cmd_best_match_check(vline, vtysh_matched_desc_ptr, 0);
                    matched_index[best_match_type] = i;
                    matched_count[best_match_type]++;
                }
            }
        }
    }
    
    if(!matched_count[cmd_exact_match]  &&!matched_count[cmd_partly_match] 
        &&!matched_count[cmd_extend_match] &&! matched_count[cmd_imcomplete_match])
    {
        vtysh_vector_free (cmd_vector);
/* modified by cuixl for support vtysh in gb, 2013-04-13 */
#ifdef _CENTEC_
        continue;
#else        
        return CMD_ERR_NO_MATCH;
#endif /*_CENTEC_*/                
    }

    if(matched_count[cmd_imcomplete_match] &&!matched_count[cmd_exact_match]  &&!matched_count[cmd_partly_match] 
        &&!matched_count[cmd_extend_match] )
    {
        vtysh_vector_free (cmd_vector);
/* modified by cuixl for support vtysh in gb, 2013-04-13 */
#ifdef _CENTEC_
        continue;
#else        
        return CMD_ERR_INCOMPLETE;
#endif /*_CENTEC_*/                        
    }

    if(matched_count[cmd_exact_match]>1 || matched_count[cmd_partly_match] >1 || matched_count[cmd_extend_match] >1) /* exact match found, can be 1 or more */
    {
        vtysh_vector_free (cmd_vector);
/* modified by cuixl for support vtysh in gb, 2013-04-13 */
#ifdef _CENTEC_
        continue;
#else          
        return CMD_ERR_AMBIGUOUS;
#endif /*_CENTEC_*/              
    }

    if(matched_count[cmd_exact_match]) /* single match */
    {
        matched_element =  vector_slot (cmd_vector, matched_index[cmd_exact_match]); 
    }
    else if(matched_count[cmd_partly_match])
    {
        matched_element =  vector_slot (cmd_vector, matched_index[cmd_partly_match]); 
    }
    else
    {
        matched_element =  vector_slot (cmd_vector, matched_index[cmd_extend_match]); 
    }

    vtysh_vector_free (cmd_vector);

    /*retry to get new desc */
    cmd_is_cmd_incomplete(matched_element->strvec, vline, vtysh_matched_desc_ptr, &if_exact_match);

    /* Argument treatment */
    argc = 0;
    for (i = 0; i < vector_max (vline); i++)
    {
        struct desc *desc = vtysh_matched_desc_ptr [i];
        if(desc->is_arg)
        {
            if(!CMD_VARIABLE(desc->cmd)) /* keywords, use origina, user input can be partiall */
            {
                char* cp = vector_slot (vline, i);
                if(cp)
                {
                    cp = XREALLOC (MTYPE_VECTOR, cp, strlen(desc->cmd)+1);
                    vector_slot (vline, i) = cp; /* cp changed,  must be freed*/
                    memcpy(cp, desc->cmd, strlen(desc->cmd));
                    cp[strlen(desc->cmd)] = '\0';
                }  
            }
            argv[argc++] = vector_slot (vline, i);
        }

        if (argc >= CMD_ARGC_MAX)
        {
/* modified by cuixl for support vtysh in gb, 2013-04-13 */
#ifdef _CENTEC_
            continue;
#else            
            return CMD_ERR_EXEED_ARGC_MAX;
#endif /*_CENTEC_*/                           
        }
    }

    /* For vtysh execution. */
    if (cmd)
        *cmd = matched_element;

    if(vtysh_cmd_arg_debug)
    {
        printf("argc=%d, argv= \n\r", argc);
        for(i=0; i<argc; i++)
        {
            printf("%s ", argv[i]);
        }
        printf("\n\r");
    }

    if (matched_element && matched_element->daemon)
    {
/* modified by cuixl for support vtysh in gb, 2013-04-13 */
#ifdef _CENTEC_
            continue;
#else            
        return CMD_SUCCESS_DAEMON;
#endif /*_CENTEC_*/                    
    }

    /* Execute matched command. */
    return (*matched_element->func) (matched_element, vty, argc, argv);
#ifdef _CENTEC_    
    }
    return CMD_SYS_ERROR;    
#endif    
}



/* Execute command by argument readline. */
int
cmd_execute_command_strict (vector vline, struct vty *vty, 
			    struct cmd_element **cmd)
{
  int i;
  int index;
  vector cmd_vector;
  struct cmd_element *cmd_element;
  struct cmd_element *matched_element;
  unsigned int matched_count, incomplete_count;
  int argc;
  char *argv[CMD_ARGC_MAX];
  int varflag;
  enum match_type match = 0;
  char *command;

  /* Make copy of command element */
  cmd_vector = vtysh_vector_copy (cmd_node_vector (vtysh_cmdvec, vty->node));
  if(!cmd_vector)
  {
      return CMD_SYS_ERROR;
  }

  for (index = 0; index < vector_max (vline); index++) 
    {
      int ret;

      command = vector_slot (vline, index);

      match = cmd_filter_by_string (vector_slot (vline, index), 
				    cmd_vector, index);

      /* If command meets '.VARARG' then finish matching. */
      if (match == vararg_match)
	break;

      ret = vtysh_is_cmd_ambiguous (command, cmd_vector, index, match);
      if (ret == 1)
	{
	  vtysh_vector_free (cmd_vector);
	  return CMD_ERR_AMBIGUOUS;
	}
      if (ret == 2)
	{
	  vtysh_vector_free (cmd_vector);
	  return CMD_ERR_NO_MATCH;
	}
    }

  /* Check matched count. */
  matched_element = NULL;
  matched_count = 0;
  incomplete_count = 0;
  for (i = 0; i < vector_max (cmd_vector); i++) 
    if (vector_slot (cmd_vector,i) != NULL)
      {
	cmd_element = vector_slot (cmd_vector,i);

	if (match == vararg_match || index >= cmd_element->cmdsize)
	  {
	    matched_element = cmd_element;
	    matched_count++;
	  }
	else
	  incomplete_count++;
      }
  
  /* Finish of using cmd_vector. */
  vtysh_vector_free (cmd_vector);

  /* To execute command, matched_count must be 1.*/
  if (matched_count == 0) 
    {
      if (incomplete_count)
	return CMD_ERR_INCOMPLETE;
      else
	return CMD_ERR_NO_MATCH;
    }

  if (matched_count > 1) 
    return CMD_ERR_AMBIGUOUS;

  /* Argument treatment */
  varflag = 0;
  argc = 0;

  for (i = 0; i < vector_max (vline); i++)
    {
      if (varflag)
	argv[argc++] = vector_slot (vline, i);
      else
	{	  
	  vector descvec = vector_slot (matched_element->strvec, i);

	  if (vector_max (descvec) == 1)
	    {
	      struct desc *desc = vector_slot (descvec, 0);
	      char *str = desc->cmd;

	      if (CMD_VARARG (str))
		varflag = 1;
	  
	      if (varflag || CMD_VARIABLE (str) || CMD_OPTION (str))
		argv[argc++] = vector_slot (vline, i);
	    }
	  else
	    argv[argc++] = vector_slot (vline, i);
	}

      if (argc >= CMD_ARGC_MAX)
	return CMD_ERR_EXEED_ARGC_MAX;
    }

  /* For vtysh execution. */
  if (cmd)
    *cmd = matched_element;

  if (matched_element->daemon)
    return CMD_SUCCESS_DAEMON;

  /* Now execute matched command */
  return (*matched_element->func) (matched_element, vty, argc, argv);
}

/* Configration make from file. */
int
config_from_file (struct vty *vty, FILE *fp)
{
#if 0
  int ret;
  vector vline;

  while (fgets (vty->buf, VTY_BUFSIZ, fp))
    {
      vline = cmd_make_strvec (vty->buf);

      /* In case of comment line */
      if (vline == NULL)
	continue;
      /* Execute configuration command : this is strict match */
      ret = cmd_execute_command_strict (vline, vty, NULL);

      /* Try again with setting node to CONFIG_NODE */
      if (ret != CMD_SUCCESS && ret != CMD_WARNING)
	{
	  if (vty->node == KEYCHAIN_KEY_NODE)
	    {
	      vty->node = KEYCHAIN_NODE;

	      ret = cmd_execute_command_strict (vline, vty, NULL);

	      if (ret != CMD_SUCCESS && ret != CMD_WARNING)
		{
		  vty->node = CONFIG_NODE;
		  ret = cmd_execute_command_strict (vline, vty, NULL);
		}
	    }
	  else
	    {
	      vty->node = CONFIG_NODE;
	      ret = cmd_execute_command_strict (vline, vty, NULL);
	    }
	}	  

      cmd_free_strvec (vline);

      if (ret != CMD_SUCCESS && ret != CMD_WARNING)
	return ret;
    }
#endif
  return CMD_SUCCESS;
}

/* Initialize command interface. Install basic nodes and commands. */
void
ctc_cmd_init (int terminal)
{
  /* Allocate initial top vector of commands. */
    vtysh_cmdvec = vtysh_vector_init (VECTOR_MIN_SIZE);
    vtysh_matched_desc_ptr =      (struct desc**)malloc(sizeof(struct desc*)*CMD_ARGC_MAX);
    if(!vtysh_cmdvec || !vtysh_matched_desc_ptr)
    {
        ctc_cli_out("\nError: no memory!!");
    }
    return;
}

void cli_enable_cmd_debug(int enable)
{
    vtysh_cmd_debug = enable?1:0;
}

void cli_enable_arg_debug(int enable)
{
    vtysh_cmd_arg_debug = enable?1:0;
}

int is_cmd_var(char *cmd)
{
    int index=0;
    if(cmd[0] == '<')
    {
        return 1;
    }

    for(index=0; index<strlen(cmd); index++)
    {
        if((cmd[index] >= 'A') && (cmd[index] <= 'Z'))
        {
            return 1;
        }
    }

    return 0;
}
