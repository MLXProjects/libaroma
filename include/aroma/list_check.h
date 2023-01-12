/********************************************************************[libaroma]*
 * Copyright (C) 2011-2015 Ahmad Amarullah (http://amarullz.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *______________________________________________________________________________
 *
 * Filename    : listitem_check.h
 * Description : list item check
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 07/03/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_aroma_h__
  #error "Include <aroma.h> instead."
#endif
#ifndef __libaroma_listitem_check_h__
#define __libaroma_listitem_check_h__

/* check signature */
#define LIBAROMA_LISTITEM_CHECK_INDENT_NOICON      0x0100
#define LIBAROMA_LISTITEM_CHECK_SMALL_ICON         0x0200
#define LIBAROMA_LISTITEM_CHECK_SHARED_ICON        0x0400
#define LIBAROMA_LISTITEM_CHECK_FREE_ICON          0x0800
#define LIBAROMA_LISTITEM_CHECK_SWITCH             0x1000
#define LIBAROMA_LISTITEM_CHECK_LEFT_CONTROL       0x2000
#define LIBAROMA_LISTITEM_CHECK_OPTION             0x4000
#define LIBAROMA_LISTITEM_CHECK_HAS_SUBMENU        0x8000

#define LIBAROMA_LISTITEM_CB_UNSELECTED           0x0
#define LIBAROMA_LISTITEM_CB_SELECTED             0x1
#define LIBAROMA_LISTITEM_CB_FREE                 0x2

/* Onchange cb */
typedef byte (*LIBAROMA_LISTITEM_CB)(
  LIBAROMA_CONTROLP ctl,
  LIBAROMA_CTL_LIST_ITEMP item,
  int id,
  byte checked,
  voidp data,
  byte state
);
/* group structure */
typedef struct {
	int index;
	int list_index;
	LIBAROMA_CONTROLP list;
	LIBAROMA_STACKP items;
} LIBAROMA_LISTITEM_CHECK_GROUP, * LIBAROMA_LISTITEM_CHECK_GROUPP;

/* Set callback */
byte libaroma_listitem_check_set_cb(
    LIBAROMA_CONTROLP ctl,
    LIBAROMA_CTL_LIST_ITEMP item,
    LIBAROMA_LISTITEM_CB cb,
    voidp data);

#define libaroma_listitem_get_selected(list) \
		libaroma_listitem_get_selected_from(list, 0)
#define libaroma_listitem_get_selected_index(list) \
		libaroma_listitem_get_selected_index_from(list, 0)

/*
 * Function		: libaroma_listitem_get_selected_index_from
 * Return Value: int
 * Descriptions: get selected item index, start searching for items at start_index
 */
int libaroma_listitem_get_selected_index_from(
	LIBAROMA_CONTROLP ctl,
	int start_index
);

/*
 * Function		: libaroma_listitem_get_selected_from
 * Return Value: LIBAROMA_CTL_LIST_ITEMP
 * Descriptions: get selected item, start searching for items at start_index
 */
LIBAROMA_CTL_LIST_ITEMP libaroma_listitem_get_selected_from(
	LIBAROMA_CONTROLP ctl,
	int start_index
);

/*
 * Function		: libaroma_listitem_set_selected
 * Return Value: byte
 * Descriptions: set item as selected
 */
byte libaroma_listitem_set_selected(
  LIBAROMA_CONTROLP ctl,
  LIBAROMA_CTL_LIST_ITEMP item,
  byte selected);

/*
 * Function		: libaroma_listitem_isoption
 * Return Value: byte
 * Descriptions: check if item is option checkbox
 */
byte libaroma_listitem_isoption(
	LIBAROMA_CTL_LIST_ITEMP item
);

/*
 * Function		: libaroma_listitem_ischecked
 * Return Value: byte
 * Descriptions: check if checkbox is checked
 */
byte libaroma_listitem_ischecked(
	LIBAROMA_CTL_LIST_ITEMP item
);

/*
 * Function    : libaroma_listitem_check
 * Return Value: LIBAROMA_CTL_LIST_ITEMP
 * Descriptions: create check item
 */
LIBAROMA_CTL_LIST_ITEMP libaroma_listitem_check(
    LIBAROMA_CONTROLP ctl,
    int id,
    byte selected,
    const char * main_text,
    const char * extra_text,
    LIBAROMA_CANVASP icon,
    word flags,
    int at_index);

/************GROUPS MANAGEMENT******************/

#define libaroma_listitem_check_create_group(list) \
		libaroma_listitem_check_create_named_group(list, NULL)
/*
 * Function		: libaroma_listitem_check_create_named_group
 * Return Value: LIBAROMA_LISTITEM_CHECK_GROUPP
 * Descriptions: create listitem group with optional caption
 */
LIBAROMA_LISTITEM_CHECK_GROUPP libaroma_listitem_check_create_named_group(
	LIBAROMA_CONTROLP list,
	char *name
);

/*
 * Function		: libaroma_listitem_check_add_to_group
 * Return Value: void
 * Descriptions: add item to listitem group
 */
void libaroma_listitem_check_add_to_group(
	LIBAROMA_LISTITEM_CHECK_GROUPP group,
	LIBAROMA_CTL_LIST_ITEMP item
);

/*
 * Function		: libaroma_listitem_check_remove_from_group
 * Return Value: void
 * Descriptions: remove item from group
 */
void libaroma_listitem_check_remove_from_group(
	LIBAROMA_LISTITEM_CHECK_GROUPP group,
	LIBAROMA_CTL_LIST_ITEMP item
);

/*
 * Function		: libaroma_listitem_check_get_group_at
 * Return Value: LIBAROMA_LISTITEM_CHECK_GROUPP
 * Descriptions: get listitem group from list at index
 */
LIBAROMA_LISTITEM_CHECK_GROUPP libaroma_listitem_check_get_group_at(
	LIBAROMA_CONTROLP list,
	int index
);

/*
 * Function		: libaroma_listitem_check_get_ingroup_index
 * Return Value: int
 * Descriptions: get item index inside group
 */
int libaroma_listitem_check_get_ingroup_index(
	LIBAROMA_LISTITEM_CHECK_GROUPP group,
	LIBAROMA_CTL_LIST_ITEMP item
);

/*
 * Function		: libaroma_listitem_check_get_ingroup_item
 * Return Value: LIBAROMA_CTL_LIST_ITEMP
 * Descriptions: get item at index inside group
 */
LIBAROMA_CTL_LIST_ITEMP libaroma_listitem_check_get_ingroup_item(
	LIBAROMA_LISTITEM_CHECK_GROUPP group,
	int index
);

/*
 * Function		: libaroma_listitem_check_find_group
 * Return Value: LIBAROMA_LISTITEM_CHECK_GROUPP
 * Descriptions: get group for item at list
 */
LIBAROMA_LISTITEM_CHECK_GROUPP libaroma_listitem_check_find_group(
	LIBAROMA_CONTROLP list,
	LIBAROMA_CTL_LIST_ITEMP item
);

/*
 * Function		: libaroma_listitem_check_get_group_selected
 * Return Value: LIBAROMA_CTL_LIST_ITEMP
 * Descriptions: get selected item at group
 */
LIBAROMA_CTL_LIST_ITEMP libaroma_listitem_check_get_group_selected(
	LIBAROMA_LISTITEM_CHECK_GROUPP group
);

/*
 * Function		: libaroma_listitem_check_get_selected_group
 * Return Value: LIBAROMA_LISTITEM_CHECK_GROUPP
 * Descriptions: get last touched group
 */
LIBAROMA_LISTITEM_CHECK_GROUPP libaroma_listitem_check_get_selected_group(
	LIBAROMA_CONTROLP list
);

/*
 * Function		: libaroma_listitem_check_get_group_selected_index
 * Return Value: int
 * Descriptions: get selected item index at group
 */
int libaroma_listitem_check_get_group_selected_index(
	LIBAROMA_LISTITEM_CHECK_GROUPP group
);



#endif /* __libaroma_listitem_check_h__ */
