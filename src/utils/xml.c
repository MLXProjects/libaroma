/********************************************************************[libaroma]*
 * Copyright © 2003-2022 by Michael R Sweet <https://www.msweet.org/mxml>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Mini-XML, a small XML file parsing library.
 *______________________________________________________________________________
 *
 * Filename		: xml.c
 * Description : xml utility
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + Mini-XML - Michael R Sweet.
 * + 22/01/23 - Author(s): Michael Jauregui
 *
 */
#if !defined(__libaroma_xml_c__) && !defined(LIBAROMA_CONFIG_NOXML)
#define __libaroma_xml_c__

#include <aroma_internal.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

/* disable false positive warning */
#ifdef __clang__
        #pragma clang diagnostic ignored "-Wuse-after-free"
#elif defined(__GNUC__)
        /* -Wuse-after-free was added in GCC 7.1 */
        #if __GNUC__ >= 12
                #pragma GCC diagnostic ignored "-Wuse-after-free"
        #endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Function prototypes for later use
 */
char *_mxml_vstrdupf(const char *format, va_list ap);
char *_mxml_strdupf(const char *format, ...);

/*
 * Libaroma wrappers
 */

LIBAROMA_XML libaroma_xml(const char *string){
	return mxmlLoadString(NULL, string, MXML_OPAQUE_CALLBACK);
}

LIBAROMA_XML libaroma_xml_file_ex(const char *path, byte writable){
	/* try open file */
	FILE *fp = fopen(path, (writable)?"r+":"r");
	if (!fp){
		/* it may not exist, create if writable enabled */
		if (writable){
			fp = fopen(path, "w+");
			if (!fp) return NULL;
		}
		else return NULL;
	}
	return mxmlLoadFile(NULL, fp, MXML_OPAQUE_CALLBACK);
}

LIBAROMA_XML libaroma_xml_get(LIBAROMA_XML xml, const char *tag){
	LIBAROMA_XML elem = mxmlFindElement(xml, xml, tag, NULL, NULL, MXML_DESCEND);
	while (mxmlGetType(elem)!=MXML_ELEMENT){
		elem = mxmlGetNextSibling(elem);
		if (!elem) return NULL;
	}
	return elem;
}

LIBAROMA_XML _libaroma_xml_find_usable(LIBAROMA_XML xml, const char *tag){
	LIBAROMA_XML sibling = xml;
	while (1){
		/* find usable sibling */
		while (mxmlGetType(sibling)!=MXML_ELEMENT){
			sibling = mxmlGetNextSibling(sibling);
			if (!sibling) return NULL;
		}
		if (!tag) return sibling;
		if (0==strcmp(mxmlGetElement(sibling), tag)) return sibling;
		sibling = mxmlGetNextSibling(sibling);
	}
}

LIBAROMA_XML libaroma_xml_sibling(LIBAROMA_XML xml, const char *tag){
	LIBAROMA_XML sibling = mxmlGetNextSibling(xml);
	return _libaroma_xml_find_usable(sibling, tag);
}

LIBAROMA_XML libaroma_xml_child(LIBAROMA_XML xml, const char *tag){
	LIBAROMA_XML child=mxmlGetFirstChild(xml);
	if (child){
		return _libaroma_xml_find_usable(child, tag);
	}
	return NULL;
}

LIBAROMA_XML libaroma_xml_next(LIBAROMA_XML xml){
	return libaroma_xml_sibling(xml, mxmlGetElement(xml));
}

LIBAROMA_XML libaroma_xml_prev(LIBAROMA_XML xml){
	return mxmlGetPrevSibling(xml);
}

LIBAROMA_XML libaroma_xml_parent(LIBAROMA_XML xml){
	return mxmlGetParent(xml);
}

const char *libaroma_xml_attr(LIBAROMA_XML xml, const char *attr){
	return mxmlElementGetAttr(xml, attr);
}

const char *libaroma_xml_name(LIBAROMA_XML xml){
	return mxmlGetElement(xml);
}

const char *libaroma_xml_value(LIBAROMA_XML xml){
	return mxmlGetOpaque(xml);
}

void libaroma_xml_free(LIBAROMA_XML xml){
	mxmlDelete(xml);
}

/*
 * Private structures...
 */

typedef struct _mxml_attr_s		/**** An XML element attribute value. ****/
{
  char			*name;		/* Attribute name */
  char			*value;		/* Attribute value */
} _mxml_attr_t;

typedef struct _mxml_element_s		/**** An XML element value. ****/
{
  char			*name;		/* Name of element */
  int			num_attrs;	/* Number of attributes */
  _mxml_attr_t		*attrs;		/* Attributes */
} _mxml_element_t;

typedef struct _mxml_text_s		/**** An XML text value. ****/
{
  int			whitespace;	/* Leading whitespace? */
  char			*string;	/* Fragment string */
} _mxml_text_t;

typedef struct _mxml_custom_s		/**** An XML custom value. ****/
{
  void			*data;		/* Pointer to (allocated) custom data */
  mxml_custom_destroy_cb_t destroy;	/* Pointer to destructor function */
} _mxml_custom_t;

typedef union _mxml_value_u		/**** An XML node value. ****/
{
  _mxml_element_t	element;	/* Element */
  int			integer;	/* Integer number */
  char			*opaque;	/* Opaque string */
  double		real;		/* Real number */
  _mxml_text_t		text;		/* Text fragment */
  _mxml_custom_t	custom;		/* Custom data @since Mini-XML 2.1@ */
} _mxml_value_t;

struct _mxml_node_s			/**** An XML node. ****/
{
  mxml_type_t		type;		/* Node type */
  struct _mxml_node_s	*next;		/* Next node under same parent */
  struct _mxml_node_s	*prev;		/* Previous node under same parent */
  struct _mxml_node_s	*parent;	/* Parent node */
  struct _mxml_node_s	*child;		/* First child node */
  struct _mxml_node_s	*last_child;	/* Last child node */
  _mxml_value_t		value;		/* Node value */
  int			ref_count;	/* Use count */
  void			*user_data;	/* User data */
};

struct _mxml_index_s			 /**** An XML node index. ****/
{
  char			*attr;		/* Attribute used for indexing or NULL */
  int			num_nodes;	/* Number of nodes in index */
  int			alloc_nodes;	/* Allocated nodes in index */
  int			cur_node;	/* Current node */
  mxml_node_t		**nodes;	/* Node array */
};

typedef struct _mxml_global_s		/**** Global, per-thread data ****/

{
  void	(*error_cb)(const char *);
  int	num_entity_cbs;
  int	(*entity_cbs[100])(const char *name);
  int	wrap;
  mxml_custom_load_cb_t	custom_load_cb;
  mxml_custom_save_cb_t	custom_save_cb;
} _mxml_global_t;

/*
 * Function prototypes...
 */

_mxml_global_t	*_mxml_global(void);
int		_mxml_entity_cb(const char *name);

/*
 * mxml-private.c
 */

/*
 * Some crazy people think that unloading a shared object is a good or safe
 * thing to do.  Unfortunately, most objects are simply *not* safe to unload
 * and bad things *will* happen.
 *
 * The following mess of conditional code allows us to provide a destructor
 * function in Mini-XML for our thread-global storage so that it can possibly
 * be unloaded safely, although since there is no standard way to do so I
 * can't even provide any guarantees that you can do it safely on all platforms.
 *
 * This code currently supports AIX, HP-UX, Linux, macOS, Solaris, and
 * Windows.  It might work on the BSDs and IRIX, but I haven't tested that.
 */

#if defined(__sun) || defined(_AIX)
#  pragma fini(_mxml_fini)
#  define _MXML_FINI _mxml_fini
#elif defined(__hpux)
#  pragma FINI _mxml_fini
#  define _MXML_FINI _mxml_fini
#elif defined(__GNUC__) /* Linux and macOS */
#  define _MXML_FINI __attribute((destructor)) _mxml_fini
#else
#  define _MXML_FINI _fini
#endif /* __sun */

/*
 * 'mxml_error()' - Display an error message.
 */

void
mxml_error(const char *format,		/* I - Printf-style format string */
           ...)				/* I - Additional arguments as needed */
{
  va_list	ap;			/* Pointer to arguments */
  char		s[1024];		/* Message string */
  _mxml_global_t *global = _mxml_global();
					/* Global data */

 /*
  * Range check input...
  */

  if (!format)
    return;

 /*
  * Format the error message string...
  */

  va_start(ap, format);

  vsnprintf(s, sizeof(s), format, ap);

  va_end(ap);

 /*
  * And then display the error message...
  */

  if (global->error_cb)
    (*global->error_cb)(s);
  else
    fprintf(stderr, "mxml: %s\n", s);
}

/*
 * 'mxml_ignore_cb()' - Default callback for ignored values.
 */

mxml_type_t				/* O - Node type */
mxml_ignore_cb(mxml_node_t *node)	/* I - Current node */
{
  (void)node;

  return (MXML_IGNORE);
}

/*
 * 'mxml_integer_cb()' - Default callback for integer values.
 */

mxml_type_t				/* O - Node type */
mxml_integer_cb(mxml_node_t *node)	/* I - Current node */
{
  (void)node;

  return (MXML_INTEGER);
}

/*
 * 'mxml_opaque_cb()' - Default callback for opaque values.
 */

mxml_type_t				/* O - Node type */
mxml_opaque_cb(mxml_node_t *node)	/* I - Current node */
{
  (void)node;

  return (MXML_OPAQUE);
}

/*
 * 'mxml_real_cb()' - Default callback for real number values.
 */

mxml_type_t				/* O - Node type */
mxml_real_cb(mxml_node_t *node)		/* I - Current node */
{
  (void)node;

  return (MXML_REAL);
}

#ifdef HAVE_PTHREAD_H			/**** POSIX threading ****/
#  include <pthread.h>

static int		_mxml_initialized = 0;
					/* Have we been initialized? */
static pthread_key_t	_mxml_key;	/* Thread local storage key */
static pthread_once_t	_mxml_key_once = PTHREAD_ONCE_INIT;
					/* One-time initialization object */
static void		_mxml_init(void);
static void		_mxml_destructor(void *g);

/*
 * '_mxml_destructor()' - Free memory used for globals...
 */

static void
_mxml_destructor(void *g)		/* I - Global data */
{
  free(g);
}

/*
 * '_mxml_fini()' - Clean up when unloaded.
 */

static void
_MXML_FINI(void)
{
  if (_mxml_initialized)
    pthread_key_delete(_mxml_key);
}

/*
 * '_mxml_global()' - Get global data.
 */

_mxml_global_t *			/* O - Global data */
_mxml_global(void)
{
  _mxml_global_t	*global;	/* Global data */

  pthread_once(&_mxml_key_once, _mxml_init);

  if ((global = (_mxml_global_t *)pthread_getspecific(_mxml_key)) == NULL)
  {
    global = (_mxml_global_t *)calloc(1, sizeof(_mxml_global_t));
    pthread_setspecific(_mxml_key, global);

    global->num_entity_cbs = 1;
    global->entity_cbs[0]  = _mxml_entity_cb;
    global->wrap           = 72;
  }

  return (global);
}

/*
 * '_mxml_init()' - Initialize global data...
 */

static void
_mxml_init(void)
{
  _mxml_initialized = 1;
  pthread_key_create(&_mxml_key, _mxml_destructor);
}

#elif defined(_WIN32) && defined(MXML1_EXPORTS) /**** WIN32 threading ****/
#  include <windows.h>

static DWORD _mxml_tls_index;		/* Index for global storage */

/*
 * 'DllMain()' - Main entry for library.
 */

BOOL WINAPI				/* O - Success/failure */
DllMain(HINSTANCE hinst,		/* I - DLL module handle */
        DWORD     reason,		/* I - Reason */
        LPVOID    reserved)		/* I - Unused */
{
  _mxml_global_t	*global;	/* Global data */

  (void)hinst;
  (void)reserved;

  switch (reason)
  {
    case DLL_PROCESS_ATTACH :		/* Called on library initialization */
        if ((_mxml_tls_index = TlsAlloc()) == TLS_OUT_OF_INDEXES)
          return (FALSE);
        break;

    case DLL_THREAD_DETACH :		/* Called when a thread terminates */
        if ((global = (_mxml_global_t *)TlsGetValue(_mxml_tls_index)) != NULL)
          free(global);
        break;

    case DLL_PROCESS_DETACH :		/* Called when library is unloaded */
        if ((global = (_mxml_global_t *)TlsGetValue(_mxml_tls_index)) != NULL)
          free(global);

        TlsFree(_mxml_tls_index);
        break;

    default:
        break;
  }

  return (TRUE);
}

/*
 * '_mxml_global()' - Get global data.
 */

_mxml_global_t *			/* O - Global data */
_mxml_global(void)
{
  _mxml_global_t	*global;	/* Global data */

  if ((global = (_mxml_global_t *)TlsGetValue(_mxml_tls_index)) == NULL)
  {
    global = (_mxml_global_t *)calloc(1, sizeof(_mxml_global_t));

    global->num_entity_cbs = 1;
    global->entity_cbs[0]  = _mxml_entity_cb;
    global->wrap           = 72;

    TlsSetValue(_mxml_tls_index, (LPVOID)global);
  }

  return (global);
}

#else					/**** No threading ****/
/*
 * '_mxml_global()' - Get global data.
 */

_mxml_global_t *			/* O - Global data */
_mxml_global(void)
{
  static _mxml_global_t	global =	/* Global data */
  {
    NULL,				/* error_cb */
    1,					/* num_entity_cbs */
    { _mxml_entity_cb },		/* entity_cbs */
    72,					/* wrap */
    NULL,				/* custom_load_cb */
    NULL				/* custom_save_cb */
  };

  return (&global);
}
#endif /* HAVE_PTHREAD_H */

/*
 * mxml-attr.c
 */
/*
 * Local functions...
 */

static int	mxml_set_attr(mxml_node_t *node, const char *name, char *value);

/*
 * 'mxmlElementDeleteAttr()' - Delete an attribute.
 *
 * @since Mini-XML 2.4@
 */

void
mxmlElementDeleteAttr(mxml_node_t *node,/* I - Element */
                      const char  *name)/* I - Attribute name */
{
  int		i;			/* Looping var */
  _mxml_attr_t	*attr;			/* Cirrent attribute */

#ifdef DEBUG
  fprintf(stderr, "mxmlElementDeleteAttr(node=%p, name=\"%s\")\n",
          node, name ? name : "(null)");
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT || !name)
    return;

 /*
  * Look for the attribute...
  */

  for (i = node->value.element.num_attrs, attr = node->value.element.attrs;
       i > 0;
       i --, attr ++)
  {
#ifdef DEBUG
    printf("    %s=\"%s\"\n", attr->name, attr->value);
#endif /* DEBUG */

    if (!strcmp(attr->name, name))
    {
     /*
      * Delete this attribute...
      */

      free(attr->name);
      free(attr->value);

      i --;
      if (i > 0)
        memmove(attr, attr + 1, i * sizeof(_mxml_attr_t));

      node->value.element.num_attrs --;

      if (node->value.element.num_attrs == 0)
        free(node->value.element.attrs);
      return;
    }
  }
}

/*
 * 'mxmlElementGetAttr()' - Get an attribute.
 *
 * This function returns @code NULL@ if the node is not an element or the
 * named attribute does not exist.
 */

const char *				/* O - Attribute value or @code NULL@ */
mxmlElementGetAttr(mxml_node_t *node,	/* I - Element node */
                   const char  *name)	/* I - Name of attribute */
{
  int		i;			/* Looping var */
  _mxml_attr_t	*attr;			/* Cirrent attribute */

#ifdef DEBUG
  fprintf(stderr, "mxmlElementGetAttr(node=%p, name=\"%s\")\n",
          node, name ? name : "(null)");
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT || !name)
    return (NULL);

 /*
  * Look for the attribute...
  */

  for (i = node->value.element.num_attrs, attr = node->value.element.attrs;
       i > 0;
       i --, attr ++)
  {
#ifdef DEBUG
    printf("    %s=\"%s\"\n", attr->name, attr->value);
#endif /* DEBUG */

    if (!strcmp(attr->name, name))
    {
#ifdef DEBUG
      printf("    Returning \"%s\"!\n", attr->value);
#endif /* DEBUG */
      return (attr->value);
    }
  }

 /*
  * Didn't find attribute, so return NULL...
  */

#ifdef DEBUG
  puts("    Returning NULL!\n");
#endif /* DEBUG */

  return (NULL);
}

/*
 * 'mxmlElementGetAttrByIndex()' - Get an element attribute by index.
 *
 * The index ("idx") is 0-based.  @code NULL@ is returned if the specified index
 * is out of range.
 *
 * @since Mini-XML 2.11@
 */

const char *                            /* O - Attribute value */
mxmlElementGetAttrByIndex(
    mxml_node_t *node,                  /* I - Node */
    int         idx,                    /* I - Attribute index, starting at 0 */
    const char  **name)                 /* O - Attribute name */
{
  if (!node || node->type != MXML_ELEMENT || idx < 0 || idx >= node->value.element.num_attrs)
    return (NULL);

  if (name)
    *name = node->value.element.attrs[idx].name;

  return (node->value.element.attrs[idx].value);
}

/*
 * 'mxmlElementGetAttrCount()' - Get the number of element attributes.
 *
 * @since Mini-XML 2.11@
 */

int                                     /* O - Number of attributes */
mxmlElementGetAttrCount(
    mxml_node_t *node)                  /* I - Node */
{
  if (node && node->type == MXML_ELEMENT)
    return (node->value.element.num_attrs);
  else
    return (0);
}

/*
 * 'mxmlElementSetAttr()' - Set an attribute.
 *
 * If the named attribute already exists, the value of the attribute
 * is replaced by the new string value. The string value is copied
 * into the element node. This function does nothing if the node is
 * not an element.
 */

void
mxmlElementSetAttr(mxml_node_t *node,	/* I - Element node */
                   const char  *name,	/* I - Name of attribute */
                   const char  *value)	/* I - Attribute value */
{
  char	*valuec;			/* Copy of value */

#ifdef DEBUG
  fprintf(stderr, "mxmlElementSetAttr(node=%p, name=\"%s\", value=\"%s\")\n",
          node, name ? name : "(null)", value ? value : "(null)");
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT || !name)
    return;

  if (value)
  {
    if ((valuec = strdup(value)) == NULL)
    {
      mxml_error("Unable to allocate memory for attribute '%s' in element %s.", name, node->value.element.name);
      return;
    }
  }
  else
    valuec = NULL;

  if (mxml_set_attr(node, name, valuec))
    free(valuec);
}

/*
 * 'mxmlElementSetAttrf()' - Set an attribute with a formatted value.
 *
 * If the named attribute already exists, the value of the attribute
 * is replaced by the new formatted string. The formatted string value is
 * copied into the element node. This function does nothing if the node
 * is not an element.
 *
 * @since Mini-XML 2.3@
 */

void
mxmlElementSetAttrf(mxml_node_t *node,	/* I - Element node */
                    const char  *name,	/* I - Name of attribute */
                    const char  *format,/* I - Printf-style attribute value */
		    ...)		/* I - Additional arguments as needed */
{
  va_list	ap;			/* Argument pointer */
  char		*value;			/* Value */

#ifdef DEBUG
  fprintf(stderr,
          "mxmlElementSetAttrf(node=%p, name=\"%s\", format=\"%s\", ...)\n",
          node, name ? name : "(null)", format ? format : "(null)");
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT || !name || !format)
    return;

 /*
  * Format the value...
  */

  va_start(ap, format);
  value = _mxml_vstrdupf(format, ap);
  va_end(ap);

  if (!value)
    mxml_error("Unable to allocate memory for attribute '%s' in element %s.",
               name, node->value.element.name);
  else if (mxml_set_attr(node, name, value))
    free(value);
}

/*
 * 'mxml_set_attr()' - Set or add an attribute name/value pair.
 */

static int				/* O - 0 on success, -1 on failure */
mxml_set_attr(mxml_node_t *node,	/* I - Element node */
              const char  *name,	/* I - Attribute name */
              char        *value)	/* I - Attribute value */
{
  int		i;			/* Looping var */
  _mxml_attr_t	*attr;			/* New attribute */

 /*
  * Look for the attribute...
  */

  for (i = node->value.element.num_attrs, attr = node->value.element.attrs;
       i > 0;
       i --, attr ++)
    if (!strcmp(attr->name, name))
    {
     /*
      * Free the old value as needed...
      */

      free(attr->value);
      attr->value = value;

      return (0);
    }

 /*
  * Add a new attribute...
  */

  if (node->value.element.num_attrs == 0)
    attr = malloc(sizeof(_mxml_attr_t));
  else
    attr = realloc(node->value.element.attrs,
                   (node->value.element.num_attrs + 1) * sizeof(_mxml_attr_t));

  if (!attr)
  {
    mxml_error("Unable to allocate memory for attribute '%s' in element %s.",
               name, node->value.element.name);
    return (-1);
  }

  node->value.element.attrs = attr;
  attr += node->value.element.num_attrs;

  if ((attr->name = strdup(name)) == NULL)
  {
    mxml_error("Unable to allocate memory for attribute '%s' in element %s.",
               name, node->value.element.name);
    return (-1);
  }

  attr->value = value;

  node->value.element.num_attrs ++;

  return (0);
}

/*
 * mxml-entity.c
 */
/*
 * 'mxmlEntityAddCallback()' - Add a callback to convert entities to Unicode.
 */

int					/* O - 0 on success, -1 on failure */
mxmlEntityAddCallback(
    mxml_entity_cb_t cb)		/* I - Callback function to add */
{
  _mxml_global_t *global = _mxml_global();
					/* Global data */

  if (global->num_entity_cbs < (int)(sizeof(global->entity_cbs) / sizeof(global->entity_cbs[0])))
  {
    global->entity_cbs[global->num_entity_cbs] = cb;
    global->num_entity_cbs ++;

    return (0);
  }
  else
  {
    mxml_error("Unable to add entity callback!");

    return (-1);
  }
}

/*
 * 'mxmlEntityGetName()' - Get the name that corresponds to the character value.
 *
 * If val does not need to be represented by a named entity, @code NULL@ is returned.
 */

const char *				/* O - Entity name or @code NULL@ */
mxmlEntityGetName(int val)		/* I - Character value */
{
  switch (val)
  {
    case '&' :
        return ("amp");

    case '<' :
        return ("lt");

    case '>' :
        return ("gt");

    case '\"' :
        return ("quot");

    default :
        return (NULL);
  }
}

/*
 * 'mxmlEntityGetValue()' - Get the character corresponding to a named entity.
 *
 * The entity name can also be a numeric constant. -1 is returned if the
 * name is not known.
 */

int					/* O - Character value or -1 on error */
mxmlEntityGetValue(const char *name)	/* I - Entity name */
{
  int		i;			/* Looping var */
  int		ch;			/* Character value */
  _mxml_global_t *global = _mxml_global();
					/* Global data */

  for (i = 0; i < global->num_entity_cbs; i ++)
    if ((ch = (global->entity_cbs[i])(name)) >= 0)
      return (ch);

  return (-1);
}

/*
 * 'mxmlEntityRemoveCallback()' - Remove a callback.
 */

void
mxmlEntityRemoveCallback(
    mxml_entity_cb_t cb)		/* I - Callback function to remove */
{
  int		i;			/* Looping var */
  _mxml_global_t *global = _mxml_global();
					/* Global data */

  for (i = 0; i < global->num_entity_cbs; i ++)
    if (cb == global->entity_cbs[i])
    {
     /*
      * Remove the callback...
      */

      global->num_entity_cbs --;

      if (i < global->num_entity_cbs)
        memmove(global->entity_cbs + i, global->entity_cbs + i + 1,
	        (global->num_entity_cbs - i) * sizeof(global->entity_cbs[0]));

      return;
    }
}

/*
 * '_mxml_entity_cb()' - Lookup standard (X)HTML entities.
 */

int					/* O - Unicode value or -1 */
_mxml_entity_cb(const char *name)	/* I - Entity name */
{
  int	diff,				/* Difference between names */
	current,			/* Current entity in search */
	first,				/* First entity in search */
	last;				/* Last entity in search */
  static const struct
  {
    const char	*name;			/* Entity name */
    int		val;			/* Character value */
  }	entities[] =
  {
    { "AElig",		198 },
    { "Aacute",		193 },
    { "Acirc",		194 },
    { "Agrave",		192 },
    { "Alpha",		913 },
    { "Aring",		197 },
    { "Atilde",		195 },
    { "Auml",		196 },
    { "Beta",		914 },
    { "Ccedil",		199 },
    { "Chi",		935 },
    { "Dagger",		8225 },
    { "Delta",		916 },
    { "Dstrok",		208 },
    { "ETH",		208 },
    { "Eacute",		201 },
    { "Ecirc",		202 },
    { "Egrave",		200 },
    { "Epsilon",	917 },
    { "Eta",		919 },
    { "Euml",		203 },
    { "Gamma",		915 },
    { "Iacute",		205 },
    { "Icirc",		206 },
    { "Igrave",		204 },
    { "Iota",		921 },
    { "Iuml",		207 },
    { "Kappa",		922 },
    { "Lambda",		923 },
    { "Mu",		924 },
    { "Ntilde",		209 },
    { "Nu",		925 },
    { "OElig",		338 },
    { "Oacute",		211 },
    { "Ocirc",		212 },
    { "Ograve",		210 },
    { "Omega",		937 },
    { "Omicron",	927 },
    { "Oslash",		216 },
    { "Otilde",		213 },
    { "Ouml",		214 },
    { "Phi",		934 },
    { "Pi",		928 },
    { "Prime",		8243 },
    { "Psi",		936 },
    { "Rho",		929 },
    { "Scaron",		352 },
    { "Sigma",		931 },
    { "THORN",		222 },
    { "Tau",		932 },
    { "Theta",		920 },
    { "Uacute",		218 },
    { "Ucirc",		219 },
    { "Ugrave",		217 },
    { "Upsilon",	933 },
    { "Uuml",		220 },
    { "Xi",		926 },
    { "Yacute",		221 },
    { "Yuml",		376 },
    { "Zeta",		918 },
    { "aacute",		225 },
    { "acirc",		226 },
    { "acute",		180 },
    { "aelig",		230 },
    { "agrave",		224 },
    { "alefsym",	8501 },
    { "alpha",		945 },
    { "amp",		'&' },
    { "and",		8743 },
    { "ang",		8736 },
    { "apos",           '\'' },
    { "aring",		229 },
    { "asymp",		8776 },
    { "atilde",		227 },
    { "auml",		228 },
    { "bdquo",		8222 },
    { "beta",		946 },
    { "brkbar",		166 },
    { "brvbar",		166 },
    { "bull",		8226 },
    { "cap",		8745 },
    { "ccedil",		231 },
    { "cedil",		184 },
    { "cent",		162 },
    { "chi",		967 },
    { "circ",		710 },
    { "clubs",		9827 },
    { "cong",		8773 },
    { "copy",		169 },
    { "crarr",		8629 },
    { "cup",		8746 },
    { "curren",		164 },
    { "dArr",		8659 },
    { "dagger",		8224 },
    { "darr",		8595 },
    { "deg",		176 },
    { "delta",		948 },
    { "diams",		9830 },
    { "die",		168 },
    { "divide",		247 },
    { "eacute",		233 },
    { "ecirc",		234 },
    { "egrave",		232 },
    { "empty",		8709 },
    { "emsp",		8195 },
    { "ensp",		8194 },
    { "epsilon",	949 },
    { "equiv",		8801 },
    { "eta",		951 },
    { "eth",		240 },
    { "euml",		235 },
    { "euro",		8364 },
    { "exist",		8707 },
    { "fnof",		402 },
    { "forall",		8704 },
    { "frac12",		189 },
    { "frac14",		188 },
    { "frac34",		190 },
    { "frasl",		8260 },
    { "gamma",		947 },
    { "ge",		8805 },
    { "gt",		'>' },
    { "hArr",		8660 },
    { "harr",		8596 },
    { "hearts",		9829 },
    { "hellip",		8230 },
    { "hibar",		175 },
    { "iacute",		237 },
    { "icirc",		238 },
    { "iexcl",		161 },
    { "igrave",		236 },
    { "image",		8465 },
    { "infin",		8734 },
    { "int",		8747 },
    { "iota",		953 },
    { "iquest",		191 },
    { "isin",		8712 },
    { "iuml",		239 },
    { "kappa",		954 },
    { "lArr",		8656 },
    { "lambda",		955 },
    { "lang",		9001 },
    { "laquo",		171 },
    { "larr",		8592 },
    { "lceil",		8968 },
    { "ldquo",		8220 },
    { "le",		8804 },
    { "lfloor",		8970 },
    { "lowast",		8727 },
    { "loz",		9674 },
    { "lrm",		8206 },
    { "lsaquo",		8249 },
    { "lsquo",		8216 },
    { "lt",		'<' },
    { "macr",		175 },
    { "mdash",		8212 },
    { "micro",		181 },
    { "middot",		183 },
    { "minus",		8722 },
    { "mu",		956 },
    { "nabla",		8711 },
    { "nbsp",		160 },
    { "ndash",		8211 },
    { "ne",		8800 },
    { "ni",		8715 },
    { "not",		172 },
    { "notin",		8713 },
    { "nsub",		8836 },
    { "ntilde",		241 },
    { "nu",		957 },
    { "oacute",		243 },
    { "ocirc",		244 },
    { "oelig",		339 },
    { "ograve",		242 },
    { "oline",		8254 },
    { "omega",		969 },
    { "omicron",	959 },
    { "oplus",		8853 },
    { "or",		8744 },
    { "ordf",		170 },
    { "ordm",		186 },
    { "oslash",		248 },
    { "otilde",		245 },
    { "otimes",		8855 },
    { "ouml",		246 },
    { "para",		182 },
    { "part",		8706 },
    { "permil",		8240 },
    { "perp",		8869 },
    { "phi",		966 },
    { "pi",		960 },
    { "piv",		982 },
    { "plusmn",		177 },
    { "pound",		163 },
    { "prime",		8242 },
    { "prod",		8719 },
    { "prop",		8733 },
    { "psi",		968 },
    { "quot",		'\"' },
    { "rArr",		8658 },
    { "radic",		8730 },
    { "rang",		9002 },
    { "raquo",		187 },
    { "rarr",		8594 },
    { "rceil",		8969 },
    { "rdquo",		8221 },
    { "real",		8476 },
    { "reg",		174 },
    { "rfloor",		8971 },
    { "rho",		961 },
    { "rlm",		8207 },
    { "rsaquo",		8250 },
    { "rsquo",		8217 },
    { "sbquo",		8218 },
    { "scaron",		353 },
    { "sdot",		8901 },
    { "sect",		167 },
    { "shy",		173 },
    { "sigma",		963 },
    { "sigmaf",		962 },
    { "sim",		8764 },
    { "spades",		9824 },
    { "sub",		8834 },
    { "sube",		8838 },
    { "sum",		8721 },
    { "sup",		8835 },
    { "sup1",		185 },
    { "sup2",		178 },
    { "sup3",		179 },
    { "supe",		8839 },
    { "szlig",		223 },
    { "tau",		964 },
    { "there4",		8756 },
    { "theta",		952 },
    { "thetasym",	977 },
    { "thinsp",		8201 },
    { "thorn",		254 },
    { "tilde",		732 },
    { "times",		215 },
    { "trade",		8482 },
    { "uArr",		8657 },
    { "uacute",		250 },
    { "uarr",		8593 },
    { "ucirc",		251 },
    { "ugrave",		249 },
    { "uml",		168 },
    { "upsih",		978 },
    { "upsilon",	965 },
    { "uuml",		252 },
    { "weierp",		8472 },
    { "xi",		958 },
    { "yacute",		253 },
    { "yen",		165 },
    { "yuml",		255 },
    { "zeta",		950 },
    { "zwj",		8205 },
    { "zwnj",		8204 }
  };

 /*
  * Do a binary search for the named entity...
  */

  first = 0;
  last  = (int)(sizeof(entities) / sizeof(entities[0]) - 1);

  while ((last - first) > 1)
  {
    current = (first + last) / 2;

    if ((diff = strcmp(name, entities[current].name)) == 0)
      return (entities[current].val);
    else if (diff < 0)
      last = current;
    else
      first = current;
  }

 /*
  * If we get here, there is a small chance that there is still
  * a match; check first and last...
  */

  if (!strcmp(name, entities[first].name))
    return (entities[first].val);
  else if (!strcmp(name, entities[last].name))
    return (entities[last].val);
  else
    return (-1);
}

/*
 * mxml-file.c
 */
/*
 * Character encoding...
 */

#define ENCODE_UTF8	0		/* UTF-8 */
#define ENCODE_UTF16BE	1		/* UTF-16 Big-Endian */
#define ENCODE_UTF16LE	2		/* UTF-16 Little-Endian */

/*
 * Macro to test for a bad XML character...
 */

#define mxml_bad_char(ch) ((ch) < ' ' && (ch) != '\n' && (ch) != '\r' && (ch) != '\t')

/*
 * Types and structures...
 */

typedef int (*_mxml_getc_cb_t)(void *, int *);
typedef int (*_mxml_putc_cb_t)(int, void *);

typedef struct _mxml_fdbuf_s		/**** File descriptor buffer ****/
{
  int		fd;			/* File descriptor */
  unsigned char	*current,		/* Current position in buffer */
		*end,			/* End of buffer */
		buffer[8192];		/* Character buffer */
} _mxml_fdbuf_t;

/*
 * Local functions...
 */

static int		mxml_add_char(int ch, char **ptr, char **buffer, int *bufsize);
static int		mxml_fd_getc(void *p, int *encoding);
static int		mxml_fd_putc(int ch, void *p);
static int		mxml_fd_read(_mxml_fdbuf_t *buf);
static int		mxml_fd_write(_mxml_fdbuf_t *buf);
static int		mxml_file_getc(void *p, int *encoding);
static int		mxml_file_putc(int ch, void *p);
static int		mxml_get_entity(mxml_node_t *parent, void *p, int *encoding, _mxml_getc_cb_t getc_cb, int *line);
static inline int	mxml_isspace(int ch)
			{
			  return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
			}
static mxml_node_t	*mxml_load_data(mxml_node_t *top, void *p, mxml_load_cb_t cb, _mxml_getc_cb_t getc_cb, mxml_sax_cb_t sax_cb, void *sax_data);
static int		mxml_parse_element(mxml_node_t *node, void *p, int *encoding, _mxml_getc_cb_t getc_cb, int *line);
static int		mxml_string_getc(void *p, int *encoding);
static int		mxml_string_putc(int ch, void *p);
static int		mxml_write_name(const char *s, void *p, _mxml_putc_cb_t putc_cb);
static int		mxml_write_node(mxml_node_t *node, void *p, mxml_save_cb_t cb, int col, _mxml_putc_cb_t putc_cb, _mxml_global_t *global);
static int		mxml_write_string(const char *s, void *p, _mxml_putc_cb_t putc_cb);
static int		mxml_write_ws(mxml_node_t *node, void *p, mxml_save_cb_t cb, int ws, int col, _mxml_putc_cb_t putc_cb);

/*
 * 'mxmlLoadFd()' - Load a file descriptor into an XML node tree.
 *
 * The nodes in the specified file are added to the specified top node.
 * If no top node is provided, the XML file MUST be well-formed with a
 * single parent node like <?xml> for the entire file. The callback
 * function returns the value type that should be used for child nodes.
 * The constants @code MXML_INTEGER_CALLBACK@, @code MXML_OPAQUE_CALLBACK@,
 * @code MXML_REAL_CALLBACK@, and @code MXML_TEXT_CALLBACK@ are defined for
 * loading child (data) nodes of the specified type.
 *
 * Note: The most common programming error when using the Mini-XML library is
 * to load an XML file using the @code MXML_TEXT_CALLBACK@, which returns inline
 * text as a series of whitespace-delimited words, instead of using the
 * @code MXML_OPAQUE_CALLBACK@ which returns the inline text as a single string
 * (including whitespace).
 */

mxml_node_t *				/* O - First node or @code NULL@ if the file could not be read. */
mxmlLoadFd(mxml_node_t    *top,		/* I - Top node */
           int            fd,		/* I - File descriptor to read from */
           mxml_load_cb_t cb)		/* I - Callback function or constant */
{
  _mxml_fdbuf_t	buf;			/* File descriptor buffer */

 /*
  * Initialize the file descriptor buffer...
  */

  buf.fd      = fd;
  buf.current = buf.buffer;
  buf.end     = buf.buffer;

 /*
  * Read the XML data...
  */

  return (mxml_load_data(top, &buf, cb, mxml_fd_getc, MXML_NO_CALLBACK, NULL));
}

/*
 * 'mxmlLoadFile()' - Load a file into an XML node tree.
 *
 * The nodes in the specified file are added to the specified top node.
 * If no top node is provided, the XML file MUST be well-formed with a
 * single parent node like <?xml> for the entire file. The callback
 * function returns the value type that should be used for child nodes.
 * The constants @code MXML_INTEGER_CALLBACK@, @code MXML_OPAQUE_CALLBACK@,
 * @code MXML_REAL_CALLBACK@, and @code MXML_TEXT_CALLBACK@ are defined for
 * loading child (data) nodes of the specified type.
 *
 * Note: The most common programming error when using the Mini-XML library is
 * to load an XML file using the @code MXML_TEXT_CALLBACK@, which returns inline
 * text as a series of whitespace-delimited words, instead of using the
 * @code MXML_OPAQUE_CALLBACK@ which returns the inline text as a single string
 * (including whitespace).
 */

mxml_node_t *				/* O - First node or @code NULL@ if the file could not be read. */
mxmlLoadFile(mxml_node_t    *top,	/* I - Top node */
             FILE           *fp,	/* I - File to read from */
             mxml_load_cb_t cb)		/* I - Callback function or constant */
{
 /*
  * Read the XML data...
  */

  return (mxml_load_data(top, fp, cb, mxml_file_getc, MXML_NO_CALLBACK, NULL));
}

/*
 * 'mxmlLoadString()' - Load a string into an XML node tree.
 *
 * The nodes in the specified string are added to the specified top node.
 * If no top node is provided, the XML string MUST be well-formed with a
 * single parent node like <?xml> for the entire string. The callback
 * function returns the value type that should be used for child nodes.
 * The constants @code MXML_INTEGER_CALLBACK@, @code MXML_OPAQUE_CALLBACK@,
 * @code MXML_REAL_CALLBACK@, and @code MXML_TEXT_CALLBACK@ are defined for
 * loading child (data) nodes of the specified type.
 *
 * Note: The most common programming error when using the Mini-XML library is
 * to load an XML file using the @code MXML_TEXT_CALLBACK@, which returns inline
 * text as a series of whitespace-delimited words, instead of using the
 * @code MXML_OPAQUE_CALLBACK@ which returns the inline text as a single string
 * (including whitespace).
 */

mxml_node_t *				/* O - First node or @code NULL@ if the string has errors. */
mxmlLoadString(mxml_node_t    *top,	/* I - Top node */
               const char     *s,	/* I - String to load */
               mxml_load_cb_t cb)	/* I - Callback function or constant */
{
 /*
  * Read the XML data...
  */

  return (mxml_load_data(top, (void *)&s, cb, mxml_string_getc, MXML_NO_CALLBACK,
                         NULL));
}

/*
 * 'mxmlSaveAllocString()' - Save an XML tree to an allocated string.
 *
 * This function returns a pointer to a string containing the textual
 * representation of the XML node tree.  The string should be freed
 * using `free()` when you are done with it.  `NULL` is returned if the node
 * would produce an empty string or if the string cannot be allocated.
 *
 * The callback argument specifies a function that returns a whitespace
 * string or `NULL` before and after each element.  If `MXML_NO_CALLBACK`
 * is specified, whitespace will only be added before `MXML_TEXT` nodes
 * with leading whitespace and before attribute names inside opening
 * element tags.
 */

char *					/* O - Allocated string or @code NULL@ */
mxmlSaveAllocString(
    mxml_node_t    *node,		/* I - Node to write */
    mxml_save_cb_t cb)			/* I - Whitespace callback or @code MXML_NO_CALLBACK@ */
{
  int	bytes;				/* Required bytes */
  char	buffer[8192];			/* Temporary buffer */
  char	*s;				/* Allocated string */

 /*
  * Write the node to the temporary buffer...
  */

  bytes = mxmlSaveString(node, buffer, sizeof(buffer), cb);

  if (bytes <= 0)
    return (NULL);

  if (bytes < (int)(sizeof(buffer) - 1))
  {
   /*
    * Node fit inside the buffer, so just duplicate that string and
    * return...
    */

    return (strdup(buffer));
  }

 /*
  * Allocate a buffer of the required size and save the node to the
  * new buffer...
  */

  if ((s = malloc(bytes + 1)) == NULL)
    return (NULL);

  mxmlSaveString(node, s, bytes + 1, cb);

 /*
  * Return the allocated string...
  */

  return (s);
}

/*
 * 'mxmlSaveFd()' - Save an XML tree to a file descriptor.
 *
 * The callback argument specifies a function that returns a whitespace
 * string or NULL before and after each element. If @code MXML_NO_CALLBACK@
 * is specified, whitespace will only be added before @code MXML_TEXT@ nodes
 * with leading whitespace and before attribute names inside opening
 * element tags.
 */

int					/* O - 0 on success, -1 on error. */
mxmlSaveFd(mxml_node_t    *node,	/* I - Node to write */
           int            fd,		/* I - File descriptor to write to */
	   mxml_save_cb_t cb)		/* I - Whitespace callback or @code MXML_NO_CALLBACK@ */
{
  int		col;			/* Final column */
  _mxml_fdbuf_t	buf;			/* File descriptor buffer */
  _mxml_global_t *global = _mxml_global();
					/* Global data */

 /*
  * Initialize the file descriptor buffer...
  */

  buf.fd      = fd;
  buf.current = buf.buffer;
  buf.end     = buf.buffer + sizeof(buf.buffer);

 /*
  * Write the node...
  */

  if ((col = mxml_write_node(node, &buf, cb, 0, mxml_fd_putc, global)) < 0)
    return (-1);

  if (col > 0)
    if (mxml_fd_putc('\n', &buf) < 0)
      return (-1);

 /*
  * Flush and return...
  */

  return (mxml_fd_write(&buf));
}

/*
 * 'mxmlSaveFile()' - Save an XML tree to a file.
 *
 * The callback argument specifies a function that returns a whitespace
 * string or NULL before and after each element. If @code MXML_NO_CALLBACK@
 * is specified, whitespace will only be added before @code MXML_TEXT@ nodes
 * with leading whitespace and before attribute names inside opening
 * element tags.
 */

int					/* O - 0 on success, -1 on error. */
mxmlSaveFile(mxml_node_t    *node,	/* I - Node to write */
             FILE           *fp,	/* I - File to write to */
	     mxml_save_cb_t cb)		/* I - Whitespace callback or @code MXML_NO_CALLBACK@ */
{
  int	col;				/* Final column */
  _mxml_global_t *global = _mxml_global();
					/* Global data */

 /*
  * Write the node...
  */

  if ((col = mxml_write_node(node, fp, cb, 0, mxml_file_putc, global)) < 0)
    return (-1);

  if (col > 0)
    if (putc('\n', fp) < 0)
      return (-1);

 /*
  * Return 0 (success)...
  */

  return (0);
}

/*
 * 'mxmlSaveString()' - Save an XML node tree to a string.
 *
 * This function returns the total number of bytes that would be
 * required for the string but only copies (bufsize - 1) characters
 * into the specified buffer.
 *
 * The callback argument specifies a function that returns a whitespace
 * string or NULL before and after each element. If @code MXML_NO_CALLBACK@
 * is specified, whitespace will only be added before @code MXML_TEXT@ nodes
 * with leading whitespace and before attribute names inside opening
 * element tags.
 */

int					/* O - Size of string */
mxmlSaveString(mxml_node_t    *node,	/* I - Node to write */
               char           *buffer,	/* I - String buffer */
               int            bufsize,	/* I - Size of string buffer */
               mxml_save_cb_t cb)	/* I - Whitespace callback or @code MXML_NO_CALLBACK@ */
{
  int	col;				/* Final column */
  char	*ptr[2];			/* Pointers for putc_cb */
  _mxml_global_t *global = _mxml_global();
					/* Global data */

 /*
  * Write the node...
  */

  ptr[0] = buffer;
  ptr[1] = buffer + bufsize;

  if ((col = mxml_write_node(node, ptr, cb, 0, mxml_string_putc, global)) < 0)
    return (-1);

  if (col > 0)
    mxml_string_putc('\n', ptr);

 /*
  * Nul-terminate the buffer...
  */

  if (ptr[0] >= ptr[1])
  {
    if (bufsize > 0)
      buffer[bufsize - 1] = '\0';
  }
  else
    ptr[0][0] = '\0';

 /*
  * Return the number of characters...
  */

  return ((int)(ptr[0] - buffer));
}

/*
 * 'mxmlSAXLoadFd()' - Load a file descriptor into an XML node tree
 *                     using a SAX callback.
 *
 * The nodes in the specified file are added to the specified top node.
 * If no top node is provided, the XML file MUST be well-formed with a
 * single parent node like <?xml> for the entire file. The callback
 * function returns the value type that should be used for child nodes.
 * The constants @code MXML_INTEGER_CALLBACK@, @code MXML_OPAQUE_CALLBACK@,
 * @code MXML_REAL_CALLBACK@, and @code MXML_TEXT_CALLBACK@ are defined for
 * loading child nodes of the specified type.
 *
 * The SAX callback must call @link mxmlRetain@ for any nodes that need to
 * be kept for later use. Otherwise, nodes are deleted when the parent
 * node is closed or after each data, comment, CDATA, or directive node.
 *
 * @since Mini-XML 2.3@
 */

mxml_node_t *				/* O - First node or @code NULL@ if the file could not be read. */
mxmlSAXLoadFd(mxml_node_t    *top,	/* I - Top node */
              int            fd,	/* I - File descriptor to read from */
              mxml_load_cb_t cb,	/* I - Callback function or constant */
              mxml_sax_cb_t  sax_cb,	/* I - SAX callback or @code MXML_NO_CALLBACK@ */
              void           *sax_data)	/* I - SAX user data */
{
  _mxml_fdbuf_t	buf;			/* File descriptor buffer */

 /*
  * Initialize the file descriptor buffer...
  */

  buf.fd      = fd;
  buf.current = buf.buffer;
  buf.end     = buf.buffer;

 /*
  * Read the XML data...
  */

  return (mxml_load_data(top, &buf, cb, mxml_fd_getc, sax_cb, sax_data));
}

/*
 * 'mxmlSAXLoadFile()' - Load a file into an XML node tree
 *                       using a SAX callback.
 *
 * The nodes in the specified file are added to the specified top node.
 * If no top node is provided, the XML file MUST be well-formed with a
 * single parent node like <?xml> for the entire file. The callback
 * function returns the value type that should be used for child nodes.
 * The constants @code MXML_INTEGER_CALLBACK@, @code MXML_OPAQUE_CALLBACK@,
 * @code MXML_REAL_CALLBACK@, and @code MXML_TEXT_CALLBACK@ are defined for
 * loading child nodes of the specified type.
 *
 * The SAX callback must call @link mxmlRetain@ for any nodes that need to
 * be kept for later use. Otherwise, nodes are deleted when the parent
 * node is closed or after each data, comment, CDATA, or directive node.
 *
 * @since Mini-XML 2.3@
 */

mxml_node_t *				/* O - First node or @code NULL@ if the file could not be read. */
mxmlSAXLoadFile(
    mxml_node_t    *top,		/* I - Top node */
    FILE           *fp,			/* I - File to read from */
    mxml_load_cb_t cb,			/* I - Callback function or constant */
    mxml_sax_cb_t  sax_cb,		/* I - SAX callback or @code MXML_NO_CALLBACK@ */
    void           *sax_data)		/* I - SAX user data */
{
 /*
  * Read the XML data...
  */

  return (mxml_load_data(top, fp, cb, mxml_file_getc, sax_cb, sax_data));
}

/*
 * 'mxmlSAXLoadString()' - Load a string into an XML node tree
 *                         using a SAX callback.
 *
 * The nodes in the specified string are added to the specified top node.
 * If no top node is provided, the XML string MUST be well-formed with a
 * single parent node like <?xml> for the entire string. The callback
 * function returns the value type that should be used for child nodes.
 * The constants @code MXML_INTEGER_CALLBACK@, @code MXML_OPAQUE_CALLBACK@,
 * @code MXML_REAL_CALLBACK@, and @code MXML_TEXT_CALLBACK@ are defined for
 * loading child nodes of the specified type.
 *
 * The SAX callback must call @link mxmlRetain@ for any nodes that need to
 * be kept for later use. Otherwise, nodes are deleted when the parent
 * node is closed or after each data, comment, CDATA, or directive node.
 *
 * @since Mini-XML 2.3@
 */

mxml_node_t *				/* O - First node or @code NULL@ if the string has errors. */
mxmlSAXLoadString(
    mxml_node_t    *top,		/* I - Top node */
    const char     *s,			/* I - String to load */
    mxml_load_cb_t cb,			/* I - Callback function or constant */
    mxml_sax_cb_t  sax_cb,		/* I - SAX callback or @code MXML_NO_CALLBACK@ */
    void           *sax_data)		/* I - SAX user data */
{
 /*
  * Read the XML data...
  */

  return (mxml_load_data(top, (void *)&s, cb, mxml_string_getc, sax_cb, sax_data));
}

/*
 * 'mxmlSetCustomHandlers()' - Set the handling functions for custom data.
 *
 * The load function accepts a node pointer and a data string and must
 * return 0 on success and non-zero on error.
 *
 * The save function accepts a node pointer and must return a malloc'd
 * string on success and @code NULL@ on error.
 *
 */

void
mxmlSetCustomHandlers(
    mxml_custom_load_cb_t load,		/* I - Load function */
    mxml_custom_save_cb_t save)		/* I - Save function */
{
  _mxml_global_t *global = _mxml_global();
					/* Global data */

  global->custom_load_cb = load;
  global->custom_save_cb = save;
}

/*
 * 'mxmlSetErrorCallback()' - Set the error message callback.
 */

void
mxmlSetErrorCallback(mxml_error_cb_t cb)/* I - Error callback function */
{
  _mxml_global_t *global = _mxml_global();
					/* Global data */

  global->error_cb = cb;
}

/*
 * 'mxmlSetWrapMargin()' - Set the wrap margin when saving XML data.
 *
 * Wrapping is disabled when "column" is 0.
 *
 * @since Mini-XML 2.3@
 */

void
mxmlSetWrapMargin(int column)		/* I - Column for wrapping, 0 to disable wrapping */
{
  _mxml_global_t *global = _mxml_global();
					/* Global data */

  global->wrap = column;
}

/*
 * 'mxml_add_char()' - Add a character to a buffer, expanding as needed.
 */

static int				/* O  - 0 on success, -1 on error */
mxml_add_char(int  ch,			/* I  - Character to add */
              char **bufptr,		/* IO - Current position in buffer */
	      char **buffer,		/* IO - Current buffer */
	      int  *bufsize)		/* IO - Current buffer size */
{
  char	*newbuffer;			/* New buffer value */

  if (*bufptr >= (*buffer + *bufsize - 4))
  {
   /*
    * Increase the size of the buffer...
    */
    if (*bufsize < 1024)
      (*bufsize) *= 2;
    else
      (*bufsize) += 1024;
    if ((newbuffer = realloc(*buffer, *bufsize)) == NULL)
    {
      mxml_error("Unable to expand string buffer to %d bytes.", *bufsize);

      return (-1);
    }

    *bufptr = newbuffer + (*bufptr - *buffer);
    *buffer = newbuffer;
  }

  if (ch < 0x80)
  {
   /*
    * Single byte ASCII...
    */

    *(*bufptr)++ = ch;
  }
  else if (ch < 0x800)
  {
   /*
    * Two-byte UTF-8...
    */

    *(*bufptr)++ = 0xc0 | (ch >> 6);
    *(*bufptr)++ = 0x80 | (ch & 0x3f);
  }
  else if (ch < 0x10000)
  {
   /*
    * Three-byte UTF-8...
    */

    *(*bufptr)++ = 0xe0 | (ch >> 12);
    *(*bufptr)++ = 0x80 | ((ch >> 6) & 0x3f);
    *(*bufptr)++ = 0x80 | (ch & 0x3f);
  }
  else
  {
   /*
    * Four-byte UTF-8...
    */

    *(*bufptr)++ = 0xf0 | (ch >> 18);
    *(*bufptr)++ = 0x80 | ((ch >> 12) & 0x3f);
    *(*bufptr)++ = 0x80 | ((ch >> 6) & 0x3f);
    *(*bufptr)++ = 0x80 | (ch & 0x3f);
  }

  return (0);
}

/*
 * 'mxml_fd_getc()' - Read a character from a file descriptor.
 */

static int				/* O  - Character or EOF */
mxml_fd_getc(void *p,			/* I  - File descriptor buffer */
             int  *encoding)		/* IO - Encoding */
{
  _mxml_fdbuf_t	*buf;			/* File descriptor buffer */
  int		ch,			/* Current character */
		temp;			/* Temporary character */

 /*
  * Grab the next character in the buffer...
  */

  buf = (_mxml_fdbuf_t *)p;

  if (buf->current >= buf->end)
    if (mxml_fd_read(buf) < 0)
      return (EOF);

  ch = *(buf->current)++;

  switch (*encoding)
  {
    case ENCODE_UTF8 :
       /*
	* Got a UTF-8 character; convert UTF-8 to Unicode and return...
	*/

	if (!(ch & 0x80))
	{
#if DEBUG > 1
          printf("mxml_fd_getc: %c (0x%04x)\n", ch < ' ' ? '.' : ch, ch);
#endif /* DEBUG > 1 */

	  if (mxml_bad_char(ch))
	  {
	    mxml_error("Bad control character 0x%02x not allowed by XML standard.",
        	       ch);
	    return (EOF);
	  }

	  return (ch);
        }
	else if (ch == 0xfe)
	{
	 /*
	  * UTF-16 big-endian BOM?
	  */

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  ch = *(buf->current)++;

	  if (ch != 0xff)
	    return (EOF);

	  *encoding = ENCODE_UTF16BE;

	  return (mxml_fd_getc(p, encoding));
	}
	else if (ch == 0xff)
	{
	 /*
	  * UTF-16 little-endian BOM?
	  */

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  ch = *(buf->current)++;

	  if (ch != 0xfe)
	    return (EOF);

	  *encoding = ENCODE_UTF16LE;

	  return (mxml_fd_getc(p, encoding));
	}
	else if ((ch & 0xe0) == 0xc0)
	{
	 /*
	  * Two-byte value...
	  */

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  temp = *(buf->current)++;

	  if ((temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = ((ch & 0x1f) << 6) | (temp & 0x3f);

	  if (ch < 0x80)
	  {
	    mxml_error("Invalid UTF-8 sequence for character 0x%04x.", ch);
	    return (EOF);
	  }
	}
	else if ((ch & 0xf0) == 0xe0)
	{
	 /*
	  * Three-byte value...
	  */

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  temp = *(buf->current)++;

	  if ((temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = ((ch & 0x0f) << 6) | (temp & 0x3f);

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  temp = *(buf->current)++;

	  if ((temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = (ch << 6) | (temp & 0x3f);

	  if (ch < 0x800)
	  {
	    mxml_error("Invalid UTF-8 sequence for character 0x%04x.", ch);
	    return (EOF);
	  }

         /*
	  * Ignore (strip) Byte Order Mark (BOM)...
	  */

	  if (ch == 0xfeff)
	    return (mxml_fd_getc(p, encoding));
	}
	else if ((ch & 0xf8) == 0xf0)
	{
	 /*
	  * Four-byte value...
	  */

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  temp = *(buf->current)++;

	  if ((temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = ((ch & 0x07) << 6) | (temp & 0x3f);

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  temp = *(buf->current)++;

	  if ((temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = (ch << 6) | (temp & 0x3f);

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  temp = *(buf->current)++;

	  if ((temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = (ch << 6) | (temp & 0x3f);

	  if (ch < 0x10000)
	  {
	    mxml_error("Invalid UTF-8 sequence for character 0x%04x.", ch);
	    return (EOF);
	  }
	}
	else
	  return (EOF);
	break;

    case ENCODE_UTF16BE :
       /*
        * Read UTF-16 big-endian char...
	*/

	if (buf->current >= buf->end)
	  if (mxml_fd_read(buf) < 0)
	    return (EOF);

	temp = *(buf->current)++;

	ch = (ch << 8) | temp;

	if (mxml_bad_char(ch))
	{
	  mxml_error("Bad control character 0x%02x not allowed by XML standard.",
        	     ch);
	  return (EOF);
	}
        else if (ch >= 0xd800 && ch <= 0xdbff)
	{
	 /*
	  * Multi-word UTF-16 char...
	  */

          int lch;

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  lch = *(buf->current)++;

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  temp = *(buf->current)++;

	  lch = (lch << 8) | temp;

          if (lch < 0xdc00 || lch >= 0xdfff)
	    return (EOF);

          ch = (((ch & 0x3ff) << 10) | (lch & 0x3ff)) + 0x10000;
	}
	break;

    case ENCODE_UTF16LE :
       /*
        * Read UTF-16 little-endian char...
	*/

	if (buf->current >= buf->end)
	  if (mxml_fd_read(buf) < 0)
	    return (EOF);

	temp = *(buf->current)++;

	ch |= (temp << 8);

        if (mxml_bad_char(ch))
	{
	  mxml_error("Bad control character 0x%02x not allowed by XML standard.",
        	     ch);
	  return (EOF);
	}
        else if (ch >= 0xd800 && ch <= 0xdbff)
	{
	 /*
	  * Multi-word UTF-16 char...
	  */

          int lch;

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  lch = *(buf->current)++;

	  if (buf->current >= buf->end)
	    if (mxml_fd_read(buf) < 0)
	      return (EOF);

	  temp = *(buf->current)++;

	  lch |= (temp << 8);

          if (lch < 0xdc00 || lch >= 0xdfff)
	    return (EOF);

          ch = (((ch & 0x3ff) << 10) | (lch & 0x3ff)) + 0x10000;
	}
	break;
  }

#if DEBUG > 1
  printf("mxml_fd_getc: %c (0x%04x)\n", ch < ' ' ? '.' : ch, ch);
#endif /* DEBUG > 1 */

  return (ch);
}

/*
 * 'mxml_fd_putc()' - Write a character to a file descriptor.
 */

static int				/* O - 0 on success, -1 on error */
mxml_fd_putc(int  ch,			/* I - Character */
             void *p)			/* I - File descriptor buffer */
{
  _mxml_fdbuf_t	*buf;			/* File descriptor buffer */

 /*
  * Flush the write buffer as needed...
  */

  buf = (_mxml_fdbuf_t *)p;

  if (buf->current >= buf->end)
    if (mxml_fd_write(buf) < 0)
      return (-1);

  *(buf->current)++ = ch;

 /*
  * Return successfully...
  */

  return (0);
}

/*
 * 'mxml_fd_read()' - Read a buffer of data from a file descriptor.
 */

static int				/* O - 0 on success, -1 on error */
mxml_fd_read(_mxml_fdbuf_t *buf)		/* I - File descriptor buffer */
{
  int	bytes;				/* Bytes read... */

 /*
  * Range check input...
  */

  if (!buf)
    return (-1);

 /*
  * Read from the file descriptor...
  */

  while ((bytes = (int)read(buf->fd, buf->buffer, sizeof(buf->buffer))) < 0)
#ifdef EINTR
    if (errno != EAGAIN && errno != EINTR)
#else
    if (errno != EAGAIN)
#endif /* EINTR */
      return (-1);

  if (bytes == 0)
    return (-1);

 /*
  * Update the pointers and return success...
  */

  buf->current = buf->buffer;
  buf->end     = buf->buffer + bytes;

  return (0);
}

/*
 * 'mxml_fd_write()' - Write a buffer of data to a file descriptor.
 */

static int				/* O - 0 on success, -1 on error */
mxml_fd_write(_mxml_fdbuf_t *buf)	/* I - File descriptor buffer */
{
  int		bytes;			/* Bytes written */
  unsigned char	*ptr;			/* Pointer into buffer */

 /*
  * Range check...
  */

  if (!buf)
    return (-1);

 /*
  * Return 0 if there is nothing to write...
  */

  if (buf->current == buf->buffer)
    return (0);

 /*
  * Loop until we have written everything...
  */

  for (ptr = buf->buffer; ptr < buf->current; ptr += bytes)
    if ((bytes = (int)write(buf->fd, ptr, buf->current - ptr)) < 0)
      return (-1);

 /*
  * All done, reset pointers and return success...
  */

  buf->current = buf->buffer;

  return (0);
}

/*
 * 'mxml_file_getc()' - Get a character from a file.
 */

static int				/* O  - Character or EOF */
mxml_file_getc(void *p,			/* I  - Pointer to file */
               int  *encoding)		/* IO - Encoding */
{
  int	ch,				/* Character from file */
	temp;				/* Temporary character */
  FILE	*fp;				/* Pointer to file */

 /*
  * Read a character from the file and see if it is EOF or ASCII...
  */

  fp = (FILE *)p;
  ch = getc(fp);

  if (ch == EOF)
    return (EOF);

  switch (*encoding)
  {
    case ENCODE_UTF8 :
       /*
	* Got a UTF-8 character; convert UTF-8 to Unicode and return...
	*/

	if (!(ch & 0x80))
	{
	  if (mxml_bad_char(ch))
	  {
	    mxml_error("Bad control character 0x%02x not allowed by XML standard.",
        	       ch);
	    return (EOF);
	  }

#if DEBUG > 1
          printf("mxml_file_getc: %c (0x%04x)\n", ch < ' ' ? '.' : ch, ch);
#endif /* DEBUG > 1 */

	  return (ch);
        }
	else if (ch == 0xfe)
	{
	 /*
	  * UTF-16 big-endian BOM?
	  */

          ch = getc(fp);
	  if (ch != 0xff)
	    return (EOF);

	  *encoding = ENCODE_UTF16BE;

	  return (mxml_file_getc(p, encoding));
	}
	else if (ch == 0xff)
	{
	 /*
	  * UTF-16 little-endian BOM?
	  */

          ch = getc(fp);
	  if (ch != 0xfe)
	    return (EOF);

	  *encoding = ENCODE_UTF16LE;

	  return (mxml_file_getc(p, encoding));
	}
	else if ((ch & 0xe0) == 0xc0)
	{
	 /*
	  * Two-byte value...
	  */

	  if ((temp = getc(fp)) == EOF || (temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = ((ch & 0x1f) << 6) | (temp & 0x3f);

	  if (ch < 0x80)
	  {
	    mxml_error("Invalid UTF-8 sequence for character 0x%04x.", ch);
	    return (EOF);
	  }
	}
	else if ((ch & 0xf0) == 0xe0)
	{
	 /*
	  * Three-byte value...
	  */

	  if ((temp = getc(fp)) == EOF || (temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = ((ch & 0x0f) << 6) | (temp & 0x3f);

	  if ((temp = getc(fp)) == EOF || (temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = (ch << 6) | (temp & 0x3f);

	  if (ch < 0x800)
	  {
	    mxml_error("Invalid UTF-8 sequence for character 0x%04x.", ch);
	    return (EOF);
	  }

         /*
	  * Ignore (strip) Byte Order Mark (BOM)...
	  */

	  if (ch == 0xfeff)
	    return (mxml_file_getc(p, encoding));
	}
	else if ((ch & 0xf8) == 0xf0)
	{
	 /*
	  * Four-byte value...
	  */

	  if ((temp = getc(fp)) == EOF || (temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = ((ch & 0x07) << 6) | (temp & 0x3f);

	  if ((temp = getc(fp)) == EOF || (temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = (ch << 6) | (temp & 0x3f);

	  if ((temp = getc(fp)) == EOF || (temp & 0xc0) != 0x80)
	    return (EOF);

	  ch = (ch << 6) | (temp & 0x3f);

	  if (ch < 0x10000)
	  {
	    mxml_error("Invalid UTF-8 sequence for character 0x%04x.", ch);
	    return (EOF);
	  }
	}
	else
	  return (EOF);
	break;

    case ENCODE_UTF16BE :
       /*
        * Read UTF-16 big-endian char...
	*/

	ch = (ch << 8) | getc(fp);

	if (mxml_bad_char(ch))
	{
	  mxml_error("Bad control character 0x%02x not allowed by XML standard.",
        	     ch);
	  return (EOF);
	}
        else if (ch >= 0xd800 && ch <= 0xdbff)
	{
	 /*
	  * Multi-word UTF-16 char...
	  */

          int lch = getc(fp);
          lch = (lch << 8) | getc(fp);

          if (lch < 0xdc00 || lch >= 0xdfff)
	    return (EOF);

          ch = (((ch & 0x3ff) << 10) | (lch & 0x3ff)) + 0x10000;
	}
	break;

    case ENCODE_UTF16LE :
       /*
        * Read UTF-16 little-endian char...
	*/

	ch |= (getc(fp) << 8);

        if (mxml_bad_char(ch))
	{
	  mxml_error("Bad control character 0x%02x not allowed by XML standard.",
        	     ch);
	  return (EOF);
	}
        else if (ch >= 0xd800 && ch <= 0xdbff)
	{
	 /*
	  * Multi-word UTF-16 char...
	  */

          int lch = getc(fp);
          lch |= (getc(fp) << 8);

          if (lch < 0xdc00 || lch >= 0xdfff)
	    return (EOF);

          ch = (((ch & 0x3ff) << 10) | (lch & 0x3ff)) + 0x10000;
	}
	break;
  }

#if DEBUG > 1
  printf("mxml_file_getc: %c (0x%04x)\n", ch < ' ' ? '.' : ch, ch);
#endif /* DEBUG > 1 */

  return (ch);
}

/*
 * 'mxml_file_putc()' - Write a character to a file.
 */

static int				/* O - 0 on success, -1 on failure */
mxml_file_putc(int  ch,			/* I - Character to write */
               void *p)			/* I - Pointer to file */
{
  return (putc(ch, (FILE *)p) == EOF ? -1 : 0);
}

/*
 * 'mxml_get_entity()' - Get the character corresponding to an entity...
 */

static int				/* O  - Character value or EOF on error */
mxml_get_entity(mxml_node_t *parent,	/* I  - Parent node */
		void        *p,		/* I  - Pointer to source */
		int         *encoding,	/* IO - Character encoding */
                int         (*getc_cb)(void *, int *),
					/* I  - Get character function */
                int         *line)	/* IO - Current line number */
{
  int	ch;				/* Current character */
  char	entity[64],			/* Entity string */
	*entptr;			/* Pointer into entity */

  entptr = entity;

  while ((ch = (*getc_cb)(p, encoding)) != EOF)
  {
    if (ch > 126 || (!isalnum(ch) && ch != '#'))
      break;
    else if (entptr < (entity + sizeof(entity) - 1))
      *entptr++ = ch;
    else
    {
      mxml_error("Entity name too long under parent <%s> on line %d.", parent ? parent->value.element.name : "null", *line);
      break;
    }
  }

  *entptr = '\0';

  if (ch != ';')
  {
    mxml_error("Character entity '%s' not terminated under parent <%s> on line %d.", entity, parent ? parent->value.element.name : "null", *line);

    if (ch == '\n')
      (*line)++;

    return (EOF);
  }

  if (entity[0] == '#')
  {
    if (entity[1] == 'x')
      ch = (int)strtol(entity + 2, NULL, 16);
    else
      ch = (int)strtol(entity + 1, NULL, 10);
  }
  else if ((ch = mxmlEntityGetValue(entity)) < 0)
    mxml_error("Entity name '%s;' not supported under parent <%s> on line %d.", entity, parent ? parent->value.element.name : "null", *line);

  if (mxml_bad_char(ch))
  {
    mxml_error("Bad control character 0x%02x under parent <%s> on line %d not allowed by XML standard.", ch, parent ? parent->value.element.name : "null", *line);
    return (EOF);
  }

  return (ch);
}

/*
 * 'mxml_load_data()' - Load data into an XML node tree.
 */

static mxml_node_t *			/* O - First node or NULL if the file could not be read. */
mxml_load_data(
    mxml_node_t     *top,		/* I - Top node */
    void            *p,			/* I - Pointer to data */
    mxml_load_cb_t  cb,			/* I - Callback function or MXML_NO_CALLBACK */
    _mxml_getc_cb_t getc_cb,		/* I - Read function */
    mxml_sax_cb_t   sax_cb,		/* I - SAX callback or MXML_NO_CALLBACK */
    void            *sax_data)		/* I - SAX user data */
{
  mxml_node_t	*node = NULL,		/* Current node */
		*first = NULL,		/* First node added */
		*parent = NULL;		/* Current parent node */
  int		line = 1,		/* Current line number */
		ch,			/* Character from file */
		whitespace;		/* Non-zero if whitespace seen */
  char		*buffer,		/* String buffer */
		*bufptr;		/* Pointer into buffer */
  int		bufsize;		/* Size of buffer */
  mxml_type_t	type;			/* Current node type */
  int		encoding;		/* Character encoding */
  _mxml_global_t *global = _mxml_global();
					/* Global data */
  static const char * const types[] =	/* Type strings... */
		{
		  "MXML_ELEMENT",	/* XML element with attributes */
		  "MXML_INTEGER",	/* Integer value */
		  "MXML_OPAQUE",	/* Opaque string */
		  "MXML_REAL",		/* Real value */
		  "MXML_TEXT",		/* Text fragment */
		  "MXML_CUSTOM"		/* Custom data */
		};

 /*
  * Read elements and other nodes from the file...
  */

  if ((buffer = malloc(64)) == NULL)
  {
    mxml_error("Unable to allocate string buffer.");
    return (NULL);
  }

  bufsize    = 64;
  bufptr     = buffer;
  parent     = top;
  first      = NULL;
  whitespace = 0;
  encoding   = ENCODE_UTF8;

  if (cb && parent)
    type = (*cb)(parent);
  else if (parent)
    type = MXML_TEXT;
  else
    type = MXML_IGNORE;

  if ((ch = (*getc_cb)(p, &encoding)) == EOF)
  {
    free(buffer);
    return (NULL);
  }
  else if (ch != '<' && !top)
  {
    free(buffer);
    mxml_error("XML does not start with '<' (saw '%c').", ch);
    return (NULL);
  }

  do
  {
    if ((ch == '<' ||
         (mxml_isspace(ch) && type != MXML_OPAQUE && type != MXML_CUSTOM)) &&
        bufptr > buffer)
    {
     /*
      * Add a new value node...
      */

      *bufptr = '\0';

      switch (type)
      {
	case MXML_INTEGER :
            node = mxmlNewInteger(parent, (int)strtol(buffer, &bufptr, 0));
	    break;

	case MXML_OPAQUE :
            node = mxmlNewOpaque(parent, buffer);
	    break;

	case MXML_REAL :
            node = mxmlNewReal(parent, strtod(buffer, &bufptr));
	    break;

	case MXML_TEXT :
            node = mxmlNewText(parent, whitespace, buffer);
	    break;

	case MXML_CUSTOM :
	    if (global->custom_load_cb)
	    {
	     /*
	      * Use the callback to fill in the custom data...
	      */

              node = mxmlNewCustom(parent, NULL, NULL);

	      if ((*global->custom_load_cb)(node, buffer))
	      {
	        mxml_error("Bad custom value '%s' in parent <%s> on line %d.", buffer, parent ? parent->value.element.name : "null", line);
		mxmlDelete(node);
		node = NULL;
	      }
	    }
	    break;

        default : /* Ignore... */
	    node = NULL;
	    break;
      }

      if (*bufptr)
      {
       /*
        * Bad integer/real number value...
	*/

        mxml_error("Bad %s value '%s' in parent <%s> on line %d.", type == MXML_INTEGER ? "integer" : "real", buffer, parent ? parent->value.element.name : "null", line);
	break;
      }

      bufptr     = buffer;
      whitespace = mxml_isspace(ch) && type == MXML_TEXT;

      if (!node && type != MXML_IGNORE)
      {
       /*
	* Print error and return...
	*/

	mxml_error("Unable to add value node of type %s to parent <%s> on line %d.", types[type], parent ? parent->value.element.name : "null", line);
	goto error;
      }

      if (sax_cb)
      {
        (*sax_cb)(node, MXML_SAX_DATA, sax_data);

        if (!mxmlRelease(node))
          node = NULL;
      }

      if (!first && node)
        first = node;
    }
    else if (mxml_isspace(ch) && type == MXML_TEXT)
      whitespace = 1;

    if (ch == '\n')
      line ++;

   /*
    * Add lone whitespace node if we have an element and existing
    * whitespace...
    */

    if (ch == '<' && whitespace && type == MXML_TEXT)
    {
      if (parent)
      {
	node = mxmlNewText(parent, whitespace, "");

	if (sax_cb)
	{
	  (*sax_cb)(node, MXML_SAX_DATA, sax_data);

	  if (!mxmlRelease(node))
	    node = NULL;
	}

	if (!first && node)
	  first = node;
      }

      whitespace = 0;
    }

    if (ch == '<')
    {
     /*
      * Start of open/close tag...
      */

      bufptr = buffer;

      while ((ch = (*getc_cb)(p, &encoding)) != EOF)
      {
        if (mxml_isspace(ch) || ch == '>' || (ch == '/' && bufptr > buffer))
	  break;
	else if (ch == '<')
	{
	  mxml_error("Bare < in element.");
	  goto error;
	}
	else if (ch == '&')
	{
	  if ((ch = mxml_get_entity(parent, p, &encoding, getc_cb, &line)) == EOF)
	    goto error;

	  if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
	    goto error;
	}
	else if (ch < '0' && ch != '!' && ch != '-' && ch != '.' && ch != '/')
	  goto error;
	else if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
	  goto error;
	else if (((bufptr - buffer) == 1 && buffer[0] == '?') ||
	         ((bufptr - buffer) == 3 && !strncmp(buffer, "!--", 3)) ||
	         ((bufptr - buffer) == 8 && !strncmp(buffer, "![CDATA[", 8)))
	  break;

	if (ch == '\n')
	  line ++;
      }

      *bufptr = '\0';

      if (!strcmp(buffer, "!--"))
      {
       /*
        * Gather rest of comment...
	*/

	while ((ch = (*getc_cb)(p, &encoding)) != EOF)
	{
	  if (ch == '>' && bufptr > (buffer + 4) &&
	      bufptr[-3] != '-' && bufptr[-2] == '-' && bufptr[-1] == '-')
	    break;
	  else if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
	    goto error;

	  if (ch == '\n')
	    line ++;
	}

       /*
        * Error out if we didn't get the whole comment...
	*/

        if (ch != '>')
	{
	 /*
	  * Print error and return...
	  */

	  mxml_error("Early EOF in comment node on line %d.", line);
	  goto error;
	}

       /*
        * Otherwise add this as an element under the current parent...
	*/

	*bufptr = '\0';

        if (!parent && first)
	{
	 /*
	  * There can only be one root element!
	  */

	  mxml_error("<%s> cannot be a second root node after <%s> on line %d.", buffer, first->value.element.name, line);
          goto error;
	}

	if ((node = mxmlNewElement(parent, buffer)) == NULL)
	{
	 /*
	  * Just print error for now...
	  */

	  mxml_error("Unable to add comment node to parent <%s> on line %d.", parent ? parent->value.element.name : "null", line);
	  break;
	}

        if (sax_cb)
        {
          (*sax_cb)(node, MXML_SAX_COMMENT, sax_data);

          if (!mxmlRelease(node))
            node = NULL;
        }

	if (node && !first)
	  first = node;
      }
      else if (!strcmp(buffer, "![CDATA["))
      {
       /*
        * Gather CDATA section...
	*/

	while ((ch = (*getc_cb)(p, &encoding)) != EOF)
	{
	  if (ch == '>' && !strncmp(bufptr - 2, "]]", 2))
	  {
	   /*
	    * Drop terminator from CDATA string...
	    */

	    bufptr[-2] = '\0';
	    break;
	  }
	  else if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
	    goto error;

	  if (ch == '\n')
	    line ++;
	}

       /*
        * Error out if we didn't get the whole comment...
	*/

        if (ch != '>')
	{
	 /*
	  * Print error and return...
	  */

	  mxml_error("Early EOF in CDATA node on line %d.", line);
	  goto error;
	}

       /*
        * Otherwise add this as an element under the current parent...
	*/

	*bufptr = '\0';

        if (!parent && first)
	{
	 /*
	  * There can only be one root element!
	  */

	  mxml_error("<%s> cannot be a second root node after <%s> on line %d.", buffer, first->value.element.name, line);
          goto error;
	}

	if ((node = mxmlNewElement(parent, buffer)) == NULL)
	{
	 /*
	  * Print error and return...
	  */

	  mxml_error("Unable to add CDATA node to parent <%s> on line %d.", parent ? parent->value.element.name : "null", line);
	  goto error;
	}

        if (sax_cb)
        {
          (*sax_cb)(node, MXML_SAX_CDATA, sax_data);

          if (!mxmlRelease(node))
            node = NULL;
        }

	if (node && !first)
	  first = node;
      }
      else if (buffer[0] == '?')
      {
       /*
        * Gather rest of processing instruction...
	*/

	while ((ch = (*getc_cb)(p, &encoding)) != EOF)
	{
	  if (ch == '>' && bufptr > buffer && bufptr[-1] == '?')
	    break;
	  else if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
	    goto error;

	  if (ch == '\n')
	    line ++;
	}

       /*
        * Error out if we didn't get the whole processing instruction...
	*/

        if (ch != '>')
	{
	 /*
	  * Print error and return...
	  */

	  mxml_error("Early EOF in processing instruction node on line %d.", line);
	  goto error;
	}

       /*
        * Otherwise add this as an element under the current parent...
	*/

	*bufptr = '\0';

        if (!parent && first)
	{
	 /*
	  * There can only be one root element!
	  */

	  mxml_error("<%s> cannot be a second root node after <%s> on line %d.", buffer, first->value.element.name, line);
          goto error;
	}

	if ((node = mxmlNewElement(parent, buffer)) == NULL)
	{
	 /*
	  * Print error and return...
	  */

	  mxml_error("Unable to add processing instruction node to parent <%s> on line %d.", parent ? parent->value.element.name : "null", line);
	  goto error;
	}

        if (sax_cb)
        {
          (*sax_cb)(node, MXML_SAX_DIRECTIVE, sax_data);

          if (strncmp(node->value.element.name, "?xml ", 5) && !mxmlRelease(node))
            node = NULL;
        }

        if (node)
	{
	  if (!first)
            first = node;

	  if (!parent)
	  {
	    parent = node;

	    if (cb)
	      type = (*cb)(parent);
	    else
	      type = MXML_TEXT;
	  }
	}
      }
      else if (buffer[0] == '!')
      {
       /*
        * Gather rest of declaration...
	*/

	do
	{
	  if (ch == '>')
	    break;
	  else
	  {
            if (ch == '&')
            {
	      if ((ch = mxml_get_entity(parent, p, &encoding, getc_cb, &line)) == EOF)
		goto error;
            }

	    if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
	      goto error;
	  }

	  if (ch == '\n')
	    line ++;
	}
        while ((ch = (*getc_cb)(p, &encoding)) != EOF);

       /*
        * Error out if we didn't get the whole declaration...
	*/

        if (ch != '>')
	{
	 /*
	  * Print error and return...
	  */

	  mxml_error("Early EOF in declaration node on line %d.", line);
	  goto error;
	}

       /*
        * Otherwise add this as an element under the current parent...
	*/

	*bufptr = '\0';

        if (!parent && first)
	{
	 /*
	  * There can only be one root element!
	  */

	  mxml_error("<%s> cannot be a second root node after <%s> on line %d.", buffer, first->value.element.name, line);
          goto error;
	}

	if ((node = mxmlNewElement(parent, buffer)) == NULL)
	{
	 /*
	  * Print error and return...
	  */

	  mxml_error("Unable to add declaration node to parent <%s> on line %d.", parent ? parent->value.element.name : "null", line);
	  goto error;
	}

        if (sax_cb)
        {
          (*sax_cb)(node, MXML_SAX_DIRECTIVE, sax_data);

          if (!mxmlRelease(node))
            node = NULL;
        }

        if (node)
	{
	  if (!first)
            first = node;

	  if (!parent)
	  {
	    parent = node;

	    if (cb)
	      type = (*cb)(parent);
	    else
	      type = MXML_TEXT;
	  }
	}
      }
      else if (buffer[0] == '/')
      {
       /*
        * Handle close tag...
	*/

        if (!parent || strcmp(buffer + 1, parent->value.element.name))
	{
	 /*
	  * Close tag doesn't match tree; print an error for now...
	  */

	  mxml_error("Mismatched close tag <%s> under parent <%s> on line %d.", buffer, parent ? parent->value.element.name : "(null)", line);
          goto error;
	}

       /*
        * Keep reading until we see >...
	*/

        while (ch != '>' && ch != EOF)
	  ch = (*getc_cb)(p, &encoding);

        node   = parent;
        parent = parent->parent;

        if (sax_cb)
        {
          (*sax_cb)(node, MXML_SAX_ELEMENT_CLOSE, sax_data);

          if (!mxmlRelease(node))
          {
            if (first == node)
	      first = NULL;

	    node = NULL;
	  }
        }

       /*
	* Ascend into the parent and set the value type as needed...
	*/

	if (cb && parent)
	  type = (*cb)(parent);
      }
      else
      {
       /*
        * Handle open tag...
	*/

        if (!parent && first)
	{
	 /*
	  * There can only be one root element!
	  */

	  mxml_error("<%s> cannot be a second root node after <%s> on line %d.", buffer, first->value.element.name, line);
          goto error;
	}

        if ((node = mxmlNewElement(parent, buffer)) == NULL)
	{
	 /*
	  * Just print error for now...
	  */

	  mxml_error("Unable to add element node to parent <%s> on line %d.", parent ? parent->value.element.name : "null", line);
	  goto error;
	}

        if (mxml_isspace(ch))
        {
	  if ((ch = mxml_parse_element(node, p, &encoding, getc_cb, &line)) == EOF)
	    goto error;
        }
        else if (ch == '/')
	{
	  if ((ch = (*getc_cb)(p, &encoding)) != '>')
	  {
	    mxml_error("Expected > but got '%c' instead for element <%s/> on line %d.", ch, buffer, line);
            mxmlDelete(node);
            node = NULL;
            goto error;
	  }

	  ch = '/';
	}

        if (sax_cb)
          (*sax_cb)(node, MXML_SAX_ELEMENT_OPEN, sax_data);

        if (!first)
	  first = node;

	if (ch == EOF)
	  break;

        if (ch != '/')
	{
	 /*
	  * Descend into this node, setting the value type as needed...
	  */

	  parent = node;

	  if (cb && parent)
	    type = (*cb)(parent);
	  else
	    type = MXML_TEXT;
	}
        else if (sax_cb)
        {
          (*sax_cb)(node, MXML_SAX_ELEMENT_CLOSE, sax_data);

          if (!mxmlRelease(node))
          {
            if (first == node)
	      first = NULL;

	    node = NULL;
	  }
        }
      }

      bufptr  = buffer;
    }
    else if (ch == '&')
    {
     /*
      * Add character entity to current buffer...
      */

      if ((ch = mxml_get_entity(parent, p, &encoding, getc_cb, &line)) == EOF)
	goto error;

      if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
	goto error;
    }
    else if (type == MXML_OPAQUE || type == MXML_CUSTOM || !mxml_isspace(ch))
    {
     /*
      * Add character to current buffer...
      */

      if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
	goto error;
    }
  }
  while ((ch = (*getc_cb)(p, &encoding)) != EOF);

 /*
  * Free the string buffer - we don't need it anymore...
  */

  free(buffer);

 /*
  * Find the top element and return it...
  */

  if (parent)
  {
    node = parent;

    while (parent != top && parent->parent)
      parent = parent->parent;

    if (node != parent)
    {
      mxml_error("Missing close tag </%s> under parent <%s> on line %d.", node->value.element.name, node->parent ? node->parent->value.element.name : "(null)", line);

      mxmlDelete(first);

      return (NULL);
    }
  }

  if (parent)
    return (parent);
  else
    return (first);

 /*
  * Common error return...
  */

  error:

  mxmlDelete(first);

  free(buffer);

  return (NULL);
}

/*
 * 'mxml_parse_element()' - Parse an element for any attributes...
 */

static int				/* O  - Terminating character */
mxml_parse_element(
    mxml_node_t     *node,		/* I  - Element node */
    void            *p,			/* I  - Data to read from */
    int             *encoding,		/* IO - Encoding */
    _mxml_getc_cb_t getc_cb,		/* I  - Data callback */
    int             *line)		/* IO - Current line number */
{
  int	ch,				/* Current character in file */
	quote;				/* Quoting character */
  char	*name,				/* Attribute name */
	*value,				/* Attribute value */
	*ptr;				/* Pointer into name/value */
  int	namesize,			/* Size of name string */
	valsize;			/* Size of value string */

 /*
  * Initialize the name and value buffers...
  */

  if ((name = malloc(64)) == NULL)
  {
    mxml_error("Unable to allocate memory for name.");
    return (EOF);
  }

  namesize = 64;

  if ((value = malloc(64)) == NULL)
  {
    free(name);
    mxml_error("Unable to allocate memory for value.");
    return (EOF);
  }

  valsize = 64;

 /*
  * Loop until we hit a >, /, ?, or EOF...
  */

  while ((ch = (*getc_cb)(p, encoding)) != EOF)
  {
#if DEBUG > 1
    fprintf(stderr, "parse_element: ch='%c'\n", ch);
#endif /* DEBUG > 1 */

   /*
    * Skip leading whitespace...
    */

    if (mxml_isspace(ch))
    {
      if (ch == '\n')
        (*line)++;

      continue;
    }

   /*
    * Stop at /, ?, or >...
    */

    if (ch == '/' || ch == '?')
    {
     /*
      * Grab the > character and print an error if it isn't there...
      */

      quote = (*getc_cb)(p, encoding);

      if (quote != '>')
      {
        mxml_error("Expected '>' after '%c' for element %s, but got '%c' on line %d.", ch, node->value.element.name, quote, *line);
        goto error;
      }

      break;
    }
    else if (ch == '<')
    {
      mxml_error("Bare < in element %s on line %d.", node->value.element.name, *line);
      goto error;
    }
    else if (ch == '>')
      break;

   /*
    * Read the attribute name...
    */

    ptr = name;
    if (mxml_add_char(ch, &ptr, &name, &namesize))
      goto error;

    if (ch == '\"' || ch == '\'')
    {
     /*
      * Name is in quotes, so get a quoted string...
      */

      quote = ch;

      while ((ch = (*getc_cb)(p, encoding)) != EOF)
      {
        if (ch == '&')
        {
	  if ((ch = mxml_get_entity(node, p, encoding, getc_cb, line)) == EOF)
	    goto error;
	}
	else if (ch == '\n')
	  (*line)++;

	if (mxml_add_char(ch, &ptr, &name, &namesize))
	  goto error;

	if (ch == quote)
          break;
      }
    }
    else
    {
     /*
      * Grab an normal, non-quoted name...
      */

      while ((ch = (*getc_cb)(p, encoding)) != EOF)
      {
	if (mxml_isspace(ch) || ch == '=' || ch == '/' || ch == '>' ||
	    ch == '?')
	{
	  if (ch == '\n')
	    (*line)++;
          break;
        }
	else
	{
          if (ch == '&')
          {
	    if ((ch = mxml_get_entity(node, p, encoding, getc_cb, line)) == EOF)
	      goto error;
          }

	  if (mxml_add_char(ch, &ptr, &name, &namesize))
	    goto error;
	}
      }
    }

    *ptr = '\0';

    if (mxmlElementGetAttr(node, name))
    {
      mxml_error("Duplicate attribute '%s' in element %s on line %d.", name, node->value.element.name, *line);
      goto error;
    }

    while (ch != EOF && mxml_isspace(ch))
    {
      ch = (*getc_cb)(p, encoding);

      if (ch == '\n')
        (*line)++;
    }

    if (ch == '=')
    {
     /*
      * Read the attribute value...
      */

      while ((ch = (*getc_cb)(p, encoding)) != EOF && mxml_isspace(ch))
      {
        if (ch == '\n')
          (*line)++;
      }

      if (ch == EOF)
      {
        mxml_error("Missing value for attribute '%s' in element %s on line %d.", name, node->value.element.name, *line);
        goto error;
      }

      if (ch == '\'' || ch == '\"')
      {
       /*
        * Read quoted value...
	*/

        quote = ch;
	ptr   = value;

        while ((ch = (*getc_cb)(p, encoding)) != EOF)
        {
	  if (ch == quote)
	  {
	    break;
	  }
	  else
	  {
	    if (ch == '&')
	    {
	      if ((ch = mxml_get_entity(node, p, encoding, getc_cb, line)) == EOF)
	        goto error;
	    }
	    else if (ch == '\n')
	      (*line)++;

	    if (mxml_add_char(ch, &ptr, &value, &valsize))
	      goto error;
	  }
	}

        *ptr = '\0';
      }
      else
      {
       /*
        * Read unquoted value...
	*/

	ptr      = value;
	if (mxml_add_char(ch, &ptr, &value, &valsize))
	  goto error;

	while ((ch = (*getc_cb)(p, encoding)) != EOF)
	{
	  if (mxml_isspace(ch) || ch == '=' || ch == '/' || ch == '>')
	  {
	    if (ch == '\n')
	      (*line)++;

            break;
          }
	  else
	  {
	    if (ch == '&')
	    {
	      if ((ch = mxml_get_entity(node, p, encoding, getc_cb, line)) == EOF)
	        goto error;
	    }

	    if (mxml_add_char(ch, &ptr, &value, &valsize))
	      goto error;
	  }
	}

        *ptr = '\0';
      }

     /*
      * Set the attribute with the given string value...
      */

      mxmlElementSetAttr(node, name, value);
    }
    else
    {
      mxml_error("Missing value for attribute '%s' in element %s on line %d.", name, node->value.element.name, *line);
      goto error;
    }

   /*
    * Check the end character...
    */

    if (ch == '/' || ch == '?')
    {
     /*
      * Grab the > character and print an error if it isn't there...
      */

      quote = (*getc_cb)(p, encoding);

      if (quote != '>')
      {
        mxml_error("Expected '>' after '%c' for element %s, but got '%c' on line %d.", ch, node->value.element.name, quote, *line);
        ch = EOF;
      }

      break;
    }
    else if (ch == '>')
      break;
  }

 /*
  * Free the name and value buffers and return...
  */

  free(name);
  free(value);

  return (ch);

 /*
  * Common error return point...
  */

  error:

  free(name);
  free(value);

  return (EOF);
}

/*
 * 'mxml_string_getc()' - Get a character from a string.
 */

static int				/* O  - Character or EOF */
mxml_string_getc(void *p,		/* I  - Pointer to file */
                 int  *encoding)	/* IO - Encoding */
{
  int		ch;			/* Character */
  const char	**s;			/* Pointer to string pointer */

  s = (const char **)p;

  if ((ch = (*s)[0] & 255) != 0 || *encoding == ENCODE_UTF16LE)
  {
   /*
    * Got character; convert UTF-8 to integer and return...
    */

    (*s)++;

    switch (*encoding)
    {
      case ENCODE_UTF8 :
	  if (!(ch & 0x80))
	  {
#if DEBUG > 1
            printf("mxml_string_getc: %c (0x%04x)\n", ch < ' ' ? '.' : ch, ch);
#endif /* DEBUG > 1 */

	    if (mxml_bad_char(ch))
	    {
	      mxml_error("Bad control character 0x%02x not allowed by XML standard.",
        		 ch);
	      return (EOF);
	    }

	    return (ch);
          }
	  else if (ch == 0xfe)
	  {
	   /*
	    * UTF-16 big-endian BOM?
	    */

            if (((*s)[0] & 255) != 0xff)
	      return (EOF);

	    *encoding = ENCODE_UTF16BE;
	    (*s)++;

	    return (mxml_string_getc(p, encoding));
	  }
	  else if (ch == 0xff)
	  {
	   /*
	    * UTF-16 little-endian BOM?
	    */

            if (((*s)[0] & 255) != 0xfe)
	      return (EOF);

	    *encoding = ENCODE_UTF16LE;
	    (*s)++;

	    return (mxml_string_getc(p, encoding));
	  }
	  else if ((ch & 0xe0) == 0xc0)
	  {
	   /*
	    * Two-byte value...
	    */

	    if (((*s)[0] & 0xc0) != 0x80)
              return (EOF);

	    ch = ((ch & 0x1f) << 6) | ((*s)[0] & 0x3f);

	    (*s)++;

	    if (ch < 0x80)
	    {
	      mxml_error("Invalid UTF-8 sequence for character 0x%04x.", ch);
	      return (EOF);
	    }

#if DEBUG > 1
            printf("mxml_string_getc: %c (0x%04x)\n", ch < ' ' ? '.' : ch, ch);
#endif /* DEBUG > 1 */

	    return (ch);
	  }
	  else if ((ch & 0xf0) == 0xe0)
	  {
	   /*
	    * Three-byte value...
	    */

	    if (((*s)[0] & 0xc0) != 0x80 ||
        	((*s)[1] & 0xc0) != 0x80)
              return (EOF);

	    ch = ((((ch & 0x0f) << 6) | ((*s)[0] & 0x3f)) << 6) | ((*s)[1] & 0x3f);

	    (*s) += 2;

	    if (ch < 0x800)
	    {
	      mxml_error("Invalid UTF-8 sequence for character 0x%04x.", ch);
	      return (EOF);
	    }

	   /*
	    * Ignore (strip) Byte Order Mark (BOM)...
	    */

	    if (ch == 0xfeff)
	      return (mxml_string_getc(p, encoding));

#if DEBUG > 1
            printf("mxml_string_getc: %c (0x%04x)\n", ch < ' ' ? '.' : ch, ch);
#endif /* DEBUG > 1 */

	    return (ch);
	  }
	  else if ((ch & 0xf8) == 0xf0)
	  {
	   /*
	    * Four-byte value...
	    */

	    if (((*s)[0] & 0xc0) != 0x80 ||
        	((*s)[1] & 0xc0) != 0x80 ||
        	((*s)[2] & 0xc0) != 0x80)
              return (EOF);

	    ch = ((((((ch & 0x07) << 6) | ((*s)[0] & 0x3f)) << 6) |
        	   ((*s)[1] & 0x3f)) << 6) | ((*s)[2] & 0x3f);

	    (*s) += 3;

	    if (ch < 0x10000)
	    {
	      mxml_error("Invalid UTF-8 sequence for character 0x%04x.", ch);
	      return (EOF);
	    }

#if DEBUG > 1
            printf("mxml_string_getc: %c (0x%04x)\n", ch < ' ' ? '.' : ch, ch);
#endif /* DEBUG > 1 */

	    return (ch);
	  }
	  else
	    return (EOF);

      case ENCODE_UTF16BE :
	 /*
          * Read UTF-16 big-endian char...
	  */

	  ch = (ch << 8) | ((*s)[0] & 255);
	  (*s) ++;

          if (mxml_bad_char(ch))
	  {
	    mxml_error("Bad control character 0x%02x not allowed by XML standard.",
        	       ch);
	    return (EOF);
	  }
          else if (ch >= 0xd800 && ch <= 0xdbff)
	  {
	   /*
	    * Multi-word UTF-16 char...
	    */

            int lch;			/* Lower word */

            if (!(*s)[0])
	      return (EOF);

            lch = (((*s)[0] & 255) << 8) | ((*s)[1] & 255);
	    (*s) += 2;

            if (lch < 0xdc00 || lch >= 0xdfff)
	      return (EOF);

            ch = (((ch & 0x3ff) << 10) | (lch & 0x3ff)) + 0x10000;
	  }

#if DEBUG > 1
          printf("mxml_string_getc: %c (0x%04x)\n", ch < ' ' ? '.' : ch, ch);
#endif /* DEBUG > 1 */

	  return (ch);

      case ENCODE_UTF16LE :
	 /*
          * Read UTF-16 little-endian char...
	  */

	  ch = ch | (((*s)[0] & 255) << 8);

	  if (!ch)
	  {
	    (*s) --;
	    return (EOF);
	  }

	  (*s) ++;

          if (mxml_bad_char(ch))
	  {
	    mxml_error("Bad control character 0x%02x not allowed by XML standard.",
        	       ch);
	    return (EOF);
	  }
          else if (ch >= 0xd800 && ch <= 0xdbff)
	  {
	   /*
	    * Multi-word UTF-16 char...
	    */

            int lch;			/* Lower word */

            if (!(*s)[1])
	      return (EOF);

            lch = (((*s)[1] & 255) << 8) | ((*s)[0] & 255);
	    (*s) += 2;

            if (lch < 0xdc00 || lch >= 0xdfff)
	      return (EOF);

            ch = (((ch & 0x3ff) << 10) | (lch & 0x3ff)) + 0x10000;
	  }

#if DEBUG > 1
          printf("mxml_string_getc: %c (0x%04x)\n", ch < ' ' ? '.' : ch, ch);
#endif /* DEBUG > 1 */

	  return (ch);
    }
  }

  return (EOF);
}

/*
 * 'mxml_string_putc()' - Write a character to a string.
 */

static int				/* O - 0 on success, -1 on failure */
mxml_string_putc(int  ch,		/* I - Character to write */
                 void *p)		/* I - Pointer to string pointers */
{
  char	**pp;				/* Pointer to string pointers */

  pp = (char **)p;

  if (pp[0] < pp[1])
    pp[0][0] = ch;

  pp[0] ++;

  return (0);
}

/*
 * 'mxml_write_name()' - Write a name string.
 */

static int				/* O - 0 on success, -1 on failure */
mxml_write_name(const char *s,		/* I - Name to write */
                void       *p,		/* I - Write pointer */
		int        (*putc_cb)(int, void *))
					/* I - Write callback */
{
  char		quote;			/* Quote character */
  const char	*name;			/* Entity name */

  if (*s == '\"' || *s == '\'')
  {
   /*
    * Write a quoted name string...
    */

    if ((*putc_cb)(*s, p) < 0)
      return (-1);

    quote = *s++;

    while (*s && *s != quote)
    {
      if ((name = mxmlEntityGetName(*s)) != NULL)
      {
	if ((*putc_cb)('&', p) < 0)
          return (-1);

        while (*name)
	{
	  if ((*putc_cb)(*name, p) < 0)
            return (-1);

          name ++;
	}

	if ((*putc_cb)(';', p) < 0)
          return (-1);
      }
      else if ((*putc_cb)(*s, p) < 0)
	return (-1);

      s ++;
    }

   /*
    * Write the end quote...
    */

    if ((*putc_cb)(quote, p) < 0)
      return (-1);
  }
  else
  {
   /*
    * Write a non-quoted name string...
    */

    while (*s)
    {
      if ((*putc_cb)(*s, p) < 0)
	return (-1);

      s ++;
    }
  }

  return (0);
}

/*
 * 'mxml_write_node()' - Save an XML node to a file.
 */

static int				/* O - Column or -1 on error */
mxml_write_node(mxml_node_t     *node,	/* I - Node to write */
                void            *p,	/* I - File to write to */
	        mxml_save_cb_t  cb,	/* I - Whitespace callback */
		int             col,	/* I - Current column */
		_mxml_putc_cb_t putc_cb,/* I - Output callback */
		_mxml_global_t  *global)/* I - Global data */
{
  mxml_node_t	*current,		/* Current node */
		*next;			/* Next node */
  int		i,			/* Looping var */
		width;			/* Width of attr + value */
  _mxml_attr_t	*attr;			/* Current attribute */
  char		s[255];			/* Temporary string */

 /*
  * Loop through this node and all of its children...
  */

  for (current = node; current; current = next)
  {
   /*
    * Print the node value...
    */

    switch (current->type)
    {
      case MXML_ELEMENT :
	  col = mxml_write_ws(current, p, cb, MXML_WS_BEFORE_OPEN, col, putc_cb);

	  if ((*putc_cb)('<', p) < 0)
	    return (-1);
	  if (current->value.element.name[0] == '?' ||
	      !strncmp(current->value.element.name, "!--", 3))
	  {
	   /*
	    * Comments and processing instructions do not use character
	    * entities.
	    */

	    const char	*ptr;		/* Pointer into name */

	    for (ptr = current->value.element.name; *ptr; ptr ++)
	      if ((*putc_cb)(*ptr, p) < 0)
		return (-1);
	  }
	  else if (!strncmp(current->value.element.name, "![CDATA[", 8))
	  {
	   /*
	    * CDATA elements do not use character entities, but also need the
	    * "]]" terminator added at the end.
	    */

	    const char	*ptr;		/* Pointer into name */

	    for (ptr = current->value.element.name; *ptr; ptr ++)
	      if ((*putc_cb)(*ptr, p) < 0)
		return (-1);

            if ((*putc_cb)(']', p) < 0)
              return (-1);
            if ((*putc_cb)(']', p) < 0)
              return (-1);
	  }
	  else if (mxml_write_name(current->value.element.name, p, putc_cb) < 0)
	    return (-1);

	  col += strlen(current->value.element.name) + 1;

	  for (i = current->value.element.num_attrs, attr = current->value.element.attrs;
	       i > 0;
	       i --, attr ++)
	  {
	    width = (int)strlen(attr->name);

	    if (attr->value)
	      width += strlen(attr->value) + 3;

	    if (global->wrap > 0 && (col + width) > global->wrap)
	    {
	      if ((*putc_cb)('\n', p) < 0)
		return (-1);

	      col = 0;
	    }
	    else
	    {
	      if ((*putc_cb)(' ', p) < 0)
		return (-1);

	      col ++;
	    }

	    if (mxml_write_name(attr->name, p, putc_cb) < 0)
	      return (-1);

	    if (attr->value)
	    {
	      if ((*putc_cb)('=', p) < 0)
		return (-1);
	      if ((*putc_cb)('\"', p) < 0)
		return (-1);
	      if (mxml_write_string(attr->value, p, putc_cb) < 0)
		return (-1);
	      if ((*putc_cb)('\"', p) < 0)
		return (-1);
	    }

	    col += width;
	  }

	  if (current->child)
	  {
	   /*
	    * Write children...
	    */

	    if ((*putc_cb)('>', p) < 0)
	      return (-1);
	    else
	      col ++;

	    col = mxml_write_ws(current, p, cb, MXML_WS_AFTER_OPEN, col, putc_cb);
	  }
	  else if (current->value.element.name[0] == '!' ||
		   current->value.element.name[0] == '?')
	  {
	   /*
	    * The ? and ! elements are special-cases...
	    */

	    if ((*putc_cb)('>', p) < 0)
	      return (-1);
	    else
	      col ++;

	    col = mxml_write_ws(current, p, cb, MXML_WS_AFTER_OPEN, col, putc_cb);
	  }
	  else
	  {
	    if ((*putc_cb)(' ', p) < 0)
	      return (-1);
	    if ((*putc_cb)('/', p) < 0)
	      return (-1);
	    if ((*putc_cb)('>', p) < 0)
	      return (-1);

	    col += 3;

	    col = mxml_write_ws(current, p, cb, MXML_WS_AFTER_OPEN, col, putc_cb);
	  }
	  break;

      case MXML_INTEGER :
	  if (current->prev)
	  {
	    if (global->wrap > 0 && col > global->wrap)
	    {
	      if ((*putc_cb)('\n', p) < 0)
		return (-1);

	      col = 0;
	    }
	    else if ((*putc_cb)(' ', p) < 0)
	      return (-1);
	    else
	      col ++;
	  }

	  snprintf(s, sizeof(s), "%d", current->value.integer);
	  if (mxml_write_string(s, p, putc_cb) < 0)
	    return (-1);

	  col += strlen(s);
	  break;

      case MXML_OPAQUE :
	  if (mxml_write_string(current->value.opaque, p, putc_cb) < 0)
	    return (-1);

	  col += strlen(current->value.opaque);
	  break;

      case MXML_REAL :
	  if (current->prev)
	  {
	    if (global->wrap > 0 && col > global->wrap)
	    {
	      if ((*putc_cb)('\n', p) < 0)
		return (-1);

	      col = 0;
	    }
	    else if ((*putc_cb)(' ', p) < 0)
	      return (-1);
	    else
	      col ++;
	  }

	  snprintf(s, sizeof(s), "%f", current->value.real);
	  if (mxml_write_string(s, p, putc_cb) < 0)
	    return (-1);

	  col += strlen(s);
	  break;

      case MXML_TEXT :
	  if (current->value.text.whitespace && col > 0)
	  {
	    if (global->wrap > 0 && col > global->wrap)
	    {
	      if ((*putc_cb)('\n', p) < 0)
		return (-1);

	      col = 0;
	    }
	    else if ((*putc_cb)(' ', p) < 0)
	      return (-1);
	    else
	      col ++;
	  }

	  if (mxml_write_string(current->value.text.string, p, putc_cb) < 0)
	    return (-1);

	  col += strlen(current->value.text.string);
	  break;

      case MXML_CUSTOM :
	  if (global->custom_save_cb)
	  {
	    char	*data;		/* Custom data string */
	    const char	*newline;	/* Last newline in string */

	    if ((data = (*global->custom_save_cb)(current)) == NULL)
	      return (-1);

	    if (mxml_write_string(data, p, putc_cb) < 0)
	      return (-1);

	    if ((newline = strrchr(data, '\n')) == NULL)
	      col += strlen(data);
	    else
	      col = (int)strlen(newline);

	    free(data);
	  }
	  break;

      default : /* Should never happen */
	  return (-1);
    }

   /*
    * Figure out the next node...
    */

    if ((next = current->child) == NULL)
    {
      if (current == node)
      {
       /*
        * Don't traverse to sibling node if we are at the "root" node...
        */

        next = NULL;
      }
      else
      {
       /*
        * Try the next sibling, and continue traversing upwards as needed...
        */

	while ((next = current->next) == NULL)
	{
	  if (current == node || !current->parent)
	    break;

	 /*
	  * The ? and ! elements are special-cases and have no end tags...
	  */

	  current = current->parent;

	  if (current->value.element.name[0] != '!' &&
	      current->value.element.name[0] != '?')
	  {
	    col = mxml_write_ws(current, p, cb, MXML_WS_BEFORE_CLOSE, col, putc_cb);

	    if ((*putc_cb)('<', p) < 0)
	      return (-1);
	    if ((*putc_cb)('/', p) < 0)
	      return (-1);
	    if (mxml_write_string(current->value.element.name, p, putc_cb) < 0)
	      return (-1);
	    if ((*putc_cb)('>', p) < 0)
	      return (-1);

	    col += strlen(current->value.element.name) + 3;

	    col = mxml_write_ws(current, p, cb, MXML_WS_AFTER_CLOSE, col, putc_cb);
	  }

	  if (current == node)
	    break;
	}
      }
    }
  }

  return (col);
}

/*
 * 'mxml_write_string()' - Write a string, escaping & and < as needed.
 */

static int				/* O - 0 on success, -1 on failure */
mxml_write_string(
    const char      *s,			/* I - String to write */
    void            *p,			/* I - Write pointer */
    _mxml_putc_cb_t putc_cb)		/* I - Write callback */
{
  const char	*name;			/* Entity name, if any */

  while (*s)
  {
    if ((name = mxmlEntityGetName(*s)) != NULL)
    {
      if ((*putc_cb)('&', p) < 0)
        return (-1);

      while (*name)
      {
	if ((*putc_cb)(*name, p) < 0)
          return (-1);
        name ++;
      }

      if ((*putc_cb)(';', p) < 0)
        return (-1);
    }
    else if ((*putc_cb)(*s, p) < 0)
      return (-1);

    s ++;
  }

  return (0);
}

/*
 * 'mxml_write_ws()' - Do whitespace callback...
 */

static int				/* O - New column */
mxml_write_ws(mxml_node_t     *node,	/* I - Current node */
              void            *p,	/* I - Write pointer */
              mxml_save_cb_t  cb,	/* I - Callback function */
	      int             ws,	/* I - Where value */
	      int             col,	/* I - Current column */
              _mxml_putc_cb_t putc_cb)	/* I - Write callback */
{
  const char	*s;			/* Whitespace string */

  if (cb && (s = (*cb)(node, ws)) != NULL)
  {
    while (*s)
    {
      if ((*putc_cb)(*s, p) < 0)
	return (-1);
      else if (*s == '\n')
	col = 0;
      else if (*s == '\t')
      {
	col += MXML_TAB;
	col = col - (col % MXML_TAB);
      }
      else
	col ++;

      s ++;
    }
  }

  return (col);
}

/*
 * mxml-get.c
 */
/*
 * 'mxmlGetCDATA()' - Get the value for a CDATA node.
 *
 * @code NULL@ is returned if the node is not a CDATA element.
 *
 * @since Mini-XML 2.7@
 */

const char *				/* O - CDATA value or @code NULL@ */
mxmlGetCDATA(mxml_node_t *node)		/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT ||
      strncmp(node->value.element.name, "![CDATA[", 8))
    return (NULL);

 /*
  * Return the text following the CDATA declaration...
  */

  return (node->value.element.name + 8);
}

/*
 * 'mxmlGetCustom()' - Get the value for a custom node.
 *
 * @code NULL@ is returned if the node (or its first child) is not a custom
 * value node.
 *
 * @since Mini-XML 2.7@
 */

const void *				/* O - Custom value or @code NULL@ */
mxmlGetCustom(mxml_node_t *node)	/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node)
    return (NULL);

 /*
  * Return the integer value...
  */

  if (node->type == MXML_CUSTOM)
    return (node->value.custom.data);
  else if (node->type == MXML_ELEMENT &&
           node->child &&
	   node->child->type == MXML_CUSTOM)
    return (node->child->value.custom.data);
  else
    return (NULL);
}

/*
 * 'mxmlGetElement()' - Get the name for an element node.
 *
 * @code NULL@ is returned if the node is not an element node.
 *
 * @since Mini-XML 2.7@
 */

const char *				/* O - Element name or @code NULL@ */
mxmlGetElement(mxml_node_t *node)	/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT)
    return (NULL);

 /*
  * Return the element name...
  */

  return (node->value.element.name);
}

/*
 * 'mxmlGetFirstChild()' - Get the first child of an element node.
 *
 * @code NULL@ is returned if the node is not an element node or if the node
 * has no children.
 *
 * @since Mini-XML 2.7@
 */

mxml_node_t *				/* O - First child or @code NULL@ */
mxmlGetFirstChild(mxml_node_t *node)	/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT)
    return (NULL);

 /*
  * Return the first child node...
  */

  return (node->child);
}

/*
 * 'mxmlGetInteger()' - Get the integer value from the specified node or its
 *                      first child.
 *
 * 0 is returned if the node (or its first child) is not an integer value node.
 *
 * @since Mini-XML 2.7@
 */

int					/* O - Integer value or 0 */
mxmlGetInteger(mxml_node_t *node)	/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node)
    return (0);

 /*
  * Return the integer value...
  */

  if (node->type == MXML_INTEGER)
    return (node->value.integer);
  else if (node->type == MXML_ELEMENT &&
           node->child &&
	   node->child->type == MXML_INTEGER)
    return (node->child->value.integer);
  else
    return (0);
}

/*
 * 'mxmlGetLastChild()' - Get the last child of an element node.
 *
 * @code NULL@ is returned if the node is not an element node or if the node
 * has no children.
 *
 * @since Mini-XML 2.7@
 */

mxml_node_t *				/* O - Last child or @code NULL@ */
mxmlGetLastChild(mxml_node_t *node)	/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT)
    return (NULL);

 /*
  * Return the node type...
  */

  return (node->last_child);
}

/*
 * 'mxmlGetNextSibling()' - Get the next node for the current parent.
 *
 * @code NULL@ is returned if this is the last child for the current parent.
 *
 * @since Mini-XML 2.7@
 */

mxml_node_t *
mxmlGetNextSibling(mxml_node_t *node)	/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node)
    return (NULL);

 /*
  * Return the node type...
  */

  return (node->next);
}

/*
 * 'mxmlGetOpaque()' - Get an opaque string value for a node or its first child.
 *
 * @code NULL@ is returned if the node (or its first child) is not an opaque
 * value node.
 *
 * @since Mini-XML 2.7@
 */

const char *				/* O - Opaque string or @code NULL@ */
mxmlGetOpaque(mxml_node_t *node)	/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node)
    return (NULL);

 /*
  * Return the integer value...
  */

  if (node->type == MXML_OPAQUE)
    return (node->value.opaque);
  else if (node->type == MXML_ELEMENT &&
           node->child &&
	   node->child->type == MXML_OPAQUE)
    return (node->child->value.opaque);
  else
    return (NULL);
}

/*
 * 'mxmlGetParent()' - Get the parent node.
 *
 * @code NULL@ is returned for a root node.
 *
 * @since Mini-XML 2.7@
 */

mxml_node_t *				/* O - Parent node or @code NULL@ */
mxmlGetParent(mxml_node_t *node)	/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node)
    return (NULL);

 /*
  * Return the node type...
  */

  return (node->parent);
}

/*
 * 'mxmlGetPrevSibling()' - Get the previous node for the current parent.
 *
 * @code NULL@ is returned if this is the first child for the current parent.
 *
 * @since Mini-XML 2.7@
 */

mxml_node_t *				/* O - Previous node or @code NULL@ */
mxmlGetPrevSibling(mxml_node_t *node)	/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node)
    return (NULL);

 /*
  * Return the node type...
  */

  return (node->prev);
}

/*
 * 'mxmlGetReal()' - Get the real value for a node or its first child.
 *
 * 0.0 is returned if the node (or its first child) is not a real value node.
 *
 * @since Mini-XML 2.7@
 */

double					/* O - Real value or 0.0 */
mxmlGetReal(mxml_node_t *node)		/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node)
    return (0.0);

 /*
  * Return the integer value...
  */

  if (node->type == MXML_REAL)
    return (node->value.real);
  else if (node->type == MXML_ELEMENT &&
           node->child &&
	   node->child->type == MXML_REAL)
    return (node->child->value.real);
  else
    return (0.0);
}

/*
 * 'mxmlGetText()' - Get the text value for a node or its first child.
 *
 * @code NULL@ is returned if the node (or its first child) is not a text node.
 * The "whitespace" argument can be @code NULL@.
 *
 * Note: Text nodes consist of whitespace-delimited words. You will only get
 * single words of text when reading an XML file with @code MXML_TEXT@ nodes.
 * If you want the entire string between elements in the XML file, you MUST read
 * the XML file with @code MXML_OPAQUE@ nodes and get the resulting strings
 * using the @link mxmlGetOpaque@ function instead.
 *
 * @since Mini-XML 2.7@
 */

const char *				/* O - Text string or @code NULL@ */
mxmlGetText(mxml_node_t *node,		/* I - Node to get */
            int         *whitespace)	/* O - 1 if string is preceded by whitespace, 0 otherwise */
{
 /*
  * Range check input...
  */

  if (!node)
  {
    if (whitespace)
      *whitespace = 0;

    return (NULL);
  }

 /*
  * Return the integer value...
  */

  if (node->type == MXML_TEXT)
  {
    if (whitespace)
      *whitespace = node->value.text.whitespace;

    return (node->value.text.string);
  }
  else if (node->type == MXML_ELEMENT &&
           node->child &&
	   node->child->type == MXML_TEXT)
  {
    if (whitespace)
      *whitespace = node->child->value.text.whitespace;

    return (node->child->value.text.string);
  }
  else
  {
    if (whitespace)
      *whitespace = 0;

    return (NULL);
  }
}

/*
 * 'mxmlGetType()' - Get the node type.
 *
 * @code MXML_IGNORE@ is returned if "node" is @code NULL@.
 *
 * @since Mini-XML 2.7@
 */

mxml_type_t				/* O - Type of node */
mxmlGetType(mxml_node_t *node)		/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node)
    return (MXML_IGNORE);

 /*
  * Return the node type...
  */

  return (node->type);
}

/*
 * 'mxmlGetUserData()' - Get the user data pointer for a node.
 *
 * @since Mini-XML 2.7@
 */

void *					/* O - User data pointer */
mxmlGetUserData(mxml_node_t *node)	/* I - Node to get */
{
 /*
  * Range check input...
  */

  if (!node)
    return (NULL);

 /*
  * Return the user data pointer...
  */

  return (node->user_data);
}

/*
 * mxml-index.c
 */
/*
 * Sort functions...
 */

static int	index_compare(mxml_index_t *ind, mxml_node_t *first,
		              mxml_node_t *second);
static int	index_find(mxml_index_t *ind, const char *element,
		           const char *value, mxml_node_t *node);
static void	index_sort(mxml_index_t *ind, int left, int right);

/*
 * 'mxmlIndexDelete()' - Delete an index.
 */

void
mxmlIndexDelete(mxml_index_t *ind)	/* I - Index to delete */
{
 /*
  * Range check input..
  */

  if (!ind)
    return;

 /*
  * Free memory...
  */

  free(ind->attr);
  free(ind->nodes);
  free(ind);
}

/*
 * 'mxmlIndexEnum()' - Return the next node in the index.
 *
 * You should call @link mxmlIndexReset@ prior to using this function to get
 * the first node in the index.  Nodes are returned in the sorted order of the
 * index.
 */

mxml_node_t *				/* O - Next node or @code NULL@ if there is none */
mxmlIndexEnum(mxml_index_t *ind)	/* I - Index to enumerate */
{
 /*
  * Range check input...
  */

  if (!ind)
    return (NULL);

 /*
  * Return the next node...
  */

  if (ind->cur_node < ind->num_nodes)
    return (ind->nodes[ind->cur_node ++]);
  else
    return (NULL);
}

/*
 * 'mxmlIndexFind()' - Find the next matching node.
 *
 * You should call @link mxmlIndexReset@ prior to using this function for
 * the first time with a particular set of "element" and "value"
 * strings. Passing @code NULL@ for both "element" and "value" is equivalent
 * to calling @link mxmlIndexEnum@.
 */

mxml_node_t *				/* O - Node or @code NULL@ if none found */
mxmlIndexFind(mxml_index_t *ind,	/* I - Index to search */
              const char   *element,	/* I - Element name to find, if any */
	      const char   *value)	/* I - Attribute value, if any */
{
  int		diff,			/* Difference between names */
		current,		/* Current entity in search */
		first,			/* First entity in search */
		last;			/* Last entity in search */

#ifdef DEBUG
  printf("mxmlIndexFind(ind=%p, element=\"%s\", value=\"%s\")\n",
         ind, element ? element : "(null)", value ? value : "(null)");
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!ind || (!ind->attr && value))
  {
#ifdef DEBUG
    puts("    returning NULL...");
    if (ind)
      printf("    ind->attr=\"%s\"\n", ind->attr ? ind->attr : "(null)");
#endif /* DEBUG */

    return (NULL);
  }

 /*
  * If both element and value are NULL, just enumerate the nodes in the
  * index...
  */

  if (!element && !value)
    return (mxmlIndexEnum(ind));

 /*
  * If there are no nodes in the index, return NULL...
  */

  if (!ind->num_nodes)
  {
#ifdef DEBUG
    puts("    returning NULL...");
    puts("    no nodes!");
#endif /* DEBUG */

    return (NULL);
  }

 /*
  * If cur_node == 0, then find the first matching node...
  */

  if (ind->cur_node == 0)
  {
   /*
    * Find the first node using a modified binary search algorithm...
    */

    first = 0;
    last  = ind->num_nodes - 1;

#ifdef DEBUG
    printf("    find first time, num_nodes=%d...\n", ind->num_nodes);
#endif /* DEBUG */

    while ((last - first) > 1)
    {
      current = (first + last) / 2;

#ifdef DEBUG
      printf("    first=%d, last=%d, current=%d\n", first, last, current);
#endif /* DEBUG */

      if ((diff = index_find(ind, element, value, ind->nodes[current])) == 0)
      {
       /*
        * Found a match, move back to find the first...
	*/

#ifdef DEBUG
        puts("    match!");
#endif /* DEBUG */

        while (current > 0 &&
	       !index_find(ind, element, value, ind->nodes[current - 1]))
	  current --;

#ifdef DEBUG
        printf("    returning first match=%d\n", current);
#endif /* DEBUG */

       /*
        * Return the first match and save the index to the next...
	*/

        ind->cur_node = current + 1;

	return (ind->nodes[current]);
      }
      else if (diff < 0)
	last = current;
      else
	first = current;

#ifdef DEBUG
      printf("    diff=%d\n", diff);
#endif /* DEBUG */
    }

   /*
    * If we get this far, then we found exactly 0 or 1 matches...
    */

    for (current = first; current <= last; current ++)
      if (!index_find(ind, element, value, ind->nodes[current]))
      {
       /*
	* Found exactly one (or possibly two) match...
	*/

#ifdef DEBUG
	printf("    returning only match %d...\n", current);
#endif /* DEBUG */

	ind->cur_node = current + 1;

	return (ind->nodes[current]);
      }

   /*
    * No matches...
    */

    ind->cur_node = ind->num_nodes;

#ifdef DEBUG
    puts("    returning NULL...");
#endif /* DEBUG */

    return (NULL);
  }
  else if (ind->cur_node < ind->num_nodes &&
           !index_find(ind, element, value, ind->nodes[ind->cur_node]))
  {
   /*
    * Return the next matching node...
    */

#ifdef DEBUG
    printf("    returning next match %d...\n", ind->cur_node);
#endif /* DEBUG */

    return (ind->nodes[ind->cur_node ++]);
  }

 /*
  * If we get this far, then we have no matches...
  */

  ind->cur_node = ind->num_nodes;

#ifdef DEBUG
  puts("    returning NULL...");
#endif /* DEBUG */

  return (NULL);
}

/*
 * 'mxmlIndexGetCount()' - Get the number of nodes in an index.
 *
 * @since Mini-XML 2.7@
 */

int					/* I - Number of nodes in index */
mxmlIndexGetCount(mxml_index_t *ind)	/* I - Index of nodes */
{
 /*
  * Range check input...
  */

  if (!ind)
    return (0);

 /*
  * Return the number of nodes in the index...
  */

  return (ind->num_nodes);
}

/*
 * 'mxmlIndexNew()' - Create a new index.
 *
 * The index will contain all nodes that contain the named element and/or
 * attribute.  If both "element" and "attr" are @code NULL@, then the index will
 * contain a sorted list of the elements in the node tree.  Nodes are
 * sorted by element name and optionally by attribute value if the "attr"
 * argument is not NULL.
 */

mxml_index_t *				/* O - New index */
mxmlIndexNew(mxml_node_t *node,		/* I - XML node tree */
             const char  *element,	/* I - Element to index or @code NULL@ for all */
             const char  *attr)		/* I - Attribute to index or @code NULL@ for none */
{
  mxml_index_t	*ind;			/* New index */
  mxml_node_t	*current,		/* Current node in index */
  		**temp;			/* Temporary node pointer array */

 /*
  * Range check input...
  */

#ifdef DEBUG
  printf("mxmlIndexNew(node=%p, element=\"%s\", attr=\"%s\")\n",
         node, element ? element : "(null)", attr ? attr : "(null)");
#endif /* DEBUG */

  if (!node)
    return (NULL);

 /*
  * Create a new index...
  */

  if ((ind = calloc(1, sizeof(mxml_index_t))) == NULL)
  {
    mxml_error("Unable to allocate memory for index.");
    return (NULL);
  }

  if (attr)
  {
    if ((ind->attr = strdup(attr)) == NULL)
    {
      mxml_error("Unable to allocate memory for index attribute.");
      free(ind);
      return (NULL);
    }
  }

  if (!element && !attr)
    current = node;
  else
    current = mxmlFindElement(node, node, element, attr, NULL, MXML_DESCEND);

  while (current)
  {
    if (ind->num_nodes >= ind->alloc_nodes)
    {
      if (!ind->alloc_nodes)
        temp = malloc(64 * sizeof(mxml_node_t *));
      else
        temp = realloc(ind->nodes, (ind->alloc_nodes + 64) * sizeof(mxml_node_t *));

      if (!temp)
      {
       /*
        * Unable to allocate memory for the index, so abort...
	*/

        mxml_error("Unable to allocate memory for index nodes.");
        mxmlIndexDelete(ind);
	return (NULL);
      }

      ind->nodes       = temp;
      ind->alloc_nodes += 64;
    }

    ind->nodes[ind->num_nodes ++] = current;

    current = mxmlFindElement(current, node, element, attr, NULL, MXML_DESCEND);
  }

 /*
  * Sort nodes based upon the search criteria...
  */

#ifdef DEBUG
  {
    int i;				/* Looping var */

    printf("%d node(s) in index.\n\n", ind->num_nodes);

    if (attr)
    {
      printf("Node      Address   Element         %s\n", attr);
      puts("--------  --------  --------------  ------------------------------");

      for (i = 0; i < ind->num_nodes; i ++)
	printf("%8d  %-8p  %-14.14s  %s\n", i, ind->nodes[i],
	       ind->nodes[i]->value.element.name,
	       mxmlElementGetAttr(ind->nodes[i], attr));
    }
    else
    {
      puts("Node      Address   Element");
      puts("--------  --------  --------------");

      for (i = 0; i < ind->num_nodes; i ++)
	printf("%8d  %-8p  %s\n", i, ind->nodes[i],
	       ind->nodes[i]->value.element.name);
    }

    putchar('\n');
  }
#endif /* DEBUG */

  if (ind->num_nodes > 1)
    index_sort(ind, 0, ind->num_nodes - 1);

#ifdef DEBUG
  {
    int i;				/* Looping var */

    puts("After sorting:\n");

    if (attr)
    {
      printf("Node      Address   Element         %s\n", attr);
      puts("--------  --------  --------------  ------------------------------");

      for (i = 0; i < ind->num_nodes; i ++)
	printf("%8d  %-8p  %-14.14s  %s\n", i, ind->nodes[i],
	       ind->nodes[i]->value.element.name,
	       mxmlElementGetAttr(ind->nodes[i], attr));
    }
    else
    {
      puts("Node      Address   Element");
      puts("--------  --------  --------------");

      for (i = 0; i < ind->num_nodes; i ++)
	printf("%8d  %-8p  %s\n", i, ind->nodes[i],
	       ind->nodes[i]->value.element.name);
    }

    putchar('\n');
  }
#endif /* DEBUG */

 /*
  * Return the new index...
  */

  return (ind);
}

/*
 * 'mxmlIndexReset()' - Reset the enumeration/find pointer in the index and
 *                      return the first node in the index.
 *
 * This function should be called prior to using @link mxmlIndexEnum@ or
 * @link mxmlIndexFind@ for the first time.
 */

mxml_node_t *				/* O - First node or @code NULL@ if there is none */
mxmlIndexReset(mxml_index_t *ind)	/* I - Index to reset */
{
#ifdef DEBUG
  printf("mxmlIndexReset(ind=%p)\n", ind);
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!ind)
    return (NULL);

 /*
  * Set the index to the first element...
  */

  ind->cur_node = 0;

 /*
  * Return the first node...
  */

  if (ind->num_nodes)
    return (ind->nodes[0]);
  else
    return (NULL);
}

/*
 * 'index_compare()' - Compare two nodes.
 */

static int				/* O - Result of comparison */
index_compare(mxml_index_t *ind,	/* I - Index */
              mxml_node_t  *first,	/* I - First node */
              mxml_node_t  *second)	/* I - Second node */
{
  int	diff;				/* Difference */

 /*
  * Check the element name...
  */

  if ((diff = strcmp(first->value.element.name,
                     second->value.element.name)) != 0)
    return (diff);

 /*
  * Check the attribute value...
  */

  if (ind->attr)
  {
    if ((diff = strcmp(mxmlElementGetAttr(first, ind->attr),
                       mxmlElementGetAttr(second, ind->attr))) != 0)
      return (diff);
  }

 /*
  * No difference, return 0...
  */

  return (0);
}

/*
 * 'index_find()' - Compare a node with index values.
 */

static int				/* O - Result of comparison */
index_find(mxml_index_t *ind,		/* I - Index */
           const char   *element,	/* I - Element name or @code NULL@ */
	   const char   *value,		/* I - Attribute value or @code NULL@ */
           mxml_node_t  *node)		/* I - Node */
{
  int	diff;				/* Difference */

 /*
  * Check the element name...
  */

  if (element)
  {
    if ((diff = strcmp(element, node->value.element.name)) != 0)
      return (diff);
  }

 /*
  * Check the attribute value...
  */

  if (value)
  {
    if ((diff = strcmp(value, mxmlElementGetAttr(node, ind->attr))) != 0)
      return (diff);
  }

 /*
  * No difference, return 0...
  */

  return (0);
}

/*
 * 'index_sort()' - Sort the nodes in the index...
 *
 * This function implements the classic quicksort algorithm...
 */

static void
index_sort(mxml_index_t *ind,		/* I - Index to sort */
           int          left,		/* I - Left node in partition */
	   int          right)		/* I - Right node in partition */
{
  mxml_node_t	*pivot,			/* Pivot node */
		*temp;			/* Swap node */
  int		templ,			/* Temporary left node */
		tempr;			/* Temporary right node */

 /*
  * Loop until we have sorted all the way to the right...
  */

  do
  {
   /*
    * Sort the pivot in the current partition...
    */

    pivot = ind->nodes[left];

    for (templ = left, tempr = right; templ < tempr;)
    {
     /*
      * Move left while left node <= pivot node...
      */

      while ((templ < right) &&
             index_compare(ind, ind->nodes[templ], pivot) <= 0)
	templ ++;

     /*
      * Move right while right node > pivot node...
      */

      while ((tempr > left) &&
             index_compare(ind, ind->nodes[tempr], pivot) > 0)
	tempr --;

     /*
      * Swap nodes if needed...
      */

      if (templ < tempr)
      {
	temp              = ind->nodes[templ];
	ind->nodes[templ] = ind->nodes[tempr];
	ind->nodes[tempr] = temp;
      }
    }

   /*
    * When we get here, the right (tempr) node is the new position for the
    * pivot node...
    */

    if (index_compare(ind, pivot, ind->nodes[tempr]) > 0)
    {
      ind->nodes[left]  = ind->nodes[tempr];
      ind->nodes[tempr] = pivot;
    }

   /*
    * Recursively sort the left partition as needed...
    */

    if (left < (tempr - 1))
      index_sort(ind, left, tempr - 1);
  }
  while (right > (left = tempr + 1));
}

/*
 * mxml-node.c
 */
/*
 * Local functions...
 */

static void		mxml_free(mxml_node_t *node);
static mxml_node_t	*mxml_new(mxml_node_t *parent, mxml_type_t type);

/*
 * 'mxmlAdd()' - Add a node to a tree.
 *
 * Adds the specified node to the parent.  If the child argument is not
 * @code NULL@, puts the new node before or after the specified child depending
 * on the value of the where argument.  If the child argument is @code NULL@,
 * puts the new node at the beginning of the child list (@code MXML_ADD_BEFORE@)
 * or at the end of the child list (@code MXML_ADD_AFTER@).  The constant
 * @code MXML_ADD_TO_PARENT@ can be used to specify a @code NULL@ child pointer.
 */

void
mxmlAdd(mxml_node_t *parent,		/* I - Parent node */
        int         where,		/* I - Where to add, @code MXML_ADD_BEFORE@ or @code MXML_ADD_AFTER@ */
        mxml_node_t *child,		/* I - Child node for where or @code MXML_ADD_TO_PARENT@ */
	mxml_node_t *node)		/* I - Node to add */
{
#ifdef DEBUG
  fprintf(stderr, "mxmlAdd(parent=%p, where=%d, child=%p, node=%p)\n", parent,
          where, child, node);
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!parent || !node)
    return;

#if DEBUG > 1
  fprintf(stderr, "    BEFORE: node->parent=%p\n", node->parent);
  if (parent)
  {
    fprintf(stderr, "    BEFORE: parent->child=%p\n", parent->child);
    fprintf(stderr, "    BEFORE: parent->last_child=%p\n", parent->last_child);
    fprintf(stderr, "    BEFORE: parent->prev=%p\n", parent->prev);
    fprintf(stderr, "    BEFORE: parent->next=%p\n", parent->next);
  }
#endif /* DEBUG > 1 */

 /*
  * Remove the node from any existing parent...
  */

  if (node->parent)
    mxmlRemove(node);

 /*
  * Reset pointers...
  */

  node->parent = parent;

  switch (where)
  {
    case MXML_ADD_BEFORE :
        if (!child || child == parent->child || child->parent != parent)
	{
	 /*
	  * Insert as first node under parent...
	  */

	  node->next = parent->child;

	  if (parent->child)
	    parent->child->prev = node;
	  else
	    parent->last_child = node;

	  parent->child = node;
	}
	else
	{
	 /*
	  * Insert node before this child...
	  */

	  node->next = child;
	  node->prev = child->prev;

	  if (child->prev)
	    child->prev->next = node;
	  else
	    parent->child = node;

	  child->prev = node;
	}
        break;

    case MXML_ADD_AFTER :
        if (!child || child == parent->last_child || child->parent != parent)
	{
	 /*
	  * Insert as last node under parent...
	  */

	  node->parent = parent;
	  node->prev   = parent->last_child;

	  if (parent->last_child)
	    parent->last_child->next = node;
	  else
	    parent->child = node;

	  parent->last_child = node;
        }
	else
	{
	 /*
	  * Insert node after this child...
	  */

	  node->prev = child;
	  node->next = child->next;

	  if (child->next)
	    child->next->prev = node;
	  else
	    parent->last_child = node;

	  child->next = node;
	}
        break;
  }

#if DEBUG > 1
  fprintf(stderr, "    AFTER: node->parent=%p\n", node->parent);
  if (parent)
  {
    fprintf(stderr, "    AFTER: parent->child=%p\n", parent->child);
    fprintf(stderr, "    AFTER: parent->last_child=%p\n", parent->last_child);
    fprintf(stderr, "    AFTER: parent->prev=%p\n", parent->prev);
    fprintf(stderr, "    AFTER: parent->next=%p\n", parent->next);
  }
#endif /* DEBUG > 1 */
}

/*
 * 'mxmlDelete()' - Delete a node and all of its children.
 *
 * If the specified node has a parent, this function first removes the
 * node from its parent using the @link mxmlRemove@ function.
 */

void
mxmlDelete(mxml_node_t *node)		/* I - Node to delete */
{
  mxml_node_t	*current,		/* Current node */
		*next;			/* Next node */

#ifdef DEBUG
  fprintf(stderr, "mxmlDelete(node=%p)\n", node);
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!node)
    return;

 /*
  * Remove the node from its parent, if any...
  */

  mxmlRemove(node);

 /*
  * Delete children...
  */

  for (current = node->child; current; current = next)
  {
   /*
    * Get the next node...
    */

    if ((next = current->child) != NULL)
    {
     /*
      * Free parent nodes after child nodes have been freed...
      */

      current->child = NULL;
      continue;
    }

    if ((next = current->next) == NULL)
    {
     /*
      * Next node is the parent, which we'll free as needed...
      */

      if ((next = current->parent) == node)
        next = NULL;
    }

   /*
    * Free child...
    */

    mxml_free(current);
  }

 /*
  * Then free the memory used by the parent node...
  */

  mxml_free(node);
}

/*
 * 'mxmlGetRefCount()' - Get the current reference (use) count for a node.
 *
 * The initial reference count of new nodes is 1. Use the @link mxmlRetain@
 * and @link mxmlRelease@ functions to increment and decrement a node's
 * reference count.
 *
 * @since Mini-XML 2.7@.
 */

int					/* O - Reference count */
mxmlGetRefCount(mxml_node_t *node)	/* I - Node */
{
 /*
  * Range check input...
  */

  if (!node)
    return (0);

 /*
  * Return the reference count...
  */

  return (node->ref_count);
}

/*
 * 'mxmlNewCDATA()' - Create a new CDATA node.
 *
 * The new CDATA node is added to the end of the specified parent's child
 * list.  The constant @code MXML_NO_PARENT@ can be used to specify that the new
 * CDATA node has no parent.  The data string must be nul-terminated and
 * is copied into the new node.  CDATA nodes currently use the
 * @code MXML_ELEMENT@ type.
 *
 * @since Mini-XML 2.3@
 */

mxml_node_t *				/* O - New node */
mxmlNewCDATA(mxml_node_t *parent,	/* I - Parent node or @code MXML_NO_PARENT@ */
	     const char  *data)		/* I - Data string */
{
  mxml_node_t	*node;			/* New node */

#ifdef DEBUG
  fprintf(stderr, "mxmlNewCDATA(parent=%p, data=\"%s\")\n",
          parent, data ? data : "(null)");
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!data)
    return (NULL);

 /*
  * Create the node and set the name value...
  */

  if ((node = mxml_new(parent, MXML_ELEMENT)) != NULL)
    node->value.element.name = _mxml_strdupf("![CDATA[%s", data);

  return (node);
}

/*
 * 'mxmlNewCustom()' - Create a new custom data node.
 *
 * The new custom node is added to the end of the specified parent's child
 * list. The constant @code MXML_NO_PARENT@ can be used to specify that the new
 * element node has no parent. @code NULL@ can be passed when the data in the
 * node is not dynamically allocated or is separately managed.
 *
 * @since Mini-XML 2.1@
 */

mxml_node_t *				/* O - New node */
mxmlNewCustom(
    mxml_node_t              *parent,	/* I - Parent node or @code MXML_NO_PARENT@ */
    void                     *data,	/* I - Pointer to data */
    mxml_custom_destroy_cb_t destroy)	/* I - Function to destroy data */
{
  mxml_node_t	*node;			/* New node */

#ifdef DEBUG
  fprintf(stderr, "mxmlNewCustom(parent=%p, data=%p, destroy=%p)\n", parent,
          data, destroy);
#endif /* DEBUG */

 /*
  * Create the node and set the value...
  */

  if ((node = mxml_new(parent, MXML_CUSTOM)) != NULL)
  {
    node->value.custom.data    = data;
    node->value.custom.destroy = destroy;
  }

  return (node);
}

/*
 * 'mxmlNewElement()' - Create a new element node.
 *
 * The new element node is added to the end of the specified parent's child
 * list. The constant @code MXML_NO_PARENT@ can be used to specify that the new
 * element node has no parent.
 */

mxml_node_t *				/* O - New node */
mxmlNewElement(mxml_node_t *parent,	/* I - Parent node or @code MXML_NO_PARENT@ */
               const char  *name)	/* I - Name of element */
{
  mxml_node_t	*node;			/* New node */

#ifdef DEBUG
  fprintf(stderr, "mxmlNewElement(parent=%p, name=\"%s\")\n", parent,
          name ? name : "(null)");
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!name)
    return (NULL);

 /*
  * Create the node and set the element name...
  */

  if ((node = mxml_new(parent, MXML_ELEMENT)) != NULL)
    node->value.element.name = strdup(name);

  return (node);
}

/*
 * 'mxmlNewInteger()' - Create a new integer node.
 *
 * The new integer node is added to the end of the specified parent's child
 * list. The constant @code MXML_NO_PARENT@ can be used to specify that the new
 * integer node has no parent.
 */

mxml_node_t *				/* O - New node */
mxmlNewInteger(mxml_node_t *parent,	/* I - Parent node or @code MXML_NO_PARENT@ */
               int         integer)	/* I - Integer value */
{
  mxml_node_t	*node;			/* New node */

#ifdef DEBUG
  fprintf(stderr, "mxmlNewInteger(parent=%p, integer=%d)\n", parent, integer);
#endif /* DEBUG */

 /*
  * Create the node and set the element name...
  */

  if ((node = mxml_new(parent, MXML_INTEGER)) != NULL)
    node->value.integer = integer;

  return (node);
}

/*
 * 'mxmlNewOpaque()' - Create a new opaque string.
 *
 * The new opaque string node is added to the end of the specified parent's
 * child list.  The constant @code MXML_NO_PARENT@ can be used to specify that
 * the new opaque string node has no parent.  The opaque string must be nul-
 * terminated and is copied into the new node.
 */

mxml_node_t *				/* O - New node */
mxmlNewOpaque(mxml_node_t *parent,	/* I - Parent node or @code MXML_NO_PARENT@ */
              const char  *opaque)	/* I - Opaque string */
{
  mxml_node_t	*node;			/* New node */

#ifdef DEBUG
  fprintf(stderr, "mxmlNewOpaque(parent=%p, opaque=\"%s\")\n", parent,
          opaque ? opaque : "(null)");
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!opaque)
    return (NULL);

 /*
  * Create the node and set the element name...
  */

  if ((node = mxml_new(parent, MXML_OPAQUE)) != NULL)
    node->value.opaque = strdup(opaque);

  return (node);
}

/*
 * 'mxmlNewOpaquef()' - Create a new formatted opaque string node.
 *
 * The new opaque string node is added to the end of the specified parent's
 * child list.  The constant @code MXML_NO_PARENT@ can be used to specify that
 * the new opaque string node has no parent.  The format string must be
 * nul-terminated and is formatted into the new node.
 */

mxml_node_t *				/* O - New node */
mxmlNewOpaquef(mxml_node_t *parent,	/* I - Parent node or @code MXML_NO_PARENT@ */
               const char  *format,	/* I - Printf-style format string */
	       ...)			/* I - Additional args as needed */
{
  mxml_node_t	*node;			/* New node */
  va_list	ap;			/* Pointer to arguments */

#ifdef DEBUG
  fprintf(stderr, "mxmlNewOpaquef(parent=%p, format=\"%s\", ...)\n", parent, format ? format : "(null)");
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!format)
    return (NULL);

 /*
  * Create the node and set the text value...
  */

  if ((node = mxml_new(parent, MXML_OPAQUE)) != NULL)
  {
    va_start(ap, format);

    node->value.opaque = _mxml_vstrdupf(format, ap);

    va_end(ap);
  }

  return (node);
}

/*
 * 'mxmlNewReal()' - Create a new real number node.
 *
 * The new real number node is added to the end of the specified parent's
 * child list.  The constant @code MXML_NO_PARENT@ can be used to specify that
 * the new real number node has no parent.
 */

mxml_node_t *				/* O - New node */
mxmlNewReal(mxml_node_t *parent,	/* I - Parent node or @code MXML_NO_PARENT@ */
            double      real)		/* I - Real number value */
{
  mxml_node_t	*node;			/* New node */

#ifdef DEBUG
  fprintf(stderr, "mxmlNewReal(parent=%p, real=%g)\n", parent, real);
#endif /* DEBUG */

 /*
  * Create the node and set the element name...
  */

  if ((node = mxml_new(parent, MXML_REAL)) != NULL)
    node->value.real = real;

  return (node);
}

/*
 * 'mxmlNewText()' - Create a new text fragment node.
 *
 * The new text node is added to the end of the specified parent's child
 * list.  The constant @code MXML_NO_PARENT@ can be used to specify that the new
 * text node has no parent.  The whitespace parameter is used to specify
 * whether leading whitespace is present before the node.  The text
 * string must be nul-terminated and is copied into the new node.
 */

mxml_node_t *				/* O - New node */
mxmlNewText(mxml_node_t *parent,	/* I - Parent node or @code MXML_NO_PARENT@ */
            int         whitespace,	/* I - 1 = leading whitespace, 0 = no whitespace */
	    const char  *string)	/* I - String */
{
  mxml_node_t	*node;			/* New node */

#ifdef DEBUG
  fprintf(stderr, "mxmlNewText(parent=%p, whitespace=%d, string=\"%s\")\n",
          parent, whitespace, string ? string : "(null)");
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!string)
    return (NULL);

 /*
  * Create the node and set the text value...
  */

  if ((node = mxml_new(parent, MXML_TEXT)) != NULL)
  {
    node->value.text.whitespace = whitespace;
    node->value.text.string     = strdup(string);
  }

  return (node);
}

/*
 * 'mxmlNewTextf()' - Create a new formatted text fragment node.
 *
 * The new text node is added to the end of the specified parent's child
 * list.  The constant @code MXML_NO_PARENT@ can be used to specify that the new
 * text node has no parent.  The whitespace parameter is used to specify
 * whether leading whitespace is present before the node.  The format
 * string must be nul-terminated and is formatted into the new node.
 */

mxml_node_t *				/* O - New node */
mxmlNewTextf(mxml_node_t *parent,	/* I - Parent node or @code MXML_NO_PARENT@ */
             int         whitespace,	/* I - 1 = leading whitespace, 0 = no whitespace */
	     const char  *format,	/* I - Printf-style format string */
	     ...)			/* I - Additional args as needed */
{
  mxml_node_t	*node;			/* New node */
  va_list	ap;			/* Pointer to arguments */

#ifdef DEBUG
  fprintf(stderr, "mxmlNewTextf(parent=%p, whitespace=%d, format=\"%s\", ...)\n",
          parent, whitespace, format ? format : "(null)");
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!format)
    return (NULL);

 /*
  * Create the node and set the text value...
  */

  if ((node = mxml_new(parent, MXML_TEXT)) != NULL)
  {
    va_start(ap, format);

    node->value.text.whitespace = whitespace;
    node->value.text.string     = _mxml_vstrdupf(format, ap);

    va_end(ap);
  }

  return (node);
}

/*
 * 'mxmlRemove()' - Remove a node from its parent.
 *
 * This function does not free memory used by the node - use @link mxmlDelete@
 * for that.  This function does nothing if the node has no parent.
 */

void
mxmlRemove(mxml_node_t *node)		/* I - Node to remove */
{
#ifdef DEBUG
  fprintf(stderr, "mxmlRemove(node=%p)\n", node);
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!node || !node->parent)
    return;

 /*
  * Remove from parent...
  */

#if DEBUG > 1
  fprintf(stderr, "    BEFORE: node->parent=%p\n", node->parent);
  if (node->parent)
  {
    fprintf(stderr, "    BEFORE: node->parent->child=%p\n", node->parent->child);
    fprintf(stderr, "    BEFORE: node->parent->last_child=%p\n", node->parent->last_child);
  }
  fprintf(stderr, "    BEFORE: node->child=%p\n", node->child);
  fprintf(stderr, "    BEFORE: node->last_child=%p\n", node->last_child);
  fprintf(stderr, "    BEFORE: node->prev=%p\n", node->prev);
  fprintf(stderr, "    BEFORE: node->next=%p\n", node->next);
#endif /* DEBUG > 1 */

  if (node->prev)
    node->prev->next = node->next;
  else
    node->parent->child = node->next;

  if (node->next)
    node->next->prev = node->prev;
  else
    node->parent->last_child = node->prev;

  node->parent = NULL;
  node->prev   = NULL;
  node->next   = NULL;

#if DEBUG > 1
  fprintf(stderr, "    AFTER: node->parent=%p\n", node->parent);
  if (node->parent)
  {
    fprintf(stderr, "    AFTER: node->parent->child=%p\n", node->parent->child);
    fprintf(stderr, "    AFTER: node->parent->last_child=%p\n", node->parent->last_child);
  }
  fprintf(stderr, "    AFTER: node->child=%p\n", node->child);
  fprintf(stderr, "    AFTER: node->last_child=%p\n", node->last_child);
  fprintf(stderr, "    AFTER: node->prev=%p\n", node->prev);
  fprintf(stderr, "    AFTER: node->next=%p\n", node->next);
#endif /* DEBUG > 1 */
}

/*
 * 'mxmlNewXML()' - Create a new XML document tree.
 *
 * The "version" argument specifies the version number to put in the
 * ?xml element node. If @code NULL@, version "1.0" is assumed.
 *
 * @since Mini-XML 2.3@
 */

mxml_node_t *				/* O - New ?xml node */
mxmlNewXML(const char *version)		/* I - Version number to use */
{
  char	element[1024];			/* Element text */

  snprintf(element, sizeof(element), "?xml version=\"%s\" encoding=\"utf-8\"?",
           version ? version : "1.0");

  return (mxmlNewElement(NULL, element));
}

/*
 * 'mxmlRelease()' - Release a node.
 *
 * When the reference count reaches zero, the node (and any children)
 * is deleted via @link mxmlDelete@.
 *
 * @since Mini-XML 2.3@
 */

int					/* O - New reference count */
mxmlRelease(mxml_node_t *node)		/* I - Node */
{
  if (node)
  {
    if ((-- node->ref_count) <= 0)
    {
      mxmlDelete(node);
      return (0);
    }
    else
      return (node->ref_count);
  }
  else
    return (-1);
}

/*
 * 'mxmlRetain()' - Retain a node.
 *
 * @since Mini-XML 2.3@
 */

int					/* O - New reference count */
mxmlRetain(mxml_node_t *node)		/* I - Node */
{
  if (node)
    return (++ node->ref_count);
  else
    return (-1);
}

/*
 * 'mxml_free()' - Free the memory used by a node.
 *
 * Note: Does not free child nodes, does not remove from parent.
 */

static void
mxml_free(mxml_node_t *node)		/* I - Node */
{
  int	i;				/* Looping var */

  switch (node->type)
  {
    case MXML_ELEMENT :
	free(node->value.element.name);

	if (node->value.element.num_attrs)
	{
	  for (i = 0; i < node->value.element.num_attrs; i ++)
	  {
	    free(node->value.element.attrs[i].name);
	    free(node->value.element.attrs[i].value);
	  }

          free(node->value.element.attrs);
	}
        break;
    case MXML_INTEGER :
       /* Nothing to do */
        break;
    case MXML_OPAQUE :
	free(node->value.opaque);
        break;
    case MXML_REAL :
       /* Nothing to do */
        break;
    case MXML_TEXT :
	free(node->value.text.string);
        break;
    case MXML_CUSTOM :
        if (node->value.custom.data &&
	    node->value.custom.destroy)
	  (*(node->value.custom.destroy))(node->value.custom.data);
	break;
    default :
        break;
  }

 /*
  * Free this node...
  */

  free(node);
}

/*
 * 'mxml_new()' - Create a new node.
 */

static mxml_node_t *			/* O - New node */
mxml_new(mxml_node_t *parent,		/* I - Parent node */
         mxml_type_t type)		/* I - Node type */
{
  mxml_node_t	*node;			/* New node */

#if DEBUG > 1
  fprintf(stderr, "mxml_new(parent=%p, type=%d)\n", parent, type);
#endif /* DEBUG > 1 */

 /*
  * Allocate memory for the node...
  */

  if ((node = calloc(1, sizeof(mxml_node_t))) == NULL)
  {
#if DEBUG > 1
    fputs("    returning NULL\n", stderr);
#endif /* DEBUG > 1 */

    return (NULL);
  }

#if DEBUG > 1
  fprintf(stderr, "    returning %p\n", node);
#endif /* DEBUG > 1 */

 /*
  * Set the node type...
  */

  node->type      = type;
  node->ref_count = 1;

 /*
  * Add to the parent if present...
  */

  if (parent)
    mxmlAdd(parent, MXML_ADD_AFTER, MXML_ADD_TO_PARENT, node);

 /*
  * Return the new node...
  */

  return (node);
}

/*
 * mxml-search.c
 */
/*
 * 'mxmlFindElement()' - Find the named element.
 *
 * The search is constrained by the name, attribute name, and value; any
 * @code NULL@ names or values are treated as wildcards, so different kinds of
 * searches can be implemented by looking for all elements of a given name
 * or all elements with a specific attribute. The descend argument determines
 * whether the search descends into child nodes; normally you will use
 * @code MXML_DESCEND_FIRST@ for the initial search and @code MXML_NO_DESCEND@
 * to find additional direct descendents of the node. The top node argument
 * constrains the search to a particular node's children.
 */

mxml_node_t *				/* O - Element node or @code NULL@ */
mxmlFindElement(mxml_node_t *node,	/* I - Current node */
                mxml_node_t *top,	/* I - Top node */
                const char  *element,	/* I - Element name or @code NULL@ for any */
		const char  *attr,	/* I - Attribute name, or @code NULL@ for none */
		const char  *value,	/* I - Attribute value, or @code NULL@ for any */
		int         descend)	/* I - Descend into tree - @code MXML_DESCEND@, @code MXML_NO_DESCEND@, or @code MXML_DESCEND_FIRST@ */
{
  const char	*temp;			/* Current attribute value */

 /*
  * Range check input...
  */

  if (!node || !top || (!attr && value))
    return (NULL);

 /*
  * Start with the next node...
  */

  node = mxmlWalkNext(node, top, descend);

 /*
  * Loop until we find a matching element...
  */

  while (node != NULL)
  {
   /*
    * See if this node matches...
    */

    if (node->type == MXML_ELEMENT &&
        node->value.element.name &&
	(!element || !strcmp(node->value.element.name, element)))
    {
     /*
      * See if we need to check for an attribute...
      */

      if (!attr)
        return (node);			/* No attribute search, return it... */

     /*
      * Check for the attribute...
      */

      if ((temp = mxmlElementGetAttr(node, attr)) != NULL)
      {
       /*
        * OK, we have the attribute, does it match?
	*/

	if (!value || !strcmp(value, temp))
	  return (node);		/* Yes, return it... */
      }
    }

   /*
    * No match, move on to the next node...
    */

    if (descend == MXML_DESCEND)
      node = mxmlWalkNext(node, top, MXML_DESCEND);
    else
      node = node->next;
  }

  return (NULL);
}

/*
 * 'mxmlFindPath()' - Find a node with the given path.
 *
 * The "path" is a slash-separated list of element names. The name "*" is
 * considered a wildcard for one or more levels of elements.  For example,
 * "foo/one/two", "bar/two/one", "*\/one", and so forth.
 *
 * The first child node of the found node is returned if the given node has
 * children and the first child is a value node.
 *
 * @since Mini-XML 2.7@
 */

mxml_node_t *				/* O - Found node or @code NULL@ */
mxmlFindPath(mxml_node_t *top,		/* I - Top node */
	     const char  *path)		/* I - Path to element */
{
  mxml_node_t	*node;			/* Current node */
  char		element[256];		/* Current element name */
  const char	*pathsep;		/* Separator in path */
  int		descend;		/* mxmlFindElement option */

 /*
  * Range check input...
  */

  if (!top || !path || !*path)
    return (NULL);

 /*
  * Search each element in the path...
  */

  node = top;
  while (*path)
  {
   /*
    * Handle wildcards...
    */

    if (!strncmp(path, "*/", 2))
    {
      path += 2;
      descend = MXML_DESCEND;
    }
    else
      descend = MXML_DESCEND_FIRST;

   /*
    * Get the next element in the path...
    */

    if ((pathsep = strchr(path, '/')) == NULL)
      pathsep = path + strlen(path);

    if (pathsep == path || (pathsep - path) >= (long int)sizeof(element))
      return (NULL);

    memcpy(element, path, pathsep - path);
    element[pathsep - path] = '\0';

    if (*pathsep)
      path = pathsep + 1;
    else
      path = pathsep;

   /*
    * Search for the element...
    */

    if ((node = mxmlFindElement(node, node, element, NULL, NULL,
                                descend)) == NULL)
      return (NULL);
  }

 /*
  * If we get this far, return the node or its first child...
  */

  if (node->child && node->child->type != MXML_ELEMENT)
    return (node->child);
  else
    return (node);
}

/*
 * 'mxmlWalkNext()' - Walk to the next logical node in the tree.
 *
 * The descend argument controls whether the first child is considered
 * to be the next node.  The top node argument constrains the walk to
 * the node's children.
 */

mxml_node_t *				/* O - Next node or @code NULL@ */
mxmlWalkNext(mxml_node_t *node,		/* I - Current node */
             mxml_node_t *top,		/* I - Top node */
             int         descend)	/* I - Descend into tree - @code MXML_DESCEND@, @code MXML_NO_DESCEND@, or @code MXML_DESCEND_FIRST@ */
{
  if (!node)
    return (NULL);
  else if (node->child && descend)
    return (node->child);
  else if (node == top)
    return (NULL);
  else if (node->next)
    return (node->next);
  else if (node->parent && node->parent != top)
  {
    node = node->parent;

    while (!node->next)
      if (node->parent == top || !node->parent)
        return (NULL);
      else
        node = node->parent;

    return (node->next);
  }
  else
    return (NULL);
}

/*
 * 'mxmlWalkPrev()' - Walk to the previous logical node in the tree.
 *
 * The descend argument controls whether the previous node's last child
 * is considered to be the previous node.  The top node argument constrains
 * the walk to the node's children.
 */

mxml_node_t *				/* O - Previous node or @code NULL@ */
mxmlWalkPrev(mxml_node_t *node,		/* I - Current node */
             mxml_node_t *top,		/* I - Top node */
             int         descend)	/* I - Descend into tree - @code MXML_DESCEND@, @code MXML_NO_DESCEND@, or @code MXML_DESCEND_FIRST@ */
{
  if (!node || node == top)
    return (NULL);
  else if (node->prev)
  {
    if (node->prev->last_child && descend)
    {
     /*
      * Find the last child under the previous node...
      */

      node = node->prev->last_child;

      while (node->last_child)
        node = node->last_child;

      return (node);
    }
    else
      return (node->prev);
  }
  else if (node->parent != top)
    return (node->parent);
  else
    return (NULL);
}

/*
 * mxml-set.c
 */
/*
 * 'mxmlSetCDATA()' - Set the element name of a CDATA node.
 *
 * The node is not changed if it (or its first child) is not a CDATA element node.
 *
 * @since Mini-XML 2.3@
 */

int					/* O - 0 on success, -1 on failure */
mxmlSetCDATA(mxml_node_t *node,		/* I - Node to set */
             const char  *data)		/* I - New data string */
{
  char	*s;				/* New element name */

 /*
  * Range check input...
  */

  if (node && node->type == MXML_ELEMENT &&
      strncmp(node->value.element.name, "![CDATA[", 8) &&
      node->child && node->child->type == MXML_ELEMENT &&
      !strncmp(node->child->value.element.name, "![CDATA[", 8))
    node = node->child;

  if (!node || node->type != MXML_ELEMENT ||
      strncmp(node->value.element.name, "![CDATA[", 8))
  {
    mxml_error("Wrong node type.");
    return (-1);
  }
  else if (!data)
  {
    mxml_error("NULL string not allowed.");
    return (-1);
  }

  if (data == (node->value.element.name + 8))
  {
   /*
    * Don't change the value...
    */

    return (0);
  }

 /*
  * Allocate the new value, free any old element value, and set the new value...
  */

  if ((s = _mxml_strdupf("![CDATA[%s", data)) == NULL)
  {
    mxml_error("Unable to allocate memory for CDATA.");
    return (-1);
  }

  free(node->value.element.name);
  node->value.element.name = s;

  return (0);
}

/*
 * 'mxmlSetCustom()' - Set the data and destructor of a custom data node.
 *
 * The node is not changed if it (or its first child) is not a custom node.
 *
 * @since Mini-XML 2.1@
 */

int					/* O - 0 on success, -1 on failure */
mxmlSetCustom(
    mxml_node_t              *node,	/* I - Node to set */
    void                     *data,	/* I - New data pointer */
    mxml_custom_destroy_cb_t destroy)	/* I - New destructor function */
{
 /*
  * Range check input...
  */

  if (node && node->type == MXML_ELEMENT &&
      node->child && node->child->type == MXML_CUSTOM)
    node = node->child;

  if (!node || node->type != MXML_CUSTOM)
  {
    mxml_error("Wrong node type.");
    return (-1);
  }

  if (data == node->value.custom.data)
  {
    node->value.custom.destroy = destroy;
    return (0);
  }

 /*
  * Free any old element value and set the new value...
  */

  if (node->value.custom.data && node->value.custom.destroy)
    (*(node->value.custom.destroy))(node->value.custom.data);

  node->value.custom.data    = data;
  node->value.custom.destroy = destroy;

  return (0);
}

/*
 * 'mxmlSetElement()' - Set the name of an element node.
 *
 * The node is not changed if it is not an element node.
 */

int					/* O - 0 on success, -1 on failure */
mxmlSetElement(mxml_node_t *node,	/* I - Node to set */
               const char  *name)	/* I - New name string */
{
  char *s;				/* New name string */

 /*
  * Range check input...
  */

  if (!node || node->type != MXML_ELEMENT)
  {
    mxml_error("Wrong node type.");
    return (-1);
  }
  else if (!name)
  {
    mxml_error("NULL string not allowed.");
    return (-1);
  }

  if (name == node->value.element.name)
    return (0);

 /*
  * Free any old element value and set the new value...
  */

  if ((s = strdup(name)) == NULL)
  {
    mxml_error("Unable to allocate memory for element name.");
    return (-1);
  }

  free(node->value.element.name);
  node->value.element.name = s;

  return (0);
}

/*
 * 'mxmlSetInteger()' - Set the value of an integer node.
 *
 * The node is not changed if it (or its first child) is not an integer node.
 */

int					/* O - 0 on success, -1 on failure */
mxmlSetInteger(mxml_node_t *node,	/* I - Node to set */
               int         integer)	/* I - Integer value */
{
 /*
  * Range check input...
  */

  if (node && node->type == MXML_ELEMENT &&
      node->child && node->child->type == MXML_INTEGER)
    node = node->child;

  if (!node || node->type != MXML_INTEGER)
  {
    mxml_error("Wrong node type.");
    return (-1);
  }

 /*
  * Set the new value and return...
  */

  node->value.integer = integer;

  return (0);
}

/*
 * 'mxmlSetOpaque()' - Set the value of an opaque node.
 *
 * The node is not changed if it (or its first child) is not an opaque node.
 */

int					/* O - 0 on success, -1 on failure */
mxmlSetOpaque(mxml_node_t *node,	/* I - Node to set */
              const char  *opaque)	/* I - Opaque string */
{
  char *s;				/* New opaque string */

 /*
  * Range check input...
  */

  if (node && node->type == MXML_ELEMENT &&
      node->child && node->child->type == MXML_OPAQUE)
    node = node->child;

  if (!node || node->type != MXML_OPAQUE)
  {
    mxml_error("Wrong node type.");
    return (-1);
  }
  else if (!opaque)
  {
    mxml_error("NULL string not allowed.");
    return (-1);
  }

  if (node->value.opaque == opaque)
    return (0);

 /*
  * Free any old opaque value and set the new value...
  */

  if ((s = strdup(opaque)) == NULL)
  {
    mxml_error("Unable to allocate memory for opaque string.");
    return (-1);
  }

  free(node->value.opaque);
  node->value.opaque = s;

  return (0);
}

/*
 * 'mxmlSetOpaquef()' - Set the value of an opaque string node to a formatted string.
 *
 * The node is not changed if it (or its first child) is not an opaque node.
 *
 * @since Mini-XML 2.11@
 */

int					/* O - 0 on success, -1 on failure */
mxmlSetOpaquef(mxml_node_t *node,	/* I - Node to set */
               const char  *format,	/* I - Printf-style format string */
	       ...)			/* I - Additional arguments as needed */
{
  va_list	ap;			/* Pointer to arguments */
  char		*s;			/* Temporary string */

 /*
  * Range check input...
  */

  if (node && node->type == MXML_ELEMENT &&
      node->child && node->child->type == MXML_OPAQUE)
    node = node->child;

  if (!node || node->type != MXML_OPAQUE)
  {
    mxml_error("Wrong node type.");
    return (-1);
  }
  else if (!format)
  {
    mxml_error("NULL string not allowed.");
    return (-1);
  }

 /*
  * Format the new string, free any old string value, and set the new value...
  */

  va_start(ap, format);
  s = _mxml_vstrdupf(format, ap);
  va_end(ap);

  if (!s)
  {
    mxml_error("Unable to allocate memory for opaque string.");
    return (-1);
  }

  free(node->value.opaque);
  node->value.opaque = s;

  return (0);
}

/*
 * 'mxmlSetReal()' - Set the value of a real number node.
 *
 * The node is not changed if it (or its first child) is not a real number node.
 */

int					/* O - 0 on success, -1 on failure */
mxmlSetReal(mxml_node_t *node,		/* I - Node to set */
            double      real)		/* I - Real number value */
{
 /*
  * Range check input...
  */

  if (node && node->type == MXML_ELEMENT &&
      node->child && node->child->type == MXML_REAL)
    node = node->child;

  if (!node || node->type != MXML_REAL)
  {
    mxml_error("Wrong node type.");
    return (-1);
  }

 /*
  * Set the new value and return...
  */

  node->value.real = real;

  return (0);
}

/*
 * 'mxmlSetText()' - Set the value of a text node.
 *
 * The node is not changed if it (or its first child) is not a text node.
 */

int					/* O - 0 on success, -1 on failure */
mxmlSetText(mxml_node_t *node,		/* I - Node to set */
            int         whitespace,	/* I - 1 = leading whitespace, 0 = no whitespace */
	    const char  *string)	/* I - String */
{
  char *s;				/* New string */

 /*
  * Range check input...
  */

  if (node && node->type == MXML_ELEMENT &&
      node->child && node->child->type == MXML_TEXT)
    node = node->child;

  if (!node || node->type != MXML_TEXT)
  {
    mxml_error("Wrong node type.");
    return (-1);
  }
  else if (!string)
  {
    mxml_error("NULL string not allowed.");
    return (-1);
  }

  if (string == node->value.text.string)
  {
    node->value.text.whitespace = whitespace;
    return (0);
  }

 /*
  * Free any old string value and set the new value...
  */

  if ((s = strdup(string)) == NULL)
  {
    mxml_error("Unable to allocate memory for text string.");
    return (-1);
  }

  free(node->value.text.string);

  node->value.text.whitespace = whitespace;
  node->value.text.string     = s;

  return (0);
}

/*
 * 'mxmlSetTextf()' - Set the value of a text node to a formatted string.
 *
 * The node is not changed if it (or its first child) is not a text node.
 */

int					/* O - 0 on success, -1 on failure */
mxmlSetTextf(mxml_node_t *node,		/* I - Node to set */
             int         whitespace,	/* I - 1 = leading whitespace, 0 = no whitespace */
             const char  *format,	/* I - Printf-style format string */
	     ...)			/* I - Additional arguments as needed */
{
  va_list	ap;			/* Pointer to arguments */
  char		*s;			/* Temporary string */

 /*
  * Range check input...
  */

  if (node && node->type == MXML_ELEMENT &&
      node->child && node->child->type == MXML_TEXT)
    node = node->child;

  if (!node || node->type != MXML_TEXT)
  {
    mxml_error("Wrong node type.");
    return (-1);
  }
  else if (!format)
  {
    mxml_error("NULL string not allowed.");
    return (-1);
  }

 /*
  * Free any old string value and set the new value...
  */

  va_start(ap, format);
  s = _mxml_vstrdupf(format, ap);
  va_end(ap);

  if (!s)
  {
    mxml_error("Unable to allocate memory for text string.");
    return (-1);
  }

  free(node->value.text.string);

  node->value.text.whitespace = whitespace;
  node->value.text.string     = s;

  return (0);
}

/*
 * 'mxmlSetUserData()' - Set the user data pointer for a node.
 *
 * @since Mini-XML 2.7@
 */

int					/* O - 0 on success, -1 on failure */
mxmlSetUserData(mxml_node_t *node,	/* I - Node to set */
                void        *data)	/* I - User data pointer */
{
 /*
  * Range check input...
  */

  if (!node)
    return (-1);

 /*
  * Set the user data pointer and return...
  */

  node->user_data = data;
  return (0);
}

/*
 * mxml-string.c
 */
/*
 * The va_copy macro is part of C99, but many compilers don't implement it.
 * Provide a "direct assignment" implmentation when va_copy isn't defined...
 */

#ifndef va_copy
#  ifdef __va_copy
#    define va_copy(dst,src) __va_copy(dst,src)
#  else
#    define va_copy(dst,src) memcpy(&dst, &src, sizeof(va_list))
#  endif /* __va_copy */
#endif /* va_copy */

#ifndef HAVE_SNPRINTF
/*
 * '_mxml_snprintf()' - Format a string.
 */

int					/* O - Number of bytes formatted */
_mxml_snprintf(char       *buffer,	/* I - Output buffer */
               size_t     bufsize,	/* I - Size of output buffer */
	       const char *format,	/* I - Printf-style format string */
	       ...)			/* I - Additional arguments as needed */
{
  va_list	ap;			/* Argument list */
  int		bytes;			/* Number of bytes formatted */

  va_start(ap, format);
  bytes = vsnprintf(buffer, bufsize, format, ap);
  va_end(ap);

  return (bytes);
}
#endif /* !HAVE_SNPRINTF */

/*
 * '_mxml_strdup()' - Duplicate a string.
 */

#ifndef HAVE_STRDUP
char *					/* O - New string pointer */
_mxml_strdup(const char *s)		/* I - String to duplicate */
{
  char	*t;				/* New string pointer */

  if (s == NULL)
    return (NULL);

  if ((t = malloc(strlen(s) + 1)) == NULL)
    return (NULL);

  return (strcpy(t, s));
}
#endif /* !HAVE_STRDUP */

/*
 * '_mxml_strdupf()' - Format and duplicate a string.
 */

char *					/* O - New string pointer */
_mxml_strdupf(const char *format,	/* I - Printf-style format string */
              ...)			/* I - Additional arguments as needed */
{
  va_list	ap;			/* Pointer to additional arguments */
  char		*s;			/* Pointer to formatted string */

 /*
  * Get a pointer to the additional arguments, format the string,
  * and return it...
  */

  va_start(ap, format);
#ifdef HAVE_VASPRINTF
  if (vasprintf(&s, format, ap) < 0)
    s = NULL;
#else
  s = _mxml_vstrdupf(format, ap);
#endif /* HAVE_VASPRINTF */
  va_end(ap);

  return (s);
}

#ifndef HAVE_STRLCAT
/*
 * '_mxml_strlcat()' - Safely concatenate a string.
 */

size_t					/* O - Number of bytes copied */
_mxml_strlcat(char       *dst,		/* I - Destination buffer */
              const char *src,		/* I - Source string */
              size_t     dstsize)	/* I - Size of destination buffer */
{
  size_t	srclen;			/* Length of source string */
  size_t	dstlen;			/* Length of destination string */

 /*
  * Figure out how much room is left...
  */

  dstlen = strlen(dst);

  if (dstsize <= (dstlen + 1))
    return (dstlen);		        /* No room, return immediately... */

  dstsize -= dstlen + 1;

 /*
  * Figure out how much room is needed...
  */

  srclen = strlen(src);

 /*
  * Copy the appropriate amount...
  */

  if (srclen > dstsize)
    srclen = dstsize;

  memmove(dst + dstlen, src, srclen);
  dst[dstlen + srclen] = '\0';

  return (dstlen + srclen);
}
#endif /* !HAVE_STRLCAT */

#ifndef HAVE_STRLCPY
/*
 * '_mxml_strlcpy()' - Safely copy a string.
 */

size_t					/* O - Number of bytes copied */
_mxml_strlcpy(char       *dst,		/* I - Destination buffer */
              const char *src,		/* I - Source string */
              size_t     dstsize)	/* I - Size of destination buffer */
{
  size_t        srclen;                 /* Length of source string */

 /*
  * Figure out how much room is needed...
  */

  dstsize --;

  srclen = strlen(src);

 /*
  * Copy the appropriate amount...
  */

  if (srclen > dstsize)
    srclen = dstsize;

  memmove(dst, src, srclen);
  dst[srclen] = '\0';

  return (srclen);
}
#endif /* !HAVE_STRLCPY */

#ifndef HAVE_VSNPRINTF
/*
 * '_mxml_vsnprintf()' - Format a string into a fixed size buffer.
 */

int					/* O - Number of bytes formatted */
_mxml_vsnprintf(char       *buffer,	/* O - Output buffer */
                size_t     bufsize,	/* O - Size of output buffer */
		const char *format,	/* I - Printf-style format string */
 		va_list    ap)		/* I - Pointer to additional arguments */
{
  char		*bufptr,		/* Pointer to position in buffer */
		*bufend,		/* Pointer to end of buffer */
		sign,			/* Sign of format width */
#ifdef HAVE_LONG_LONG_INT
		size,			/* Size character (h, l, L) */
#endif
		type;			/* Format type character */
  int		width,			/* Width of field */
		prec;			/* Number of characters of precision */
  char		tformat[100],		/* Temporary format string for sprintf() */
		*tptr,			/* Pointer into temporary format */
		temp[1024];		/* Buffer for formatted numbers */
  char		*s;			/* Pointer to string */
  int		slen;			/* Length of string */
  int		bytes;			/* Total number of bytes needed */

 /*
  * Loop through the format string, formatting as needed...
  */

  bufptr = buffer;
  bufend = buffer + bufsize - 1;
  bytes  = 0;

  while (*format)
  {
    if (*format == '%')
    {
      tptr = tformat;
      *tptr++ = *format++;

      if (*format == '%')
      {
        if (bufptr && bufptr < bufend)
          *bufptr++ = *format;
        bytes ++;
        format ++;
	continue;
      }
      else if (strchr(" -+#\'", *format))
      {
        *tptr++ = *format;
        sign = *format++;
      }
      else
        sign = 0;

      if (*format == '*')
      {
       /*
        * Get width from argument...
	*/

	format ++;
	width = va_arg(ap, int);

	snprintf(tptr, sizeof(tformat) - (tptr - tformat), "%d", width);
	tptr += strlen(tptr);
      }
      else
      {
	width = 0;

	while (isdigit(*format & 255))
	{
	  if (tptr < (tformat + sizeof(tformat) - 1))
	    *tptr++ = *format;

	  width = width * 10 + *format++ - '0';
	}
      }

      if (*format == '.')
      {
	if (tptr < (tformat + sizeof(tformat) - 1))
	  *tptr++ = *format;

        format ++;

        if (*format == '*')
	{
         /*
	  * Get precision from argument...
	  */

	  format ++;
	  prec = va_arg(ap, int);

	  snprintf(tptr, sizeof(tformat) - (tptr - tformat), "%d", prec);
	  tptr += strlen(tptr);
	}
	else
	{
	  prec = 0;

	  while (isdigit(*format & 255))
	  {
	    if (tptr < (tformat + sizeof(tformat) - 1))
	      *tptr++ = *format;

	    prec = prec * 10 + *format++ - '0';
	  }
	}
      }
      else
        prec = -1;

      if (*format == 'l' && format[1] == 'l')
      {
#ifdef HAVE_LONG_LONG_INT
        size = 'L';
#endif

		if (tptr < (tformat + sizeof(tformat) - 2))
		{
		  *tptr++ = 'l';
		  *tptr++ = 'l';
		}

		format += 2;
      }
      else if (*format == 'h' || *format == 'l' || *format == 'L')
      {
		if (tptr < (tformat + sizeof(tformat) - 1))
		  *tptr++ = *format;
#ifdef HAVE_LONG_LONG_INT
		size = *format++;
#endif
      }

      if (!*format)
        break;

      if (tptr < (tformat + sizeof(tformat) - 1))
        *tptr++ = *format;

      type  = *format++;
      *tptr = '\0';

      switch (type)
      {
	case 'E' : /* Floating point formats */
	case 'G' :
	case 'e' :
	case 'f' :
	case 'g' :
	    if ((width + 2) > (int)sizeof(temp))
	      break;

	    sprintf(temp, tformat, va_arg(ap, double));

            bytes += strlen(temp);

            if (bufptr)
	    {
	      if ((bufptr + strlen(temp)) > bufend)
	      {
		strncpy(bufptr, temp, (size_t)(bufend - bufptr));
		bufptr = bufend;
	      }
	      else
	      {
		strcpy(bufptr, temp);
		bufptr += strlen(temp);
	      }
	    }
	    break;

        case 'B' : /* Integer formats */
	case 'X' :
	case 'b' :
        case 'd' :
	case 'i' :
	case 'o' :
	case 'u' :
	case 'x' :
	    if ((width + 2) > (int)sizeof(temp))
	      break;

#ifdef HAVE_LONG_LONG_INT
	    if (size == 'L')
	      sprintf(temp, tformat, va_arg(ap, long long));
	    else
#endif /* HAVE_LONG_LONG_INT */
	    sprintf(temp, tformat, va_arg(ap, int));

            bytes += strlen(temp);

	    if (bufptr)
	    {
	      if ((bufptr + strlen(temp)) > bufend)
	      {
		strncpy(bufptr, temp, (size_t)(bufend - bufptr));
		bufptr = bufend;
	      }
	      else
	      {
		strcpy(bufptr, temp);
		bufptr += strlen(temp);
	      }
	    }
	    break;

	case 'p' : /* Pointer value */
	    if ((width + 2) > (int)sizeof(temp))
	      break;

	    sprintf(temp, tformat, va_arg(ap, void *));

            bytes += strlen(temp);

	    if (bufptr)
	    {
	      if ((bufptr + strlen(temp)) > bufend)
	      {
		strncpy(bufptr, temp, (size_t)(bufend - bufptr));
		bufptr = bufend;
	      }
	      else
	      {
		strcpy(bufptr, temp);
		bufptr += strlen(temp);
	      }
	    }
	    break;

        case 'c' : /* Character or character array */
	    bytes += width;

	    if (bufptr)
	    {
	      if (width <= 1)
	        *bufptr++ = va_arg(ap, int);
	      else
	      {
		if ((bufptr + width) > bufend)
		  width = bufend - bufptr;

		memcpy(bufptr, va_arg(ap, char *), (size_t)width);
		bufptr += width;
	      }
	    }
	    break;

	case 's' : /* String */
	    if ((s = va_arg(ap, char *)) == NULL)
	      s = "(null)";

	    slen = strlen(s);
	    if (slen > width && prec != width)
	      width = slen;

            bytes += width;

	    if (bufptr)
	    {
	      if ((bufptr + width) > bufend)
	        width = bufend - bufptr;

              if (slen > width)
	        slen = width;

	      if (sign == '-')
	      {
		strncpy(bufptr, s, (size_t)slen);
		memset(bufptr + slen, ' ', (size_t)(width - slen));
	      }
	      else
	      {
		memset(bufptr, ' ', (size_t)(width - slen));
		strncpy(bufptr + width - slen, s, (size_t)slen);
	      }

	      bufptr += width;
	    }
	    break;

	case 'n' : /* Output number of chars so far */
	    *(va_arg(ap, int *)) = bytes;
	    break;
      }
    }
    else
    {
      bytes ++;

      if (bufptr && bufptr < bufend)
        *bufptr++ = *format;

      format ++;
    }
  }

 /*
  * Nul-terminate the string and return the number of characters needed.
  */

  *bufptr = '\0';

  return (bytes);
}
#endif /* !HAVE_VSNPRINTF */

/*
 * '_mxml_vstrdupf()' - Format and duplicate a string.
 */

char *					/* O - New string pointer */
_mxml_vstrdupf(const char *format,	/* I - Printf-style format string */
               va_list    ap)		/* I - Pointer to additional arguments */
{
#ifdef HAVE_VASPRINTF
  char		*s;			/* String */

  if (vasprintf(&s, format, ap) < 0)
    s = NULL;

  return (s);

#else
  int		bytes;			/* Number of bytes required */
  char		*buffer;		/* String buffer */
#  ifndef _WIN32
  char		temp[256];		/* Small buffer for first vsnprintf */
#  endif /* !_WIN32 */

 /*
  * First format with a tiny buffer; this will tell us how many bytes are
  * needed...
  */

#  ifdef _WIN32
  bytes = _vscprintf(format, ap);

#  else
  va_list	apcopy;			/* Copy of argument list */

  va_copy(apcopy, ap);
  if ((bytes = vsnprintf(temp, sizeof(temp), format, apcopy)) < (int)sizeof(temp))
  {
   /*
    * Hey, the formatted string fits in the tiny buffer, so just dup that...
    */

    return (strdup(temp));
  }
#  endif /* _WIN32 */

 /*
  * Allocate memory for the whole thing and reformat to the new buffer...
  */

  if ((buffer = calloc(1, bytes + 1)) != NULL)
    vsnprintf(buffer, bytes + 1, format, ap);

 /*
  * Return the new string...
  */

  return (buffer);
#endif /* HAVE_VASPRINTF */
}

#ifdef __cplusplus
}
#endif
#endif /* __libaroma_xml_c__ */
