#ifndef COMMON_MOCKS_H
#define COMMON_MOCKS_H

#include "xmpp/xmpp.h"

void mock_connection_status(jabber_conn_status_t status);
void mock_connection_account_name(char *name);
void mock_connection_presence_message(char *message);
void expect_room_list_request(char *conf_server);

void mock_jabber_connect_with_details(void);
void jabber_connect_with_username_password_expect_and_return(char *jid,
    char *password, jabber_conn_status_t result);
void jabber_connect_with_altdomain_expect_and_return(char *altdomain,
    jabber_conn_status_t result);
void jabber_connect_with_details_return(jabber_conn_status_t result);

void mock_jabber_connect_with_account(void);
void jabber_connect_with_account_expect_and_return(ProfAccount *account,
    jabber_conn_status_t result);
void jabber_connect_with_account_return(jabber_conn_status_t result);

void mock_presence_update(void);
void presence_update_expect(resource_presence_t presence, char *msg, int idle);

#endif
