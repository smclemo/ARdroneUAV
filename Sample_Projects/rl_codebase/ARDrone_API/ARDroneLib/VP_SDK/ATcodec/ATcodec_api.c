/**
 * @file ATcodec_api.c
 * @author aurelien.morelle@parrot.fr
 * @date 2006/12/06
 */

#include <VP_Os/vp_os_types.h>
#include <VP_Os/vp_os_delay.h>
#include <VP_Os/vp_os_assert.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_signal.h>
#include <VP_Os/vp_os_print.h>

#if !defined(TARGET_OS_IPHONE) && !defined(TARGET_IPHONE_SIMULATOR)
# include <VP_Api/vp_api_thread_helper.h>
#endif // ! TARGET_OS_IPHONE && ! TARGET_IPHONE_SIMULATOR

#include <ATcodec/ATcodec_api.h>
#include <ATcodec/ATcodec_Buffer.h>
#include <ATcodec/ATcodec.h>
#include <ATcodec/ATcodec_Tree.h>


static AT_CODEC_FUNCTIONS_PTRS func_ptrs;
static ATcodec_Tree_t default_tree;
static int atcodec_lib_init_ok = 0;

// only for client
static vp_os_mutex_t ATcodec_cond_mutex;
static vp_os_cond_t  ATcodec_wait_cond;
static int8_t        ATcodec_Message_Buffer[INTERNAL_BUFFER_SIZE];
static int32_t       ATcodec_Message_len = 0;

static int32_t v_continue = 0;


static ATCODEC_RET
test_dyn_strs_one_node(ATcodec_Tree_t *tree, ATcodec_Tree_Node_t *node, int depth, const char *dynamic_str, ATcodec_Memory_t *memory, AT_CODEC_Message_Received *ptr, int *len_dec)
{
  char *static_str, *dyn_str, *mem_index;
  int len;
  ATcodec_Memory_t str, fmt;
	
  static_str = (char *)ATcodec_Buffer_getElement(&tree->strs, ((ATcodec_Message_Data_t *)ATcodec_Buffer_getElement(&tree->leaves, node->data))->static_str);
  if((len = strlen(static_str)) > depth)
    {
      if(strncmp(static_str+depth, dynamic_str, (len-depth)*sizeof(char)))
	return ATCODEC_FALSE;
    }
	
  dyn_str = (char *)ATcodec_Buffer_getElement(&tree->strs, ((ATcodec_Message_Data_t *)ATcodec_Buffer_getElement(&tree->leaves, node->data))->dynamic_str);
  ATcodec_Memory_Init(&str, dynamic_str+(len-depth), 0, 1, NULL, NULL);
  ATcodec_Memory_Init(&fmt, dyn_str, 0, 1, NULL, NULL);
	
  mem_index = memory->current;
  if(vp_atcodec_sscanf(&str, &fmt, memory, len_dec) == ATCODEC_TRUE)
    {
      memory->current = mem_index;
      *ptr = ((ATcodec_Message_Data_t *)ATcodec_Buffer_getElement(&tree->leaves, node->data))->func_ptr;
      *len_dec += len;
      return ATCODEC_TRUE;
    }
	
  return ATCODEC_FALSE;
}


// We assume we know that current->isleaf != -1 here
static ATCODEC_RET
test_dyn_strs(ATcodec_Tree_t *tree, ATcodec_Tree_Node_t *current, int depth, const char *dynamic_str, ATcodec_Memory_t *memory, AT_CODEC_Message_Received *ptr, int *len_dec)
{
  ATcodec_Tree_Node_t *son;
  int i;
	
  if(current->type == ATCODEC_TREE_NODE_TYPE_LEAF)
    {
      // just current to be tested
      return test_dyn_strs_one_node(tree, current, depth, dynamic_str, memory, ptr, len_dec);
    }
  else
    {
      VP_OS_ASSERT(current->type == ATCODEC_TREE_NODE_TYPE_MULTILEAVES);
      // all sons have to be tested
      for(i = 0 ; i < current->nb_sons ; i++)
	{
	  son = (ATcodec_Tree_Node_t *)ATcodec_Buffer_getElement(&tree->sons, current->sons[i]);
	  if(test_dyn_strs_one_node(tree, son, depth, dynamic_str, memory, ptr, len_dec) == ATCODEC_TRUE)
	    return ATCODEC_TRUE;
	}
    }
	
  return ATCODEC_FALSE;
}


static ATCODEC_RET
test_process_node(ATcodec_Tree_t *tree, ATcodec_Tree_Node_t *node, const char *cpy, ATcodec_Memory_t *memory, int *len_dec, int depth)
{
  ATcodec_Tree_Node_t *son;
  AT_CODEC_Message_Received ptr;
  AT_CODEC_MSG_ID id = (AT_CODEC_MSG_ID)-1;
  char *fmt_str;
  int32_t output_len;
  char output_params[INTERNAL_BUFFER_SIZE];
  int8_t output_str[INTERNAL_BUFFER_SIZE];
  ATcodec_Memory_t output, dest, fmt;
	
  if(test_dyn_strs(tree, node, depth, cpy, memory, &ptr, len_dec) == ATCODEC_TRUE)
    {
      ATcodec_Memory_Init(&output, &output_params[0], INTERNAL_BUFFER_SIZE, 1, NULL, NULL);
      VP_OS_ASSERT(ptr);
      ptr(memory, &output, &id);
      if((int)id != -1)
	{
	  son = ATcodec_Tree_Node_get(tree, id);
	  fmt_str = (char *)ATcodec_Buffer_getElement(&tree->strs, ((ATcodec_Message_Data_t *)ATcodec_Buffer_getElement(&tree->leaves, son->data))->total_str);
			
	  ATcodec_Memory_Init(&dest, (char*)&output_str[0], INTERNAL_BUFFER_SIZE, 1, NULL, NULL);
	  ATcodec_Memory_Init(&fmt, fmt_str, 0, 1, NULL, NULL);
			
	  output.current = (char *)output.start;
	  if(vp_atcodec_sprintf_params(&dest, &output_len, &fmt, &output) != ATCODEC_TRUE)
	    return ATCODEC_FALSE;
			
	  if(func_ptrs.write(&output_str[0], &output_len) != AT_CODEC_WRITE_OK)
	    return ATCODEC_FALSE;
	}
      return ATCODEC_TRUE;
    }
	
  return ATCODEC_FALSE;
}


static ATCODEC_RET
find_process_node(ATcodec_Tree_t *tree, const char *message, ATcodec_Memory_t *memory, int *len_dec)
{
  ATcodec_Tree_Node_t *current = ATcodec_Tree_Node_get(tree, tree->root);
  char *cpy = (char *)message;
  int depth = 0;
	
  while(current->type == ATCODEC_TREE_NODE_TYPE_NODE)
    {
      if(current->sons[0] != -1)
	{
	  if(test_process_node(tree, ATcodec_Tree_Node_get(tree, current->sons[0]), cpy, memory, len_dec, depth) == ATCODEC_TRUE)
	    return ATCODEC_TRUE;
	}
      if(!*cpy || current->sons[(int)(*cpy)] == -1)
	{
	  return ATCODEC_FALSE;
	}
      else
	{
	  current = ATcodec_Tree_Node_get(tree, current->sons[(int)(*cpy++)]);
	  depth++;
	}
    }
	
  return test_process_node(tree, current, cpy, memory, len_dec, depth);
}


static int
static_len(const char *str)
{
  int len = 0;
  int percent = 0;
  int escaped = 1;
	
  do
    {
      switch(*str)
	{
	case '%':
	  if(escaped)
	    percent = 1-percent;
	  escaped = 1;
	  break;
	case 's':
	case 'd':
	case 'c':
	case 'l':
	  if(percent)
	    {
	      return (len-1);
	    }
	  escaped = 1;
	  percent = 0;
	  break;
	case '[':
	  if(escaped)
	    {
	      return len;
	    }
	  break;
	case '\\':
	  escaped = 1-escaped;
	  percent = 0;
	  break;
	case '\0':
	  return len;
	  break;
	default:
	  escaped = 1;
	  percent = 0;
	  break;
	}
		
      len++;
    }
  while(*str++);
	
  return len;
}


AT_CODEC_ERROR_CODE at_default_cb(ATcodec_Memory_t *input, ATcodec_Memory_t *output, int *id)
{
  ATCODEC_PRINT("< AT message received >\n");
	
  return AT_CODEC_GENERAL_OK;
}


AT_CODEC_MSG_ID
ATcodec_Add_Hashed_Message (const char *str, AT_CODEC_MSG_ID from_cmd, AT_CODEC_Message_Received func_ptr, int priority)
{
  return ATcodec_Add_Hashed_Message_Tree  (&default_tree, str, from_cmd, func_ptr, priority);
}


AT_CODEC_MSG_ID
ATcodec_Add_Hashed_Message_Tree  (ATcodec_Tree_t *tree, const char *str, AT_CODEC_MSG_ID from_cmd, AT_CODEC_Message_Received func_ptr, int priority)
{
  ATcodec_Message_Data_t data, *p_data;
  ATcodec_Tree_Node_t *node;
  int len_s, len_d, node_i;
  char buffer[1024];
	
  data.static_str = -1;
  data.dynamic_str = -1;
  data.total_str = -1;
	
  data.from_cmd = from_cmd;
  data.func_ptr = func_ptr;
  data.priority = priority;
	
  len_s = static_len(str);
  VP_OS_ASSERT(len_s < 1024);
  vp_os_memcpy(&buffer[0], str, len_s);
  buffer[len_s] = '\0';
  node_i = ATcodec_Tree_insert(tree, &buffer[0], &data);
  node = ATcodec_Tree_Node_get(tree, node_i);
	
  p_data = ATcodec_Buffer_getElement(&tree->leaves, node->data);
  p_data->static_str = node->strkey;
	
  // backup dynamic_str
  p_data->dynamic_str = tree->strs.nbElements;
  len_d = strlen(str+len_s)+1;
  ATcodec_Buffer_pushElements (&tree->strs, str+len_s, len_d);
	
  // backup concatenation of static_str and dynamic_str
  p_data->total_str = tree->strs.nbElements;
  ATcodec_Buffer_pushElements (&tree->strs, str, len_s+len_d);
	
  return (AT_CODEC_MSG_ID)node_i;
}


AT_CODEC_MSG_ID
ATcodec_Add_Defined_Message (const char *str)
{
  return ATcodec_Add_Defined_Message_Tree (&default_tree, str);
}


AT_CODEC_MSG_ID
ATcodec_Add_Defined_Message_Tree (ATcodec_Tree_t *tree, const char *str)
{
  return ATcodec_Add_Hashed_Message_Tree (tree, str, 0, NULL, 0);
}


void
ATcodec_Init_Library        (AT_CODEC_FUNCTIONS_PTRS *funcs)
{
  ATcodec_Init_Library_Tree   (&default_tree, funcs);
}


void
ATcodec_Init_Library_Tree   (ATcodec_Tree_t *tree, AT_CODEC_FUNCTIONS_PTRS *funcs)
{
  VP_OS_ASSERT(funcs);
  VP_OS_ASSERT(funcs->open);
  VP_OS_ASSERT(funcs->read);
  VP_OS_ASSERT(funcs->enable);
  VP_OS_ASSERT(funcs->write);
  VP_OS_ASSERT(funcs->close);
  VP_OS_ASSERT(funcs->init);
  VP_OS_ASSERT(funcs->shutdown);
	
  ATcodec_Tree_init(tree, sizeof(ATcodec_Message_Data_t), 1);
	
  memcpy(&func_ptrs, funcs, sizeof(AT_CODEC_FUNCTIONS_PTRS));
	
  vp_os_mutex_init(&ATcodec_cond_mutex);
  vp_os_cond_init(&ATcodec_wait_cond, &ATcodec_cond_mutex);
	
  if(func_ptrs.init() != AT_CODEC_INIT_OK)
    {
      ATCODEC_PRINT("ATcodec Init error\n");
    }
  else
    {
      ATcodec_Tree_print(tree);
      atcodec_lib_init_ok = 1;
      ATcodec_Message_len = 0;
    }
}


void
ATcodec_Shutdown_Library        (void)
{
  ATcodec_Shutdown_Library_Tree(&default_tree);
}


void
ATcodec_Shutdown_Library_Tree   (ATcodec_Tree_t *tree)
{
  /*   ATcodec_Buffer_destroy(tree->strs); */
  /*   ATcodec_Buffer_destroy(tree->sons); */
  /*   ATcodec_Buffer_destroy(tree->leaves); */
  atcodec_lib_init_ok = 0;
}


// \todo CHANGE THE '\r'
static int append_reception(char *buffer, int len, char *global_buffer, int *global_len)
{
  int res = 0, i;
	
  for(i = 0 ; i < len ; i++)
    {
      global_buffer[(*global_len)++] = buffer[i];
      if(*global_len == INTERNAL_BUFFER_SIZE)
	{
	  // buffer overflow => purge
	  *global_len = 0;
	  return -1;
	}
      if(buffer[i] == '\r')
	{
	  res++;
	}
    }
	
  global_buffer[*global_len] = '\0';
	
  return res;
}


static ATCODEC_RET process_received_data(ATcodec_Tree_t *tree, int nb, char *global_buffer, int *global_len)
{
  ATCODEC_RET res = ATCODEC_TRUE;
  int len_dec;
  char memory[INTERNAL_BUFFER_SIZE];
  ATcodec_Memory_t mem;

  while(nb--)
    {
      ATcodec_Memory_Init(&mem, &memory[0], INTERNAL_BUFFER_SIZE, 1, NULL, NULL);
      if(find_process_node(tree, global_buffer, &mem, &len_dec) != ATCODEC_TRUE)
	{
	  for(len_dec = 0 ; len_dec < *global_len && global_buffer[len_dec] != '\r' ; len_dec++)
	    {
	      // nothing
	    }
	  ATCODEC_PRINT("PURGE\n");
	  if(global_buffer[len_dec++] == '\r')
	    {
	      if(global_buffer[len_dec++] == '\0')
		len_dec++;
	    }
	}
		
      if(--len_dec >= *global_len)
	len_dec = *global_len;
		
      if(*global_len == len_dec)
	{
	  ATCODEC_ZERO_MEMSET(global_buffer, 0, *global_len*sizeof(char));
	  *global_len = 0;
	}
      else
	{
	  memmove(global_buffer, global_buffer+len_dec, *global_len-len_dec);
	  ATCODEC_ZERO_MEMSET(global_buffer+*global_len-len_dec, 0, (len_dec)*sizeof(char));
	  *global_len -= len_dec;
	}
    }
	
  return res;
}


#if !defined(TARGET_OS_IPHONE) && !defined(TARGET_IPHONE_SIMULATOR)
DEFINE_THREAD_ROUTINE_STACK(ATcodec_Commands_Server,data,ATCODEC_STACK_SIZE)
{
  ATcodec_Tree_t *tree = &default_tree;
  AT_CODEC_ERROR_CODE res;
  int32_t v_loop, v_read, len, nb_cmd = 0;
  char buffer[INTERNAL_BUFFER_SIZE]; // user-defined
  char global_buffer[INTERNAL_BUFFER_SIZE];
  int global_len;
	
  v_continue = 1;
  PRINT("Thread AT Commands Server Start\n");
	
  while(!atcodec_lib_init_ok)
    {
      vp_os_thread_yield();
    }
	
  while(v_continue)
    {
      global_len = 0;
		
      // open and init
      if((res = func_ptrs.open()) != AT_CODEC_OPEN_OK)
	v_continue = 0;
		
      for ( v_loop = 1 ; v_loop && func_ptrs.enable() == AT_CODEC_ENABLE_OK; )
	{
	  v_read = 1;
	  do
	    {
	      // wait so that thread can give the hand : delay user-defined / OS-dependent
	      // ...
	      vp_os_delay(ATCODEC_SERVER_YIELD_DELAY);
	      //vp_os_thread_yield();
				
	      // read some bytes
	      len = INTERNAL_BUFFER_SIZE/2; // user-defined
	      res = func_ptrs.read((int8_t*)&buffer[0], (int32_t*)&len);
	      if(res == AT_CODEC_READ_OK)
		{
		  if(len > 0)
		    {
		      // process characters and update v_read
		      // \todo Do not use nb_cmd ?
		      if((nb_cmd = append_reception(&buffer[0], len, &global_buffer[0], &global_len)) > 0)
			{
			  v_read = 0;
			}
		      else if(nb_cmd == -1)
			{
			  // a buffer overflow occurs
			  ATCODEC_PRINT("Overflow\n");
			  v_loop = 0;
			}
		      else
			{
			  v_read = 1;
			}
		    }
		  else
		    {
		      if(len < 0)
			{
			  ATCODEC_PRINT("read returns a neg length\n");
			  v_loop = 0;
			}
		    }
		}
	      else
		{
		  // an error occurs
		  ATCODEC_PRINT("an error occurs\n");
		  v_loop = 0;
		}
	    }
	  while (v_read && v_loop);
			
	  // process what has been received if no error occurs
	  if(v_loop)
	    {
	      // ...
	      if(process_received_data(tree, nb_cmd, &global_buffer[0], &global_len) != ATCODEC_TRUE)
		{
		  ATCODEC_PRINT("process_received returns false\n");
		  v_loop = 0;
		}
	    }
	}
		
      // close and un-init : user-defined
      if((res = func_ptrs.close()) != AT_CODEC_CLOSE_OK)
	v_continue = 0;
    }
	
  if((res = func_ptrs.shutdown()) != AT_CODEC_SHUTDOWN_OK)
    {
      ATCODEC_PRINT("ATcodec Shutdown error\n");
    }
	
  return((THREAD_RET)0);
}
#endif // ! TARGET_OS_IPHONE && ! TARGET_IPHONE_SIMULATOR


static ATCODEC_RET
valist_ATcodec_Queue_Message_valist_Tree(ATcodec_Tree_t *tree, AT_CODEC_MSG_ID id, va_list *va)
{
  int32_t len;
  ATCODEC_RET res;
  ATcodec_Tree_Node_t *node = ATcodec_Tree_Node_get(tree, (int)id);
  ATcodec_Message_Data_t *data = (ATcodec_Message_Data_t *)ATcodec_Buffer_getElement(&tree->leaves, node->data);
  char *total_str = (char *)ATcodec_Buffer_getElement(&tree->strs, data->total_str);
  ATcodec_Memory_t msg, fmt;
	
  if(!atcodec_lib_init_ok)
    return ATCODEC_FALSE;

  vp_os_mutex_lock(&ATcodec_cond_mutex);

  ATcodec_Memory_Init(&msg, (char*)&ATcodec_Message_Buffer[ATcodec_Message_len], INTERNAL_BUFFER_SIZE, 1, NULL, NULL);
  ATcodec_Memory_Init(&fmt, total_str, 0, 1, NULL, NULL);

  if((res = vp_atcodec_sprintf_valist(&msg, &len, &fmt, va)) !=  ATCODEC_TRUE)
    {
      vp_os_mutex_unlock(&ATcodec_cond_mutex);
      va_end(*va);
      return res;
    }

  ATcodec_Message_len += len;
	
  //vp_os_cond_signal(&ATcodec_wait_cond);
  vp_os_mutex_unlock(&ATcodec_cond_mutex);
  va_end(*va);
	
  return ATCODEC_TRUE;
}


ATCODEC_RET
ATcodec_Queue_Message_valist(AT_CODEC_MSG_ID id, ...)
{
  ATCODEC_RET res;
  va_list va;

  va_start(va, id);

  res = valist_ATcodec_Queue_Message_valist_Tree(&default_tree, id, &va);
	
  va_end(va);
	
  return res;
}


ATCODEC_RET
ATcodec_Queue_Message_valist_Tree(ATcodec_Tree_t *tree, AT_CODEC_MSG_ID id, ...)
{
  ATCODEC_RET res;
  va_list va;

  va_start(va, id);

  res = valist_ATcodec_Queue_Message_valist_Tree(tree, id, &va);
	
  va_end(va);
	
  return res;
}


ATCODEC_RET
ATcodec_Queue_Message_params(AT_CODEC_MSG_ID id, ATcodec_Memory_t *params)
{
  return ATcodec_Queue_Message_params_Tree(&default_tree, id, params);
}


ATCODEC_RET
ATcodec_Queue_Message_params_Tree(ATcodec_Tree_t *tree, AT_CODEC_MSG_ID id, ATcodec_Memory_t *params)
{
  int32_t len;
  ATCODEC_RET res;
  ATcodec_Tree_Node_t *node = ATcodec_Tree_Node_get(tree, (int)id);
  ATcodec_Message_Data_t *data = (ATcodec_Message_Data_t *)ATcodec_Buffer_getElement(&tree->leaves, node->data);
  char *total_str = (char *)ATcodec_Buffer_getElement(&tree->strs, data->total_str);
  ATcodec_Memory_t msg, fmt;
	
  if(!atcodec_lib_init_ok)
    return ATCODEC_FALSE;
	
  ATcodec_Memory_Init(&msg, (char*)&ATcodec_Message_Buffer[ATcodec_Message_len], INTERNAL_BUFFER_SIZE, 1, NULL, NULL);
  ATcodec_Memory_Init(&fmt, total_str, 0, 1, NULL, NULL);
	
  params->current = (char *)params->start;
  if((res = vp_atcodec_sprintf_params(&msg, &len, &fmt, params)) !=  ATCODEC_TRUE)
    {
      vp_os_mutex_unlock(&ATcodec_cond_mutex);
      return res;
    }
	
  ATcodec_Message_len += len;
	
  //vp_os_cond_signal(&ATcodec_wait_cond);
  vp_os_mutex_unlock(&ATcodec_cond_mutex);
	
  return ATCODEC_TRUE;
}


ATCODEC_RET
ATcodec_Send_Messages()
{
  ATCODEC_RET res = ATCODEC_TRUE;

  if(!atcodec_lib_init_ok)
    return ATCODEC_FALSE;
	
  vp_os_mutex_lock(&ATcodec_cond_mutex);
  // printf("ATcodec_Send_Messages : buf=%s\n", &ATcodec_Message_Buffer[0]);	
  if(ATcodec_Message_len && func_ptrs.write((int8_t*)&ATcodec_Message_Buffer[0], (int32_t*)&ATcodec_Message_len) != AT_CODEC_WRITE_OK)
    res = ATCODEC_FALSE;
	
  ATcodec_Message_len = 0;
	
  vp_os_mutex_unlock(&ATcodec_cond_mutex);
	
  return res;
}

#if !defined(TARGET_OS_IPHONE) && !defined(TARGET_IPHONE_SIMULATOR)
DEFINE_THREAD_ROUTINE_STACK(ATcodec_Commands_Client,data,ATCODEC_STACK_SIZE)
{
  AT_CODEC_ERROR_CODE res;
  int32_t v_loop;
	
  v_continue = 1;
  PRINT("Thread AT Commands Client Start\n");
	
  while(!atcodec_lib_init_ok)
    {
      vp_os_thread_yield();
    }
	
  while(v_continue)
    {
      // open and init
      if((res = func_ptrs.open()) != AT_CODEC_OPEN_OK)
	v_continue = 0;
		
      for ( v_loop = 1 ; v_loop ; )
	{
	  // vp_os_delay(ATCODEC_SERVER_YIELD_DELAY);
	  vp_os_thread_yield();
			
	  // wait a successful ATcodec_Queue_... has been called
	  vp_os_mutex_lock(&ATcodec_cond_mutex);
	  //vp_os_cond_wait(&ATcodec_wait_cond);
			
	  //ATCODEC_PRINT("Must send \"%s\"\n", &ATcodec_Message_Buffer[0]);
	  if(ATcodec_Message_len && func_ptrs.write((int8_t*)&ATcodec_Message_Buffer[0], (int32_t*)&ATcodec_Message_len) != AT_CODEC_WRITE_OK)
	    v_loop = 0;
			
	  ATcodec_Message_len = 0;
	  vp_os_mutex_unlock(&ATcodec_cond_mutex);
	}
		
      // close and un-init : user-defined
      if((res = func_ptrs.close()) != AT_CODEC_CLOSE_OK)
	v_continue = 0;
    }
	
  if((res = func_ptrs.shutdown()) != AT_CODEC_SHUTDOWN_OK)
    {
      ATCODEC_PRINT("ATcodec Shutdown error\n");
    }
	
  return((THREAD_RET)0);
}
#endif // ! TARGET_OS_IPHONE && ! TARGET_IPHONE_SIMULATOR


void ATcodec_exit_thread(void)
{
  v_continue = 0;
}
