#ifndef MOCK_UI_H
#define MICK_UI_H

#include <glib.h>
#include <setjmp.h>
#include <cmocka.h>

void stub_cons_show(void);

void mock_cons_show(void);
void expect_cons_show(char *output);
void expect_cons_show_calls(int n);

void mock_cons_show_error(void);
void expect_cons_show_error(char *output);

void mock_cons_show_account(void);
void expect_cons_show_account(ProfAccount *account);

void mock_cons_show_account_list(void);
void expect_cons_show_account_list(gchar **accounts);

void stub_ui_ask_password(void);
void mock_ui_ask_password(void);
void mock_ui_ask_password_returns(char *password);

#endif
