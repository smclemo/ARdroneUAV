/**
 * @file ATcodec_tree.c
 * @author aurelien.morelle@parrot.com
 * @date 2007/08/20
 */

#include <VP_Os/vp_os_types.h>
#include <VP_Os/vp_os_assert.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_malloc.h>
#include <ATcodec/ATcodec_Tree.h>
#include <ATcodec/ATcodec_api.h>
#include <ATcodec/ATcodec_Buffer.h>
#include <ATcodec/ATcodec.h>


/* \todo Remove nodes fields initializations that are not necessary in each function */


/* Is it really necessary ? */
static void
ATcodec_Tree_Node_init(ATcodec_Tree_Node_t *node)
{
  VP_OS_ASSERT(node);

  node->type = ATCODEC_TREE_NODE_TYPE_EMPTY;
  node->depth = 0;
  node->nb_sons = 0;
  node->data = -1;
}


void
ATcodec_Tree_init(ATcodec_Tree_t *tree, size_t leaf_size, int nb_start)
{
  ATcodec_Tree_Node_t s_node;

  VP_OS_ASSERT(tree);

  ATcodec_Buffer_init(&tree->leaves, leaf_size, nb_start);
  ATcodec_Buffer_init(&tree->strs, sizeof(char), nb_start);
  ATcodec_Buffer_init(&tree->sons, sizeof(ATcodec_Tree_Node_t), nb_start);

  ATcodec_Tree_Node_init(&s_node);
  tree->root = tree->sons.nbElements;
  ATcodec_Buffer_pushElement(&tree->sons, &s_node);
}


static int
ATcodec_Tree_Node_insert(ATcodec_Tree_t *tree, ATCODEC_TREE_NODE_TYPE type, int depth, int str_index, int data, ATcodec_Tree_Node_t *father, int son_index)
{
  ATcodec_Tree_Node_t s_node;
  int node_index;

  node_index = tree->sons.nbElements;
  if(father)
    father->sons[son_index] = node_index;

  s_node.type = type;
  s_node.depth = depth;
  s_node.strkey = str_index;
  s_node.data = data;

  ATcodec_Buffer_pushElement(&tree->sons, &s_node);

  return node_index;
}


/* Put it public ? */
static int
ATcodec_Tree_Node_search(ATcodec_Tree_t *tree, char *str)
{
  ATcodec_Tree_Node_t *node;
  int searching = 1;
  int depth = 0;
  int str_index;
  int node_index = -1;

  VP_OS_ASSERT(str);
  VP_OS_ASSERT(*str);
  VP_OS_ASSERT(tree);

  str_index = tree->strs.nbElements;
  ATcodec_Buffer_pushElements(&tree->strs, str, strlen(str)+1);

  node = ATcodec_Tree_Node_get(tree, tree->root);

  while(searching)
    {
      switch(node->type)
	{
	case ATCODEC_TREE_NODE_TYPE_EMPTY:
	  {
	    node->type = ATCODEC_TREE_NODE_TYPE_LEAF;
	    node->depth = depth;
	    node->strkey = str_index;
	    node_index = 0;
	    searching = 0;
	  }
	  break;

	case ATCODEC_TREE_NODE_TYPE_NODE:
	  {
	    node_index = node->sons[(int)*str++];
	    if(node_index == -1)
	      {
		node_index = ATcodec_Tree_Node_insert(tree, ATCODEC_TREE_NODE_TYPE_LEAF, depth+1, str_index, -1, node, (int)*(str-1));
		searching = 0;
	      }
	    node = ATcodec_Tree_Node_get(tree, node_index);
	    depth++;
	  }
	  break;

	case ATCODEC_TREE_NODE_TYPE_LEAF:
	  {
	    char *leaf_str = (char *)ATcodec_Buffer_getElement(&tree->strs, node->strkey);
	    int i;

	    while(*str && *str == leaf_str[depth])
	      {
		node->type = ATCODEC_TREE_NODE_TYPE_NODE;
		for(i = 0 ; i < 256 ; i++)
		  node->sons[i] = -1;

		node_index = ATcodec_Tree_Node_insert(tree, ATCODEC_TREE_NODE_TYPE_LEAF, depth+1, node->strkey, node->data, node, (int)leaf_str[depth]);
		depth++;

		node = ATcodec_Tree_Node_get(tree, node_index);
		str++;
	      }

	    if(*str == leaf_str[depth])
	      {
		node->type = ATCODEC_TREE_NODE_TYPE_MULTILEAVES;
		node->nb_sons = 2;

		node_index = ATcodec_Tree_Node_insert(tree, ATCODEC_TREE_NODE_TYPE_LEAF, ++depth, node->strkey, node->data, node, 0);
		node->data = -1;

		node_index = ATcodec_Tree_Node_insert(tree, ATCODEC_TREE_NODE_TYPE_LEAF, depth, str_index, -1, node, 1);
	      }
	    else
	      {
		node_index = ATcodec_Tree_Node_insert(tree, ATCODEC_TREE_NODE_TYPE_LEAF, node->depth+1, node->strkey, node->data, NULL, -1);

		node->type = ATCODEC_TREE_NODE_TYPE_NODE;
		for(i = 0 ; i < 256 ; i++)
		  node->sons[i] = -1;
		node->sons[(int)leaf_str[depth]] = node_index;

		node_index = ATcodec_Tree_Node_insert(tree, ATCODEC_TREE_NODE_TYPE_LEAF, ++depth, str_index, -1, node, (int)*str);
	      }
	    searching = 0;
	  }
	  break;

	case ATCODEC_TREE_NODE_TYPE_MULTILEAVES:
	  {
	    node_index = ATcodec_Tree_Node_insert(tree, ATCODEC_TREE_NODE_TYPE_LEAF, ++depth, str_index, -1, node, node->nb_sons++);
	    searching = 0;
	  }
	  break;
	}
    }

  return node_index;
}


int
ATcodec_Tree_insert(ATcodec_Tree_t *tree, char *str, void *data)
{
  ATcodec_Tree_Node_t *node;
  int node_i;

  VP_OS_ASSERT(tree);

  node_i = ATcodec_Tree_Node_search(tree, str);

  node = ATcodec_Tree_Node_get(tree, node_i);
  node->data = tree->leaves.nbElements;
  ATcodec_Buffer_pushElement(&tree->leaves, data);

  return node_i;
}


ATcodec_Tree_Node_t *
ATcodec_Tree_Node_get(ATcodec_Tree_t *tree, int node)
{
  return (ATcodec_Tree_Node_t *)ATcodec_Buffer_getElement(&tree->sons, node);
}


#define AT_CODEC_PRINT_NODE_CHAR_CASE(CARAC_SRC,STR_DST) \
  case CARAC_SRC:                                        \
    PRINT(STR_DST);                                      \
    break

#define AT_CODEC_PRINT_NODE_CHAR(CARAC)                  \
  switch(CARAC)                                          \
  {                                                      \
    AT_CODEC_PRINT_NODE_CHAR_CASE('\r',"<CR>");          \
    AT_CODEC_PRINT_NODE_CHAR_CASE('\n',"<LF>");          \
    AT_CODEC_PRINT_NODE_CHAR_CASE('\0',"<\\0>");         \
    AT_CODEC_PRINT_NODE_CHAR_CASE(' ',"< >");            \
    default:                                             \
      PRINT("%c", CARAC);                                \
      break;                                             \
  }

void
ATcodec_Tree_Node_print(ATcodec_Tree_t *tree, ATcodec_Tree_Node_t *node)
{
#ifdef ATCODEC_DEBUG
  static int tab = 0;
  int i, j;
  char *sta;

  if(node->type == ATCODEC_TREE_NODE_TYPE_LEAF)
    {
      sta = ATcodec_Buffer_getElement(&tree->strs, node->strkey);
      for(j = 0 ; j < tab ; j++)
	ATCODEC_PRINT(" . ");
      do
	{
	  AT_CODEC_PRINT_NODE_CHAR(*sta);
	}
      while(*sta++);
      ATCODEC_PRINT("\"\n");
    }
  else if(node->type == ATCODEC_TREE_NODE_TYPE_NODE)
    {
      for(i = 0 ; i < 256 ; i++)
	{
	  if(node->sons[i] != -1)
	    {
	      for(j = 0 ; j < tab ; j++)
		ATCODEC_PRINT(" . ");
	      AT_CODEC_PRINT_NODE_CHAR((char)i);
	      ATCODEC_PRINT("\n");
	      tab++;
	      ATcodec_Tree_Node_print(tree, ATcodec_Tree_Node_get(tree, node->sons[i]));
	      tab--;
	    }
	}
    }
  else if(node->type == ATCODEC_TREE_NODE_TYPE_MULTILEAVES)
    {
      for(i = 0 ; i < node->nb_sons ; i++)
	{
	  tab++;
	  ATcodec_Tree_Node_print(tree, ATcodec_Tree_Node_get(tree, node->sons[i]));
	  tab--;
	}
    }
#endif // > ATCODEC_DEBUG
}


void
ATcodec_Tree_print(ATcodec_Tree_t *tree)
{
#ifdef ATCODEC_DEBUG
  ATcodec_Tree_Node_print(tree, ATcodec_Tree_Node_get(tree, tree->root));
#endif // > ATCODEC_DEBUG
}

