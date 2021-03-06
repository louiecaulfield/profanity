/*
 * server_events.c
 *
 * Copyright (C) 2012, 2013 James Booth <boothj5@gmail.com>
 *
 * This file is part of Profanity.
 *
 * Profanity is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Profanity is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Profanity.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <string.h>

#include "chat_session.h"
#include "log.h"
#include "muc.h"
#include "config/preferences.h"
#include "roster_list.h"
#include "ui/ui.h"
#ifdef HAVE_LIBOTR
#include "otr.h"
#endif

void
handle_error_message(const char *from, const char *err_msg)
{
    ui_handle_error_message(from, err_msg);

    if (g_strcmp0(err_msg, "conflict") == 0) {
        // remove the room from muc
        Jid *room_jid = jid_create(from);
        if (!muc_get_roster_received(room_jid->barejid)) {
            muc_leave_room(room_jid->barejid);
        }
        jid_destroy(room_jid);
    }
}

void
handle_login_account_success(char *account_name)
{
    ProfAccount *account = accounts_get_account(account_name);
#ifdef HAVE_LIBOTR
    otr_on_connect(account);
#endif
    resource_presence_t resource_presence = accounts_get_login_presence(account->name);
    contact_presence_t contact_presence = contact_presence_from_resource_presence(resource_presence);
    cons_show_login_success(account);
    title_bar_set_presence(contact_presence);
    log_info("%s logged in successfully", account->jid);
    ui_current_page_off();
    status_bar_print_message(account->jid);
    status_bar_refresh();

    accounts_free_account(account);
}

void
handle_lost_connection(void)
{
    cons_show_error("Lost connection.");
    roster_clear();
    muc_clear_invites();
    chat_sessions_clear();
    ui_disconnected();
    ui_current_page_off();
}

void
handle_failed_login(void)
{
    cons_show_error("Login failed.");
    log_info("Login failed");
    ui_current_page_off();
}

void
handle_software_version_result(const char * const jid, const char * const  presence,
    const char * const name, const char * const version, const char * const os)
{
    cons_show_software_version(jid, presence, name, version, os);
    ui_current_page_off();
}

void
handle_disco_info(const char *from, GSList *identities, GSList *features)
{
    cons_show_disco_info(from, identities, features);
    ui_current_page_off();
}

void
handle_room_list(GSList *rooms, const char *conference_node)
{
    cons_show_room_list(rooms, conference_node);
    ui_current_page_off();
}

void
handle_disco_items(GSList *items, const char *jid)
{
    cons_show_disco_items(items, jid);
    ui_current_page_off();
}

void
handle_room_invite(jabber_invite_t invite_type,
    const char * const invitor, const char * const room,
    const char * const reason)
{
    Jid *room_jid = jid_create(room);
    if (!muc_room_is_active(room_jid) && !muc_invites_include(room)) {
        cons_show_room_invite(invitor, room, reason);
        muc_add_invite(room);
        ui_current_page_off();
    }
    jid_destroy(room_jid);
}

void
handle_room_broadcast(const char *const room_jid,
    const char * const message)
{
    ui_room_broadcast(room_jid, message);
    ui_current_page_off();
}

void
handle_room_subject(const char * const room_jid, const char * const subject)
{
    ui_room_subject(room_jid, subject);
    ui_current_page_off();
}

void
handle_room_history(const char * const room_jid, const char * const nick,
    GTimeVal tv_stamp, const char * const message)
{
    ui_room_history(room_jid, nick, tv_stamp, message);
    ui_current_page_off();
}

void
handle_room_message(const char * const room_jid, const char * const nick,
    const char * const message)
{
    ui_room_message(room_jid, nick, message);
    ui_current_page_off();

    if (prefs_get_boolean(PREF_GRLOG)) {
        Jid *jid = jid_create(jabber_get_fulljid());
        groupchat_log_chat(jid->barejid, room_jid, nick, message);
        jid_destroy(jid);
    }
}

void
handle_duck_result(const char * const result)
{
    ui_duck_result(result);
    ui_current_page_off();
}

void
handle_incoming_message(char *from, char *message, gboolean priv)
{
#ifdef HAVE_LIBOTR
    gboolean was_decrypted = FALSE;
    char *newmessage;
    if (!priv) {
        newmessage = otr_decrypt_message(from, message, &was_decrypted);

        // internal OTR message
        if (newmessage == NULL) {
            return;
        }
    } else {
        newmessage = message;
    }

    ui_incoming_msg(from, newmessage, NULL, priv);
    ui_current_page_off();

    if (prefs_get_boolean(PREF_CHLOG) && !priv) {
        Jid *from_jid = jid_create(from);
        const char *jid = jabber_get_fulljid();
        Jid *jidp = jid_create(jid);

        if (!was_decrypted || (strcmp(prefs_get_string(PREF_OTR_LOG), "on") == 0)) {
            chat_log_chat(jidp->barejid, from_jid->barejid, newmessage, PROF_IN_LOG, NULL);
        } else if (strcmp(prefs_get_string(PREF_OTR_LOG), "redact") == 0) {
            chat_log_chat(jidp->barejid, from_jid->barejid, "[redacted]", PROF_IN_LOG, NULL);
        }

        jid_destroy(jidp);
        jid_destroy(from_jid);
    }

    if (!priv)
        otr_free_message(newmessage);
#else
    ui_incoming_msg(from, message, NULL, priv);
    ui_current_page_off();

    if (prefs_get_boolean(PREF_CHLOG) && !priv) {
        Jid *from_jid = jid_create(from);
        const char *jid = jabber_get_fulljid();
        Jid *jidp = jid_create(jid);
        chat_log_chat(jidp->barejid, from_jid->barejid, message, PROF_IN_LOG, NULL);
        jid_destroy(jidp);
        jid_destroy(from_jid);
    }
#endif
}

void
handle_delayed_message(char *from, char *message, GTimeVal tv_stamp,
    gboolean priv)
{
    ui_incoming_msg(from, message, &tv_stamp, priv);
    ui_current_page_off();

    if (prefs_get_boolean(PREF_CHLOG) && !priv) {
        Jid *from_jid = jid_create(from);
        const char *jid = jabber_get_fulljid();
        Jid *jidp = jid_create(jid);
        chat_log_chat(jidp->barejid, from_jid->barejid, message, PROF_IN_LOG, &tv_stamp);
        jid_destroy(jidp);
        jid_destroy(from_jid);
    }
}

void
handle_typing(char *from)
{
    ui_contact_typing(from);
    ui_current_page_off();
}

void
handle_gone(const char * const from)
{
    ui_recipient_gone(from);
    ui_current_page_off();
}

void
handle_subscription(const char *from, jabber_subscr_t type)
{
    switch (type) {
    case PRESENCE_SUBSCRIBE:
        /* TODO: auto-subscribe if needed */
        cons_show("Received authorization request from %s", from);
        log_info("Received authorization request from %s", from);
        ui_print_system_msg_from_recipient(from, "Authorization request, type '/sub allow' to accept or '/sub deny' to reject");
        ui_current_page_off();
        if (prefs_get_boolean(PREF_NOTIFY_SUB)) {
            notify_subscription(from);
        }
        break;
    case PRESENCE_SUBSCRIBED:
        cons_show("Subscription received from %s", from);
        log_info("Subscription received from %s", from);
        ui_print_system_msg_from_recipient(from, "Subscribed");
        ui_current_page_off();
        break;
    case PRESENCE_UNSUBSCRIBED:
        cons_show("%s deleted subscription", from);
        log_info("%s deleted subscription", from);
        ui_print_system_msg_from_recipient(from, "Unsubscribed");
        ui_current_page_off();
        break;
    default:
        /* unknown type */
        break;
    }
}

void
handle_contact_offline(char *contact, char *resource, char *status)
{
    gboolean updated = roster_contact_offline(contact, resource, status);

    if (resource != NULL && updated && prefs_get_boolean(PREF_STATUSES)) {
        Jid *jid = jid_create_from_bare_and_resource(contact, resource);
        PContact result = roster_get_contact(contact);
        if (p_contact_subscription(result) != NULL) {
            if (strcmp(p_contact_subscription(result), "none") != 0) {
                ui_contact_offline(jid->fulljid, "offline", status);
                ui_current_page_off();
            }
        }
        jid_destroy(jid);
    }
}

void
handle_contact_online(char *contact, Resource *resource,
    GDateTime *last_activity)
{
    gboolean updated = roster_update_presence(contact, resource, last_activity);

    if (updated && prefs_get_boolean(PREF_STATUSES)) {
        PContact result = roster_get_contact(contact);
        if (p_contact_subscription(result) != NULL) {
            if (strcmp(p_contact_subscription(result), "none") != 0) {
                const char *show = string_from_resource_presence(resource->presence);
                ui_contact_online(contact, resource->name, show, resource->status, last_activity);
                ui_current_page_off();
            }
        }
    }
}

void
handle_leave_room(const char * const room)
{
    muc_leave_room(room);
}

void
handle_room_nick_change(const char * const room,
    const char * const nick)
{
    ui_room_nick_change(room, nick);
    ui_current_page_off();
}

void
handle_room_roster_complete(const char * const room)
{
    muc_set_roster_received(room);
    GList *roster = muc_get_roster(room);
    ui_room_roster(room, roster, NULL);
    ui_current_page_off();
}

void
handle_room_member_presence(const char * const room,
    const char * const nick, const char * const show,
    const char * const status, const char * const caps_str)
{
    gboolean updated = muc_add_to_roster(room, nick, show, status, caps_str);

    if (updated) {
        ui_room_member_presence(room, nick, show, status);
        ui_current_page_off();
    }
}

void
handle_room_member_online(const char * const room, const char * const nick,
    const char * const show, const char * const status,
    const char * const caps_str)
{
    muc_add_to_roster(room, nick, show, status, caps_str);
    ui_room_member_online(room, nick, show, status);
    ui_current_page_off();
}

void
handle_room_member_offline(const char * const room, const char * const nick,
    const char * const show, const char * const status)
{
    muc_remove_from_roster(room, nick);
    ui_room_member_offline(room, nick);
    ui_current_page_off();
}

void
handle_room_member_nick_change(const char * const room,
    const char * const old_nick, const char * const nick)
{
    ui_room_member_nick_change(room, old_nick, nick);
    ui_current_page_off();
}

void
handle_group_add(const char * const contact,
    const char * const group)
{
    ui_group_added(contact, group);
    ui_current_page_off();
}

void
handle_group_remove(const char * const contact,
    const char * const group)
{
    ui_group_removed(contact, group);
    ui_current_page_off();
}

void
handle_roster_remove(const char * const barejid)
{
    ui_roster_remove(barejid);
    ui_current_page_off();
}

void
handle_roster_add(const char * const barejid, const char * const name)
{
    ui_roster_add(barejid, name);
    ui_current_page_off();
}
