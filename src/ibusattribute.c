/* vim:set et sts=4: */
/* IBus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <glib.h>
#include "ibusattribute.h"
#include "ibusmessage.h"

/**
 * SECTION: IBusAttribute
 * @short_description: Attribute
 * @include: ibus.h
 *
 * Entry point for using IBus functionality.
 *
 **/

static gboolean          ibus_attr_list_serialize    (IBusAttrList       *attr_list,
                                                      IBusMessageIter    *iter);
static IBusAttrList     *ibus_attr_list_deserialize  (IBusMessageIter    *iter);
static gboolean          ibus_attribute_serialize    (IBusAttribute      *attr,
                                                      IBusMessageIter    *iter);
static IBusAttribute    *ibus_attribute_deserialize  (IBusMessageIter    *iter);

GType
ibus_attribute_get_type ()
{
    static GType type = 0;
    if (type == 0) {
        type = g_boxed_type_register_static ("IBusAttribute",
                    (GBoxedCopyFunc)ibus_attribute_copy,
                    (GBoxedFreeFunc)ibus_attribute_free);

        ibus_message_register_type (type,
                                    (IBusSerializeFunc) ibus_attribute_serialize,
                                    (IBusDeserializeFunc) ibus_attribute_deserialize);
    }
    return type;
}

IBusAttribute *
ibus_attribute_new (guint type, guint value, guint start_index, guint end_index)
{
    g_return_val_if_fail (
        type == IBUS_ATTR_TYPE_UNDERLINE  ||
        type == IBUS_ATTR_TYPE_FOREGROUND ||
        type == IBUS_ATTR_TYPE_BACKGROUND, NULL);

    IBusAttribute *attr = g_slice_new (IBusAttribute);

    attr->type = type;
    attr->value = value;
    attr->start_index = start_index;
    attr->end_index = end_index;
    return attr;
}

IBusAttribute *
ibus_attribute_copy (IBusAttribute * attr)
{
    g_assert (attr != NULL);
    return ibus_attribute_new (attr->type, attr->value, attr->start_index, attr->end_index);
}

void
ibus_attribute_free (IBusAttribute *attr)
{
    g_assert (attr != NULL);
    g_slice_free (IBusAttribute, attr);
}

IBusAttribute *
ibus_attr_underline_new (guint underline_type, guint start_index, guint end_index)
{
    g_return_val_if_fail (
        underline_type == IBUS_ATTR_UNDERLINE_NONE   ||
        underline_type == IBUS_ATTR_UNDERLINE_SINGLE ||
        underline_type == IBUS_ATTR_UNDERLINE_DOUBLE ||
        underline_type == IBUS_ATTR_UNDERLINE_LOW, NULL);

    return ibus_attribute_new (IBUS_ATTR_TYPE_UNDERLINE, underline_type, start_index, end_index);
}

IBusAttribute *
ibus_attr_foreground_new (guint color, guint start_index, guint end_index)
{
    return ibus_attribute_new (IBUS_ATTR_TYPE_FOREGROUND, color, start_index, end_index);
}

IBusAttribute *
ibus_attr_background_new (guint color, guint start_index, guint end_index)
{
    return ibus_attribute_new (IBUS_ATTR_TYPE_BACKGROUND, color, start_index, end_index);
}

GType
ibus_attr_list_get_type ()
{
    static GType type = 0;
    if (type == 0) {
        type = g_boxed_type_register_static ("IBusAttrList",
                    (GBoxedCopyFunc)ibus_attr_list_copy,
                    (GBoxedFreeFunc)ibus_attr_list_unref);
        
        ibus_message_register_type (type,
                                    (IBusSerializeFunc) ibus_attr_list_serialize,
                                    (IBusDeserializeFunc) ibus_attr_list_deserialize);
    }
    return type;
}

IBusAttrList *
ibus_attr_list_new ()
{
    IBusAttrList *attr_list = g_slice_new(IBusAttrList);
    attr_list->refcount = 1;
    attr_list->attributes = g_array_new (TRUE, TRUE, sizeof (IBusAttribute *));
    return attr_list;
}

IBusAttrList *
ibus_attr_list_copy (IBusAttrList *attr_list)
{
    IBusAttrList *new_list = ibus_attr_list_new ();
    guint i;
    for (i = 0;; i++) {
        IBusAttribute *attr = ibus_attr_list_get (attr_list, i);
        if (attr == NULL) {
            break;
        }
        ibus_attr_list_append (new_list, ibus_attribute_copy (attr));
    }
    return new_list;
}

IBusAttrList *
ibus_attr_list_ref (IBusAttrList *attr_list)
{
    if (attr_list == NULL) {
        return NULL;
    }
    attr_list->refcount ++;
    return attr_list;
}

void
ibus_attr_list_unref (IBusAttrList *attr_list)
{
    if (attr_list == NULL) {
        return;
    }

    attr_list->refcount --;
    if (attr_list->refcount <= 0) {
        guint i;
        for (i = 0; i < attr_list->attributes->len; i++) {
            IBusAttribute *attr = g_array_index (attr_list->attributes,
                                        IBusAttribute *, i);
            if (attr != NULL) {
                ibus_attribute_free (attr);
            }
        }
        g_array_free (attr_list->attributes, TRUE);
        g_slice_free (IBusAttrList, attr_list);
    }
}

void
ibus_attr_list_append (IBusAttrList *attr_list, IBusAttribute *attr)
{
    g_return_if_fail (attr_list != NULL);
    g_return_if_fail (attr != NULL);
    g_array_append_val (attr_list->attributes, attr);
}

IBusAttribute *
ibus_attr_list_get (IBusAttrList *attr_list, guint index)
{
    g_return_val_if_fail (attr_list != NULL, NULL);
    IBusAttribute *attr = NULL;
    if (index < attr_list->attributes->len) {
        attr = g_array_index (attr_list->attributes, IBusAttribute *, index);
    }
    return attr;
}

IBusAttribute *
ibus_attribute_deserialize (IBusMessageIter *iter)
{
    DBusMessageIter sub_iter;
    gboolean retval;

    guint type, value, start_index, end_index;

    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_STRUCT, &sub_iter);
    g_assert (retval);

    ibus_message_iter_get (&sub_iter, G_TYPE_UINT, &type);
    ibus_message_iter_get (&sub_iter, G_TYPE_UINT, &value);
    ibus_message_iter_get (&sub_iter, G_TYPE_UINT, &start_index);
    ibus_message_iter_get (&sub_iter, G_TYPE_UINT, &end_index);

    return ibus_attribute_new (type, value, start_index, end_index);
}

IBusAttrList *
ibus_attr_list_deserialize (IBusMessageIter *iter)
{
    gint type;
    DBusMessageIter sub_iter;
    IBusAttrList *attr_list;
    gboolean retval;

    retval = ibus_message_iter_recurse (iter, IBUS_TYPE_ARRAY, &sub_iter);
    g_assert (retval);
    
    attr_list = ibus_attr_list_new ();
    
    while (ibus_message_iter_get_arg_type (&sub_iter) != G_TYPE_INVALID) {
        IBusAttribute *attr;
        
        ibus_message_iter_get (&sub_iter, IBUS_TYPE_ATTRIBUTE, &attr);

        if (attr == NULL)
            break;
        ibus_attr_list_append (attr_list, attr);
    }

    return attr_list;
}

gboolean
ibus_attribute_serialize (IBusAttribute   *attr,
                         IBusMessageIter *iter)
{
    g_assert (attr != NULL);
    g_assert (iter != NULL);

    DBusMessageIter sub_iter;

    ibus_message_iter_open_container (iter, IBUS_TYPE_STRUCT, NULL, &sub_iter);
    ibus_message_iter_append (&sub_iter, G_TYPE_UINT, &attr->type);
    ibus_message_iter_append (&sub_iter, G_TYPE_UINT, &attr->value);
    ibus_message_iter_append (&sub_iter, G_TYPE_UINT, &attr->start_index);
    ibus_message_iter_append (&sub_iter, G_TYPE_UINT, &attr->end_index);
    ibus_message_iter_close_container (iter, &sub_iter);

    return TRUE;
}


gboolean
ibus_attr_list_serialize (IBusAttrList    *attr_list,
                         IBusMessageIter *iter)
{
    g_assert (attr_list != NULL);
    g_assert (iter != NULL);

    gint i;
    DBusMessageIter sub_iter;

    ibus_message_iter_open_container(iter, IBUS_TYPE_ARRAY, "(uuuu)", &sub_iter);
    for (i = 0; ;i++) {
        IBusAttribute *attr;
        attr = ibus_attr_list_get (attr_list, i);
        if (attr == NULL)
            break;
        ibus_message_iter_append (&sub_iter, IBUS_TYPE_ATTRIBUTE, &attr);
    }
    ibus_message_iter_close_container (iter, &sub_iter);
    
    return TRUE;
}

