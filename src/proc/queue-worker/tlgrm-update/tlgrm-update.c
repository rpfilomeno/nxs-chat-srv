#include <nxs-core/nxs-core.h>

// clang-format off

/* Project core include */
#include <nxs-chat-srv-core.h>
#include <nxs-chat-srv-meta.h>
#include <nxs-chat-srv-collections.h>
#include <nxs-chat-srv-units.h>

#include <proc/queue-worker/tlgrm-update/tlgrm-update.h>
#include <proc/queue-worker/tlgrm-update/ctx/tlgrm-update-ctx.h>
#include <proc/queue-worker/tlgrm-update/tlgrm-update-subproc.h>

/* Definitions */



/* Project globals */
extern		nxs_process_t			process;
extern		nxs_chat_srv_cfg_t		nxs_chat_srv_cfg;

/* Module typedefs */



/* Module declarations */

typedef struct
{
	nxs_chat_srv_m_db_sess_type_t		type;
	nxs_chat_srv_err_t			(*handler)(nxs_chat_srv_m_tlgrm_bttn_callback_t *callback, nxs_chat_srv_m_tlgrm_update_t *update, nxs_chat_srv_u_db_sess_t *sess_ctx, nxs_chat_srv_u_db_cache_t *cache_ctx, nxs_chat_srv_m_user_ctx_t *user_ctx);
} handlers_callback_t;

typedef struct
{
	nxs_chat_srv_m_db_sess_type_t		type;
	nxs_chat_srv_err_t			(*handler)(nxs_chat_srv_u_db_queue_t *queue_ctx, nxs_chat_srv_u_db_sess_t *sess_ctx, nxs_chat_srv_u_db_cache_t *cache_ctx, nxs_chat_srv_m_user_ctx_t *user_ctx);
} handlers_message_t;

typedef struct
{
	nxs_string_t				command;
	nxs_bool_t				has_args;
	nxs_chat_srv_err_t			(*handler)(nxs_chat_srv_m_tlgrm_update_t *update, nxs_chat_srv_u_db_sess_t *sess_ctx, nxs_chat_srv_u_db_cache_t *cache_ctx, nxs_chat_srv_m_user_ctx_t *user_ctx);
	nxs_string_t				description;
} handlers_commands_t;

/* Module internal (static) functions prototypes */

// clang-format on

static size_t check_user(nxs_chat_srv_u_db_cache_t *cache_ctx, nxs_chat_srv_m_user_ctx_t *user_ctx, nxs_chat_srv_m_tlgrm_update_t *update);

static nxs_chat_srv_err_t handler_callback_exec(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                                nxs_chat_srv_u_db_sess_t * sess_ctx,
                                                nxs_chat_srv_u_db_cache_t *cache_ctx,
                                                nxs_chat_srv_m_user_ctx_t *user_ctx);

static nxs_chat_srv_err_t handler_message_exec(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                               nxs_chat_srv_u_db_sess_t * sess_ctx,
                                               nxs_chat_srv_u_db_cache_t *cache_ctx,
                                               nxs_chat_srv_m_user_ctx_t *user_ctx);

static nxs_chat_srv_err_t handler_callback_sess_type_message(nxs_chat_srv_m_tlgrm_bttn_callback_t *callback,
                                                             nxs_chat_srv_m_tlgrm_update_t *       update,
                                                             nxs_chat_srv_u_db_sess_t *            sess_ctx,
                                                             nxs_chat_srv_u_db_cache_t *           cache_ctx,
                                                             nxs_chat_srv_m_user_ctx_t *           user_ctx);
static nxs_chat_srv_err_t handler_callback_sess_type_new_issue(nxs_chat_srv_m_tlgrm_bttn_callback_t *callback,
                                                               nxs_chat_srv_m_tlgrm_update_t *       update,
                                                               nxs_chat_srv_u_db_sess_t *            sess_ctx,
                                                               nxs_chat_srv_u_db_cache_t *           cache_ctx,
                                                               nxs_chat_srv_m_user_ctx_t *           user_ctx);

static nxs_chat_srv_err_t handler_message_sess_type_empty(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                                          nxs_chat_srv_u_db_sess_t * sess_ctx,
                                                          nxs_chat_srv_u_db_cache_t *cache_ctx,
                                                          nxs_chat_srv_m_user_ctx_t *user_ctx);

static nxs_chat_srv_err_t handler_message_sess_type_message(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                                            nxs_chat_srv_u_db_sess_t * sess_ctx,
                                                            nxs_chat_srv_u_db_cache_t *cache_ctx,
                                                            nxs_chat_srv_m_user_ctx_t *user_ctx);
static nxs_chat_srv_err_t handler_message_sess_type_new_issue(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                                              nxs_chat_srv_u_db_sess_t * sess_ctx,
                                                              nxs_chat_srv_u_db_cache_t *cache_ctx,
                                                              nxs_chat_srv_m_user_ctx_t *user_ctx);

static nxs_chat_srv_err_t handler_message_reply(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                                nxs_chat_srv_u_db_sess_t * sess_ctx,
                                                nxs_chat_srv_m_user_ctx_t *user_ctx,
                                                size_t                     tlgrm_userid);
static nxs_chat_srv_err_t handler_message_reply_ext(nxs_chat_srv_u_db_sess_t *sess_ctx, size_t tlgrm_userid, size_t issue_id);

static nxs_chat_srv_err_t handler_command_start(nxs_chat_srv_m_tlgrm_update_t *update,
                                                nxs_chat_srv_u_db_sess_t *     sess_ctx,
                                                nxs_chat_srv_u_db_cache_t *    cache_ctx,
                                                nxs_chat_srv_m_user_ctx_t *    user_ctx);
static nxs_chat_srv_err_t handler_command_help(nxs_chat_srv_m_tlgrm_update_t *update,
                                               nxs_chat_srv_u_db_sess_t *     sess_ctx,
                                               nxs_chat_srv_u_db_cache_t *    cache_ctx,
                                               nxs_chat_srv_m_user_ctx_t *    user_ctx);
static nxs_chat_srv_err_t handler_command_dialogdestroy(nxs_chat_srv_m_tlgrm_update_t *update,
                                                        nxs_chat_srv_u_db_sess_t *     sess_ctx,
                                                        nxs_chat_srv_u_db_cache_t *    cache_ctx,
                                                        nxs_chat_srv_m_user_ctx_t *    user_ctx);
static nxs_chat_srv_err_t handler_command_dummy(nxs_chat_srv_m_tlgrm_update_t *update,
                                                nxs_chat_srv_u_db_sess_t *     sess_ctx,
                                                nxs_chat_srv_u_db_cache_t *    cache_ctx,
                                                nxs_chat_srv_m_user_ctx_t *    user_ctx);

static void messages_queue_clean_text_messages(nxs_chat_srv_u_db_queue_t *queue_ctx);
static void messages_queue_clean_reply(nxs_chat_srv_u_db_queue_t *queue_ctx);
static nxs_chat_srv_err_t messages_queue_elt_parse(nxs_chat_srv_u_db_sess_t *sess_ctx, nxs_chat_srv_m_tlgrm_update_t *update);
static nxs_chat_srv_err_t messages_queue_extract_files(nxs_chat_srv_u_db_sess_t *sess_ctx, nxs_chat_srv_m_tlgrm_update_t *update);

// clang-format off

/* Module initializations */

static handlers_callback_t handlers_callback[] =
{
	{NXS_CHAT_SRV_M_DB_SESS_TYPE_MESSAGE,		&handler_callback_sess_type_message},
	{NXS_CHAT_SRV_M_DB_SESS_TYPE_NEW_ISSUE,		&handler_callback_sess_type_new_issue},

	{NXS_CHAT_SRV_M_DB_SESS_TYPE_NONE,		NULL}
};

static handlers_message_t handlers_message[] =
{
	{NXS_CHAT_SRV_M_DB_SESS_TYPE_MESSAGE,		&handler_message_sess_type_message},
	{NXS_CHAT_SRV_M_DB_SESS_TYPE_NEW_ISSUE,		&handler_message_sess_type_new_issue},

	{NXS_CHAT_SRV_M_DB_SESS_TYPE_NONE,		NULL}
};

static handlers_commands_t handlers_command[] =
{
	{nxs_string("/start"),			NXS_NO,		&handler_command_start,			NXS_STRING_NULL_STR},
	{nxs_string("/help"),			NXS_NO,		&handler_command_help,			nxs_string(NXS_CHAT_SRV_TLGRM_MESSAGE_CMD_DESC_HELP)},
	{nxs_string("/dialogdestroy"),		NXS_NO,		&handler_command_dialogdestroy,		nxs_string(NXS_CHAT_SRV_TLGRM_MESSAGE_CMD_DESC_DIALOGDESTROY)},
	{nxs_string("/issuecreate"),		NXS_NO,		handler_command_dummy,			nxs_string(NXS_CHAT_SRV_TLGRM_MESSAGE_CMD_DESC_ISSUECREATE)},

	{NXS_STRING_NULL_STR,			NXS_NO,		NULL,					NXS_STRING_NULL_STR}
};

static nxs_string_t _s_cf_ignore_value	= nxs_string("1");
static nxs_string_t _s_stickers_mime	= nxs_string("image/webp");

/* Module global functions */

// clang-format on

nxs_chat_srv_err_t nxs_chat_srv_p_queue_worker_tlgrm_update_add(nxs_chat_srv_m_ra_queue_type_t type, nxs_string_t *data)
{
	nxs_string_t                  base64_decoded;
	nxs_chat_srv_m_tlgrm_update_t update;
	nxs_chat_srv_u_db_sess_t *    sess_ctx;
	nxs_chat_srv_u_db_cache_t *   cache_ctx;
	nxs_chat_srv_u_db_ids_t *     ids_ctx;
	nxs_chat_srv_m_user_ctx_t     user_ctx;
	nxs_chat_srv_u_db_queue_t *   queue_ctx;
	nxs_chat_srv_err_t            rc;
	size_t                        tlgrm_user_id, i;

	rc = NXS_CHAT_SRV_E_OK;

	nxs_string_init(&base64_decoded);

	nxs_chat_srv_c_tlgrm_update_init(&update);

	sess_ctx  = nxs_chat_srv_u_db_sess_init();
	cache_ctx = nxs_chat_srv_u_db_cache_init();
	ids_ctx   = nxs_chat_srv_u_db_ids_init();
	queue_ctx = nxs_chat_srv_u_db_queue_init();

	nxs_chat_srv_c_user_ctx_init(&user_ctx);

	nxs_base64_decode_string(&base64_decoded, data);

	if(nxs_chat_srv_c_tlgrm_update_pull_json(&update, (nxs_buf_t *)&base64_decoded) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if(nxs_chat_srv_u_db_cache_pull(cache_ctx) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if((tlgrm_user_id = check_user(cache_ctx, &user_ctx, &update)) == 0) {

		/* User not found */

		/* TODO: initial dialog with unknown user */

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if(nxs_chat_srv_u_db_ids_set(ids_ctx, user_ctx.r_userid, tlgrm_user_id) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if(nxs_chat_srv_u_db_sess_pull(sess_ctx, tlgrm_user_id) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	switch(nxs_chat_srv_c_tlgrm_update_get_type(&update)) {

		case NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_CALLBACK:

			nxs_error(rc,
			          nxs_chat_srv_u_db_queue_add(queue_ctx, tlgrm_user_id, NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_CALLBACK, data),
			          error);

			break;

		case NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_MESSAGE:

			for(i = 0; handlers_command[i].handler != NULL; i++) {

				if(handlers_command[i].has_args == NXS_NO) {

					if(nxs_string_cmp(&update.message.text, 0, &handlers_command[i].command, 0) == NXS_YES) {

						nxs_error(rc,
						          nxs_chat_srv_u_db_queue_add(
						                  queue_ctx, tlgrm_user_id, NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_COMMAND, data),
						          error);
					}
				}
				else {

					if(nxs_string_ncmp(&update.message.text,
					                   0,
					                   &handlers_command[i].command,
					                   0,
					                   nxs_string_len(&handlers_command[i].command)) == NXS_YES) {

						nxs_error(rc,
						          nxs_chat_srv_u_db_queue_add(
						                  queue_ctx, tlgrm_user_id, NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_COMMAND, data),
						          error);
					}
				}
			}

			/* if command not found */

			nxs_error(rc,
			          nxs_chat_srv_u_db_queue_add(queue_ctx, tlgrm_user_id, NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_MESSAGE, data),
			          error);

			break;

		default:

			nxs_log_write_warn(
			        &process, "[%s]: tlgrm runtime queue add error: unknown tlgrm update type", nxs_proc_get_name(&process));

			nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);

			break;
	}

error:

	nxs_string_free(&base64_decoded);

	nxs_chat_srv_c_tlgrm_update_free(&update);

	nxs_chat_srv_c_user_ctx_free(&user_ctx);

	sess_ctx  = nxs_chat_srv_u_db_sess_free(sess_ctx);
	cache_ctx = nxs_chat_srv_u_db_cache_free(cache_ctx);
	ids_ctx   = nxs_chat_srv_u_db_ids_free(ids_ctx);
	queue_ctx = nxs_chat_srv_u_db_queue_free(queue_ctx);

	return rc;
}

nxs_chat_srv_err_t nxs_chat_srv_p_queue_worker_tlgrm_update_runtime_queue(void)
{
	nxs_chat_srv_m_tlgrm_update_t *update;
	nxs_chat_srv_u_db_sess_t *     sess_ctx;
	nxs_chat_srv_u_db_cache_t *    cache_ctx;
	nxs_chat_srv_m_user_ctx_t      user_ctx;
	nxs_chat_srv_u_db_queue_t *    queue_ctx;
	nxs_chat_srv_err_t             rc;
	size_t                         tlgrm_user_id;

	rc = NXS_CHAT_SRV_E_OK;

	sess_ctx  = nxs_chat_srv_u_db_sess_init();
	cache_ctx = nxs_chat_srv_u_db_cache_init();
	queue_ctx = nxs_chat_srv_u_db_queue_init();

	nxs_chat_srv_c_user_ctx_init(&user_ctx);

	switch(nxs_chat_srv_u_db_queue_get(queue_ctx)) {

		case NXS_CHAT_SRV_E_OK:

			/* got (locked) queue */

			update = nxs_chat_srv_u_db_queue_list_get_head(queue_ctx);

			if(nxs_chat_srv_u_db_cache_pull(cache_ctx) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if((tlgrm_user_id = check_user(cache_ctx, &user_ctx, update)) == 0) {

				/* User not found */

				/* TODO: unknown user initial dialog */

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_u_db_sess_pull(sess_ctx, tlgrm_user_id) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			do {

				switch(nxs_chat_srv_c_tlgrm_update_get_type(update)) {

					case NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_CALLBACK:

						handler_callback_exec(queue_ctx, sess_ctx, cache_ctx, &user_ctx);

						nxs_chat_srv_u_tlgrm_answercallbackquery_push(&update->callback_query.id, NULL, 0);

						break;

					case NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_MESSAGE:

						handler_message_exec(queue_ctx, sess_ctx, cache_ctx, &user_ctx);

						break;

					default:

						nxs_log_write_warn(&process,
						                   "[%s]: tlgrm runtime queue error: unknown tlgrm update type",
						                   nxs_proc_get_name(&process));

						nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);

						break;
				}
			} while((update = nxs_chat_srv_u_db_queue_list_get_head(queue_ctx)) != NULL);

			break;

		case NXS_CHAT_SRV_E_EXIST:

			break;

		default:

			break;
	}

error:

	nxs_chat_srv_c_user_ctx_free(&user_ctx);

	sess_ctx  = nxs_chat_srv_u_db_sess_free(sess_ctx);
	cache_ctx = nxs_chat_srv_u_db_cache_free(cache_ctx);
	queue_ctx = nxs_chat_srv_u_db_queue_free(queue_ctx);

	return rc;
}

/* Module internal (static) functions */

static size_t check_user(nxs_chat_srv_u_db_cache_t *cache_ctx, nxs_chat_srv_m_user_ctx_t *user_ctx, nxs_chat_srv_m_tlgrm_update_t *update)
{
	nxs_string_t *tlgrm_username, *tlgrm_userlang, *s;
	size_t        tlgrm_user_id, i;
	nxs_bool_t    f;

	if(cache_ctx == NULL || user_ctx == NULL || update == NULL) {

		return 0;
	}

	if(update->callback_query._is_used == NXS_YES) {

		tlgrm_user_id  = update->callback_query.from.id;
		tlgrm_username = &update->callback_query.from.username;
		tlgrm_userlang = &update->callback_query.from.language_code;
	}
	else {

		tlgrm_user_id  = update->message.from.id;
		tlgrm_username = &update->message.from.username;
		tlgrm_userlang = &update->message.from.language_code;
	}

	if(nxs_array_count(&nxs_chat_srv_cfg.dev_accounts) > 0) {

		for(f = NXS_NO, i = 0; i < nxs_array_count(&nxs_chat_srv_cfg.dev_accounts); i++) {

			s = nxs_array_get(&nxs_chat_srv_cfg.dev_accounts, i);

			if(nxs_string_cmp(s, 0, tlgrm_username, 0) == NXS_YES) {

				f = NXS_YES;

				break;
			}
		}

		if(f == NXS_NO) {

			nxs_log_write_warn(
			        &process,
			        "[%s]: access denied: tlgrm user not in 'dev_accounts' list (tlgrm userid: %zu, tlgrm user name: %r)",
			        nxs_proc_get_name(&process),
			        tlgrm_user_id,
			        tlgrm_username);

			return 0;
		}
	}

	if(nxs_chat_srv_u_db_cache_user_get(cache_ctx, tlgrm_username, user_ctx) == NXS_CHAT_SRV_E_OK) {

		nxs_string_clone(&user_ctx->t_userlang, tlgrm_userlang);

		return tlgrm_user_id;
	}

	nxs_log_write_warn(&process,
	                   "[%s]: access denied: tlgrm user not found (tlgrm userid: %zu, tlgrm user name: %r)",
	                   nxs_proc_get_name(&process),
	                   tlgrm_user_id,
	                   tlgrm_username);

	return 0;
}

static nxs_chat_srv_err_t handler_callback_exec(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                                nxs_chat_srv_u_db_sess_t * sess_ctx,
                                                nxs_chat_srv_u_db_cache_t *cache_ctx,
                                                nxs_chat_srv_m_user_ctx_t *user_ctx)
{
	nxs_chat_srv_m_db_sess_type_t        type;
	nxs_chat_srv_m_tlgrm_bttn_callback_t callback;
	nxs_chat_srv_m_tlgrm_update_t *      update;
	nxs_chat_srv_err_t                   rc;
	size_t                               i;

	if(queue_ctx == NULL || sess_ctx == NULL || cache_ctx == NULL || user_ctx == NULL) {

		return NXS_CHAT_SRV_E_PTR;
	}

	if((update = nxs_chat_srv_u_db_queue_list_pull_head(queue_ctx)) == NULL) {

		return NXS_CHAT_SRV_E_OK;
	}

	rc = NXS_CHAT_SRV_E_OK;

	if(nxs_chat_srv_u_db_sess_check_exist(sess_ctx) == NXS_NO) {

		/* user has no sessions, skipping request */
	}
	else {

		if(nxs_chat_srv_c_tlgrm_bttn_callback_deserialize(&callback, &update->callback_query.data)) {

			nxs_chat_srv_p_queue_worker_tlgrm_update_win_error(sess_ctx, update->callback_query.message.chat.id, NULL);

			nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
		}

		type = nxs_chat_srv_u_db_sess_get_type(sess_ctx);

		for(i = 0; handlers_callback[i].type != NXS_CHAT_SRV_M_DB_SESS_TYPE_NONE; i++) {

			if(handlers_callback[i].type == type) {

				if(handlers_callback[i].handler != NULL) {

					nxs_error(
					        rc, handlers_callback[i].handler(&callback, update, sess_ctx, cache_ctx, user_ctx), error);
				}
				else {

					nxs_error(rc, NXS_CHAT_SRV_E_OK, error);
				}
			}
		}

		if(handlers_callback[i].handler != NULL) {

			nxs_error(rc, handlers_callback[i].handler(&callback, update, sess_ctx, cache_ctx, user_ctx), error);
		}
	}

error:

	if(rc != NXS_CHAT_SRV_E_OK) {

		nxs_log_write_warn(&process,
		                   "[%s]: error in 'proc.queue-worker.tlgrm-update.handler_callback_exec' (user id: %zu, "
		                   "error code: %d)",
		                   nxs_proc_get_name(&process),
		                   nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx),
		                   rc);
	}

	return rc;
}

static nxs_chat_srv_err_t handler_message_exec(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                               nxs_chat_srv_u_db_sess_t * sess_ctx,
                                               nxs_chat_srv_u_db_cache_t *cache_ctx,
                                               nxs_chat_srv_m_user_ctx_t *user_ctx)
{
	nxs_chat_srv_m_db_sess_type_t  type;
	nxs_chat_srv_m_tlgrm_update_t *update;
	nxs_chat_srv_err_t             rc;
	size_t                         i, tlgrm_userid;

	rc = NXS_CHAT_SRV_E_OK;

	tlgrm_userid = nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx);

	if((update = nxs_chat_srv_u_db_queue_list_get_head(queue_ctx)) == NULL) {

		return NXS_CHAT_SRV_E_OK;
	}

	/* looking for command */
	for(i = 0; handlers_command[i].handler != NULL; i++) {

		if(handlers_command[i].has_args == NXS_NO) {

			if(nxs_string_cmp(&update->message.text, 0, &handlers_command[i].command, 0) == NXS_YES) {

				update = nxs_chat_srv_u_db_queue_list_pull_head(queue_ctx);

				nxs_error(rc, handlers_command[i].handler(update, sess_ctx, cache_ctx, user_ctx), error);
			}
		}
		else {

			if(nxs_string_ncmp(&update->message.text,
			                   0,
			                   &handlers_command[i].command,
			                   0,
			                   nxs_string_len(&handlers_command[i].command)) == NXS_YES) {

				update = nxs_chat_srv_u_db_queue_list_pull_head(queue_ctx);

				nxs_error(rc, handlers_command[i].handler(update, sess_ctx, cache_ctx, user_ctx), error);
			}
		}
	}

	/* if command not found */

	if(update->message.reply_to_message != NULL) {

		/* reply to message */

		if(handler_message_reply(queue_ctx, sess_ctx, user_ctx, tlgrm_userid) == NXS_CHAT_SRV_E_OK) {

			nxs_error(rc, NXS_CHAT_SRV_E_OK, error);
		}

		nxs_chat_srv_p_queue_worker_tlgrm_update_win_wrong_reply_runtime(update->message.chat.id, NULL);
	}

	/* empty reply or issue ID is not determined */

	if(nxs_chat_srv_u_db_sess_check_exist(sess_ctx) == NXS_NO) {

		/* user has no sessions, creating new one and starting dialog */

		if(handler_message_sess_type_empty(queue_ctx, sess_ctx, cache_ctx, user_ctx) != NXS_CHAT_SRV_E_OK) {

			nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
		}
	}
	else {

		type = nxs_chat_srv_u_db_sess_get_type(sess_ctx);

		for(i = 0; handlers_message[i].type != NXS_CHAT_SRV_M_DB_SESS_TYPE_NONE; i++) {

			if(handlers_message[i].type == type) {

				if(handlers_message[i].handler != NULL) {

					nxs_error(rc, handlers_message[i].handler(queue_ctx, sess_ctx, cache_ctx, user_ctx), error);
				}
				else {

					nxs_error(rc, NXS_CHAT_SRV_E_OK, error);
				}
			}
		}

		if(handlers_message[i].handler != NULL) {

			nxs_error(rc, handlers_message[i].handler(queue_ctx, sess_ctx, cache_ctx, user_ctx), error);
		}
	}

error:

	if(rc != NXS_CHAT_SRV_E_OK) {

		nxs_log_write_warn(&process,
		                   "[%s]: error in 'proc.queue-worker.tlgrm-update.handler_message_exec' (user id: %zu, "
		                   "error code: %d)",
		                   nxs_proc_get_name(&process),
		                   tlgrm_userid,
		                   rc);
	}

	return rc;
}

static nxs_chat_srv_err_t handler_callback_sess_type_message(nxs_chat_srv_m_tlgrm_bttn_callback_t *callback,
                                                             nxs_chat_srv_m_tlgrm_update_t *       update,
                                                             nxs_chat_srv_u_db_sess_t *            sess_ctx,
                                                             nxs_chat_srv_u_db_cache_t *           cache_ctx,
                                                             nxs_chat_srv_m_user_ctx_t *           user_ctx)
{
	nxs_chat_srv_err_t                       rc;
	nxs_chat_srv_m_db_cache_issue_priority_t issue_priority;
	nxs_chat_srv_m_db_sess_file_t *          file;
	nxs_array_t                              cache_projects, *files, *rdmn_uploads;
	nxs_string_t *                           api_key, *message, file_name, file_path;
	nxs_chat_srv_u_db_issues_t *             db_issue_ctx;
	nxs_chat_srv_u_last_issues_t *           last_issue_ctx;
	nxs_chat_srv_u_tlgrm_attachments_t *     tlgrm_attachments_ctx;
	nxs_chat_srv_u_rdmn_attachments_t *      rdmn_attachments_ctx;
	nxs_chat_srv_u_rdmn_issues_t *           rdmn_issues_ctx;
	nxs_bool_t                               private_notes;
	size_t chat_id, bot_message_id, usr_message_id, issues_count, status_id, assigned_to_id, i, files_count;

	rc = NXS_CHAT_SRV_E_OK;

	status_id      = 0;
	assigned_to_id = 0;

	nxs_string_init(&file_name);
	nxs_string_init(&file_path);

	db_issue_ctx          = nxs_chat_srv_u_db_issues_init();
	last_issue_ctx        = nxs_chat_srv_u_last_issues_init();
	tlgrm_attachments_ctx = nxs_chat_srv_u_tlgrm_attachments_init();
	rdmn_attachments_ctx  = nxs_chat_srv_u_rdmn_attachments_init();
	rdmn_issues_ctx       = nxs_chat_srv_u_rdmn_issues_init();

	api_key        = nxs_chat_srv_u_db_sess_get_rdmn_api_key(sess_ctx);
	chat_id        = nxs_chat_srv_u_db_sess_get_chat_id(sess_ctx);
	bot_message_id = nxs_chat_srv_u_db_sess_get_bot_message_id(sess_ctx);
	usr_message_id = nxs_chat_srv_u_db_sess_get_usr_message_id(sess_ctx);
	files          = nxs_chat_srv_u_db_sess_get_files(sess_ctx);
	files_count    = nxs_chat_srv_u_db_sess_get_files_count(sess_ctx);
	message        = nxs_chat_srv_u_db_sess_t_get_message(sess_ctx);
	private_notes  = nxs_chat_srv_u_db_sess_t_get_message_is_private(sess_ctx);

	nxs_array_init2(&cache_projects, nxs_chat_srv_m_db_cache_project_t);

	switch(callback->type) {

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_NEW_ISSUE:

			/* new issue creation processing */

			nxs_chat_srv_u_db_cache_prio_get_default(cache_ctx, &issue_priority);

			nxs_chat_srv_u_db_cache_projects_get(cache_ctx, &cache_projects);

			if(nxs_chat_srv_u_db_sess_t_conv_to_new_issue(sess_ctx,
			                                              &cache_projects,
			                                              user_ctx->r_userid,
			                                              issue_priority.id,
			                                              issue_priority.name,
			                                              NULL,
			                                              NULL,
			                                              NULL) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_new_issue(sess_ctx, chat_id, bot_message_id, NXS_YES, NULL) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			break;

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE:
		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_S_IN_PROGRESS:
		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_S_NEED_FEEDBACK:
		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_S_RESOLVED:
		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_PRIVATE:
		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_WF_IGNORE:
		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_TAKE_ISSUE:

			/* add comment to selected issue processing */

			switch(callback->type) {

				case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_S_IN_PROGRESS:

					status_id = nxs_chat_srv_cfg.rdmn.status_in_progress;

					break;

				case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_S_NEED_FEEDBACK:

					status_id = nxs_chat_srv_cfg.rdmn.status_need_feedback;

					break;

				case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_S_RESOLVED:

					status_id = nxs_chat_srv_cfg.rdmn.status_resolved;

					break;

				case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_PRIVATE:

					private_notes = NXS_YES;

					break;

				case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_WF_IGNORE:

					nxs_chat_srv_u_rdmn_issues_cf_add(
					        rdmn_issues_ctx, nxs_chat_srv_cfg.rdmn.cf_ignore_status, &_s_cf_ignore_value);

					break;

				case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_TO_ISSUE_EXT_TAKE_ISSUE:

					status_id      = nxs_chat_srv_cfg.rdmn.status_in_progress;
					assigned_to_id = user_ctx->r_userid;

					break;

				default:

					if(nxs_chat_srv_u_db_sess_t_get_message_is_ext(sess_ctx) == NXS_YES) {

						if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_add_note_ext(
						           chat_id, bot_message_id, callback->object_id, NULL) != NXS_CHAT_SRV_E_OK) {

							nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
						}

						nxs_error(rc, NXS_CHAT_SRV_E_OK, error);
					}

					break;
			}

			for(i = 0; i < nxs_array_count(files); i++) {

				file = nxs_array_get(files, i);

				if(nxs_chat_srv_u_tlgrm_attachments_download(
				           tlgrm_attachments_ctx, chat_id, file, &file_name, &file_path) != NXS_CHAT_SRV_E_OK) {

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
				}

				if(nxs_chat_srv_u_rdmn_attachments_upload(
				           rdmn_attachments_ctx, api_key, &file_name, &file_path, &file->mime_type) != NXS_CHAT_SRV_E_OK) {

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
				}
			}

			rdmn_uploads = nxs_chat_srv_u_rdmn_attachments_uploads_get(rdmn_attachments_ctx);

			switch(nxs_chat_srv_u_rdmn_issues_add_note(rdmn_issues_ctx,
			                                           callback->object_id,
			                                           assigned_to_id,
			                                           message,
			                                           private_notes,
			                                           status_id,
			                                           rdmn_uploads,
			                                           api_key)) {

				case NXS_CHAT_SRV_E_OK:

					/* set issue 'callback->object_id' as last for telegram user 'tlgrm_userid' */
					nxs_chat_srv_u_last_issues_set(last_issue_ctx, chat_id, callback->object_id);

					if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_added_to_issue(
					           sess_ctx, chat_id, bot_message_id, NULL, callback->object_id) != NXS_CHAT_SRV_E_OK) {

						nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
					}

					nxs_chat_srv_u_db_issues_set(db_issue_ctx, chat_id, usr_message_id, callback->object_id);
					nxs_chat_srv_u_db_issues_set(db_issue_ctx, chat_id, bot_message_id, callback->object_id);

					nxs_chat_srv_u_db_sess_destroy(sess_ctx);

					break;

				case NXS_CHAT_SRV_E_WARN:

					if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_add_note_ext(
					           chat_id, bot_message_id, callback->object_id, NULL) != NXS_CHAT_SRV_E_OK) {

						nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
					}

					break;

				default:

					nxs_log_write_warn(
					        &process,
					        "[%s]: can't send user message to Redmine issue: can't add note into Redmine issue ("
					        "rdmn user id: %zu)",
					        nxs_proc_get_name(&process),
					        user_ctx->r_userid);

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);

					break;
			}

			break;

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_SESSION_DESTROY:

			nxs_chat_srv_u_db_sess_destroy(sess_ctx);

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_session_destroyed(
			           sess_ctx, update->callback_query.message.chat.id, NULL) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			break;

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_SELECT_ISSUE:

			if(nxs_chat_srv_u_rdmn_issues_get_query(rdmn_issues_ctx,
			                                        nxs_chat_srv_cfg.rdmn.issue_list_query_id,
			                                        callback->object_id,
			                                        NXS_CHAT_SRV_TLGRM_ISSUES_LIMIT,
			                                        api_key,
			                                        &issues_count) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_select_issue(sess_ctx,
			                                                             rdmn_issues_ctx,
			                                                             update->callback_query.message.chat.id,
			                                                             bot_message_id,
			                                                             callback->object_id,
			                                                             issues_count,
			                                                             NULL) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			break;

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_BACK:

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_begin(
			           chat_id, bot_message_id, user_ctx->r_userid, api_key, private_notes, files_count, NULL) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			break;

		default:

			nxs_log_write_warn(&process,
			                   "[%s]: wrong callback type (callback type: %d, "
			                   "user id: %zu)",
			                   nxs_proc_get_name(&process),
			                   callback->type,
			                   nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx),
			                   rc);

			nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);

			break;
	}

error:

	if(rc != NXS_CHAT_SRV_E_OK) {

		nxs_chat_srv_p_queue_worker_tlgrm_update_win_error(sess_ctx, update->callback_query.message.chat.id, NULL);

		nxs_log_write_warn(&process,
		                   "[%s]: error in 'proc.queue-worker.tlgrm-update.handler_callback_sess_type_message' ("
		                   "user id: %zu, "
		                   "error code: %d)",
		                   nxs_proc_get_name(&process),
		                   nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx),
		                   rc);
	}

	nxs_string_free(&file_name);
	nxs_string_free(&file_path);

	nxs_array_free(&cache_projects);

	db_issue_ctx          = nxs_chat_srv_u_db_issues_free(db_issue_ctx);
	last_issue_ctx        = nxs_chat_srv_u_last_issues_free(last_issue_ctx);
	tlgrm_attachments_ctx = nxs_chat_srv_u_tlgrm_attachments_free(tlgrm_attachments_ctx);
	rdmn_attachments_ctx  = nxs_chat_srv_u_rdmn_attachments_free(rdmn_attachments_ctx);
	rdmn_issues_ctx       = nxs_chat_srv_u_rdmn_issues_free(rdmn_issues_ctx);

	return rc;
}

static nxs_chat_srv_err_t handler_callback_sess_type_new_issue(nxs_chat_srv_m_tlgrm_bttn_callback_t *callback,
                                                               nxs_chat_srv_m_tlgrm_update_t *       update,
                                                               nxs_chat_srv_u_db_sess_t *            sess_ctx,
                                                               nxs_chat_srv_u_db_cache_t *           cache_ctx,
                                                               nxs_chat_srv_m_user_ctx_t *           user_ctx)
{
	size_t                                   chat_id, bot_message_id, projects_count, files_count;
	nxs_string_t *                           api_key;
	nxs_array_t                              projects;
	nxs_chat_srv_err_t                       rc;
	nxs_chat_srv_m_db_cache_issue_priority_t issue_priority;
	nxs_bool_t                               private_notes;

	nxs_array_init2(&projects, nxs_chat_srv_m_db_sess_project_t);

	rc = NXS_CHAT_SRV_E_OK;

	api_key        = nxs_chat_srv_u_db_sess_get_rdmn_api_key(sess_ctx);
	chat_id        = nxs_chat_srv_u_db_sess_get_chat_id(sess_ctx);
	bot_message_id = nxs_chat_srv_u_db_sess_get_bot_message_id(sess_ctx);
	files_count    = nxs_chat_srv_u_db_sess_get_files_count(sess_ctx);

	switch(callback->type) {

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_CREATE_ISSUE:

			if(nxs_chat_srv_u_db_sess_set_wait_for(sess_ctx, NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_ISSUE_SUBJECT) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_enter_subject(sess_ctx, chat_id, 0, NULL) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			break;

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_SELECT_PROJECT:

			if(nxs_chat_srv_u_db_sess_get_wait_for(sess_ctx) != NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_PROJECTNAME_REGEX) {

				/* skipping set 'wait for' if it already in correct value */

				if(nxs_chat_srv_u_db_sess_set_wait_for(sess_ctx, NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_PROJECTNAME_REGEX) !=
				   NXS_CHAT_SRV_E_OK) {

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
				}
			}

			if(nxs_chat_srv_u_db_sess_t_get_new_issue_projects(
			           sess_ctx, &projects, callback->object_id, NXS_CHAT_SRV_TLGRM_PROJECTS_LIMIT, &projects_count) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_log_write_warn(&process,
				                   "[%s]: can't find user selected projects from session (callback type: %d, "
				                   "user id: %zu)",
				                   nxs_proc_get_name(&process),
				                   callback->type,
				                   nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx),
				                   rc);

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_select_project(
			           sess_ctx, chat_id, bot_message_id, &projects, callback->object_id, projects_count, NULL) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			break;

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_SELECTED_PROJECT:

			if(nxs_chat_srv_u_db_sess_t_set_new_issue(sess_ctx, callback->object_id, 0, NULL, NULL, NULL, NULL) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_new_issue(sess_ctx, chat_id, bot_message_id, NXS_YES, NULL) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			break;

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_SELECT_PRIORITY:

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_select_priority(
			           sess_ctx, cache_ctx, chat_id, bot_message_id, NULL) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			break;

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_SELECTED_PRIORITY:

			if(nxs_chat_srv_u_db_cache_prio_get_by_id(cache_ctx, callback->object_id, &issue_priority) != NXS_CHAT_SRV_E_OK) {

				nxs_log_write_warn(&process,
				                   "[%s]: issue priority id does not exist (issue priority id: %zu, "
				                   "user id: %zu)",
				                   nxs_proc_get_name(&process),
				                   callback->object_id,
				                   nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx),
				                   rc);

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_u_db_sess_t_set_new_issue(sess_ctx, 0, issue_priority.id, issue_priority.name, NULL, NULL, NULL) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_new_issue(sess_ctx, chat_id, bot_message_id, NXS_YES, NULL) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			break;

		/*
		 * TODO: at the moment change description functionality for new issue is disabled
		 */
		/*
		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_CHANGE_DESCRIPTION:

		        if(nxs_chat_srv_u_db_sess_set_wait_for(sess_ctx, NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_ISSUE_DESCRIPTION) !=
		           NXS_CHAT_SRV_E_OK) {

		                nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
		        }

		        if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_enter_description(sess_ctx, chat_id, 0, NULL) !=
		           NXS_CHAT_SRV_E_OK) {

		                nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
		        }

		        break;
		*/

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_SESSION_DESTROY:

			nxs_chat_srv_u_db_sess_destroy(sess_ctx);

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_session_destroyed(
			           sess_ctx, update->callback_query.message.chat.id, NULL) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			break;

		case NXS_CHAT_SRV_M_TLGRM_BTTN_CALLBACK_TYPE_BACK:

			if(nxs_chat_srv_u_db_sess_t_conv_to_message(sess_ctx, NULL) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			private_notes = nxs_chat_srv_u_db_sess_t_get_message_is_private(sess_ctx);

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_begin(
			           chat_id, bot_message_id, user_ctx->r_userid, api_key, private_notes, files_count, NULL) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			break;

		default:

			nxs_log_write_warn(&process,
			                   "[%s]: wrong callback type (callback type: %d, "
			                   "user id: %zu)",
			                   nxs_proc_get_name(&process),
			                   callback->type,
			                   nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx),
			                   rc);

			nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);

			break;
	}

error:

	nxs_array_free(&projects);

	if(rc != NXS_CHAT_SRV_E_OK) {

		nxs_chat_srv_p_queue_worker_tlgrm_update_win_error(sess_ctx, update->callback_query.message.chat.id, NULL);

		nxs_log_write_warn(&process,
		                   "[%s]: error in 'proc.queue-worker.tlgrm-update.handler_callback_sess_type_new_issue' ("
		                   "user id: %zu, "
		                   "error code: %d)",
		                   nxs_proc_get_name(&process),
		                   nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx),
		                   rc);
	}

	return rc;
}

static nxs_chat_srv_err_t handler_message_sess_type_empty(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                                          nxs_chat_srv_u_db_sess_t * sess_ctx,
                                                          nxs_chat_srv_u_db_cache_t *cache_ctx,
                                                          nxs_chat_srv_m_user_ctx_t *user_ctx)
{
	nxs_chat_srv_u_rdmn_user_t *   rdmn_user_ctx;
	nxs_chat_srv_m_tlgrm_update_t *update;
	nxs_chat_srv_m_tlgrm_message_t response_message;
	nxs_bool_t                     response_status, private_message;
	nxs_string_t *                 rdmn_api_key;
	nxs_buf_t                      response_buf;
	nxs_chat_srv_err_t             rc;
	size_t                         message_id, chat_id, files_count;

	if(queue_ctx == NULL || sess_ctx == NULL || cache_ctx == NULL || user_ctx == NULL) {

		return NXS_CHAT_SRV_E_PTR;
	}

	if((update = nxs_chat_srv_u_db_queue_list_get_head(queue_ctx)) == NULL) {

		return NXS_CHAT_SRV_E_OK;
	}

	rc = NXS_CHAT_SRV_E_OK;

	rdmn_user_ctx = nxs_chat_srv_u_rdmn_user_init();

	nxs_chat_srv_c_tlgrm_message_init(&response_message);

	nxs_buf_init2(&response_buf);

	message_id = update->message.message_id;
	chat_id    = update->message.chat.id;

	if(nxs_chat_srv_u_rdmn_user_pull(rdmn_user_ctx, user_ctx->r_userid) != NXS_CHAT_SRV_E_OK) {

		nxs_log_write_warn(&process,
		                   "[%s]: can't get user data from Redmine ("
		                   "rdmn user id: %zu)",
		                   nxs_proc_get_name(&process),
		                   user_ctx->r_userid);

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	rdmn_api_key = nxs_chat_srv_u_rdmn_user_get_api_key(rdmn_user_ctx);

	if(nxs_string_len(rdmn_api_key) == 0) {

		nxs_log_write_warn(
		        &process, "[%s]: user api key has zero size (rdmn user id: %zu)", nxs_proc_get_name(&process), user_ctx->r_userid);

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if(nxs_chat_srv_u_db_sess_start(sess_ctx,
	                                rdmn_api_key,
	                                0,
	                                message_id,
	                                0,
	                                NXS_CHAT_SRV_M_DB_SESS_TYPE_MESSAGE,
	                                NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_NONE) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	for(; update != NULL; update = nxs_chat_srv_u_db_queue_list_del_head(queue_ctx)) {

		if(nxs_chat_srv_c_tlgrm_update_get_type(update) != NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_MESSAGE ||
		   update->message.reply_to_message != NULL) {

			break;
		}
		else {

			if(messages_queue_elt_parse(sess_ctx, update) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}
		}
	}

	private_message = nxs_chat_srv_u_db_sess_t_get_message_is_private(sess_ctx);
	files_count     = nxs_chat_srv_u_db_sess_get_files_count(sess_ctx);

	if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_begin(
	           chat_id, 0, user_ctx->r_userid, rdmn_api_key, private_message, files_count, &response_buf) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if(nxs_chat_srv_c_tlgrm_message_result_pull_json(&response_message, &response_status, &response_buf) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if(nxs_chat_srv_u_db_sess_set_ids(sess_ctx, response_message.chat.id, 0, response_message.message_id) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

error:

	nxs_buf_free(&response_buf);

	nxs_chat_srv_c_tlgrm_message_free(&response_message);

	rdmn_user_ctx = nxs_chat_srv_u_rdmn_user_free(rdmn_user_ctx);

	if(rc != NXS_CHAT_SRV_E_OK) {

		nxs_chat_srv_p_queue_worker_tlgrm_update_win_error(sess_ctx, chat_id, NULL);

		nxs_log_write_warn(&process,
		                   "[%s]: error in 'proc.queue-worker.tlgrm-update.handler_message_sess_type_empty' ("
		                   "user id: %zu, "
		                   "error code: %d)",
		                   nxs_proc_get_name(&process),
		                   nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx),
		                   rc);
	}

	return rc;
}

static nxs_chat_srv_err_t handler_message_sess_type_message(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                                            nxs_chat_srv_u_db_sess_t * sess_ctx,
                                                            nxs_chat_srv_u_db_cache_t *cache_ctx,
                                                            nxs_chat_srv_m_user_ctx_t *user_ctx)
{
	nxs_chat_srv_m_tlgrm_update_t *update;
	nxs_chat_srv_err_t             rc;
	nxs_string_t *                 api_key;
	size_t                         tlgrm_userid, bot_message_id, files_count;
	nxs_bool_t                     f, private_notes;

	rc = NXS_CHAT_SRV_E_OK;

	tlgrm_userid   = nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx);
	bot_message_id = nxs_chat_srv_u_db_sess_get_bot_message_id(sess_ctx);
	api_key        = nxs_chat_srv_u_db_sess_get_rdmn_api_key(sess_ctx);
	private_notes  = nxs_chat_srv_u_db_sess_t_get_message_is_private(sess_ctx);

	/*
	 * Processing queue and add all files into existing session (updating windown on client).
	 * Messages withot attachments will be removed from queue.
	 */

	for(f = NXS_NO, update = nxs_chat_srv_u_db_queue_list_get_head(queue_ctx); update != NULL;
	    update = nxs_chat_srv_u_db_queue_list_del_head(queue_ctx)) {

		if((rc = messages_queue_extract_files(sess_ctx, update)) == NXS_CHAT_SRV_E_ERR) {

			return NXS_CHAT_SRV_E_ERR;
		}

		if(rc == NXS_CHAT_SRV_E_TYPE) {

			break;
		}

		if(rc == NXS_CHAT_SRV_E_OK) {

			f = NXS_YES;
		}

		/* if rc == NXS_CHAT_SRV_E_EXIST: continue */
	}

	if(f == NXS_YES) {

		files_count = nxs_chat_srv_u_db_sess_get_files_count(sess_ctx);

		nxs_error(rc,
		          nxs_chat_srv_p_queue_worker_tlgrm_update_win_begin(
		                  tlgrm_userid, bot_message_id, user_ctx->r_userid, api_key, private_notes, files_count, NULL),
		          error);
	}
	else {

		nxs_error(rc, nxs_chat_srv_p_queue_worker_tlgrm_update_win_wrong_action(sess_ctx, tlgrm_userid, NULL), error);
	}

error:

	if(rc != NXS_CHAT_SRV_E_OK) {

		nxs_log_write_warn(&process,
		                   "[%s]: error in 'proc.queue-worker.tlgrm-update.handler_message_sess_type_message' ("
		                   "user id: %zu, "
		                   "error code: %d)",
		                   nxs_proc_get_name(&process),
		                   nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx),
		                   rc);
	}

	return rc;
}

static nxs_chat_srv_err_t handler_message_sess_type_new_issue(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                                              nxs_chat_srv_u_db_sess_t * sess_ctx,
                                                              nxs_chat_srv_u_db_cache_t *cache_ctx,
                                                              nxs_chat_srv_m_user_ctx_t *user_ctx)
{
	nxs_string_t                  description, subject, project_name_regex, *api_key, file_name, file_path;
	size_t                        chat_id, bot_message_id, usr_message_id, projects_count, project_id, priority_id, new_issue_id, i;
	nxs_chat_srv_u_db_issues_t *  db_issue_ctx;
	nxs_chat_srv_u_last_issues_t *last_issue_ctx;
	nxs_chat_srv_u_tlgrm_attachments_t *tlgrm_attachments_ctx;
	nxs_chat_srv_u_rdmn_attachments_t * rdmn_attachments_ctx;
	nxs_chat_srv_u_rdmn_issues_t *      rdmn_issues_ctx;
	nxs_chat_srv_m_tlgrm_message_t      response_message;
	nxs_chat_srv_m_tlgrm_update_t *     update;
	nxs_chat_srv_m_db_sess_file_t *     file;
	nxs_bool_t                          response_status, is_private, f;
	nxs_array_t                         projects, *files, *rdmn_uploads;
	nxs_buf_t                           response_buf;
	nxs_chat_srv_err_t                  rc;

	/*
	 * Processing queue and add all files into existing session (updating windown on client).
	 */
	for(f = NXS_NO, update = nxs_chat_srv_u_db_queue_list_get_head(queue_ctx); update != NULL;
	    update = nxs_chat_srv_u_db_queue_list_del_head(queue_ctx)) {

		if((rc = messages_queue_extract_files(sess_ctx, update)) == NXS_CHAT_SRV_E_ERR) {

			return NXS_CHAT_SRV_E_ERR;
		}

		if(rc == NXS_CHAT_SRV_E_OK) {

			f = NXS_YES;
		}
		else {

			break;
		}
	}

	if(f == NXS_YES) {

		chat_id        = nxs_chat_srv_u_db_sess_get_chat_id(sess_ctx);
		bot_message_id = nxs_chat_srv_u_db_sess_get_bot_message_id(sess_ctx);

		if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_new_issue(sess_ctx, chat_id, bot_message_id, NXS_YES, NULL) !=
		   NXS_CHAT_SRV_E_OK) {

			return NXS_CHAT_SRV_E_ERR;
		}

		if(nxs_chat_srv_u_db_sess_set_wait_for(sess_ctx, NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_NONE) != NXS_CHAT_SRV_E_OK) {

			return NXS_CHAT_SRV_E_ERR;
		}

		return NXS_CHAT_SRV_E_OK;
	}

	rc = NXS_CHAT_SRV_E_OK;

	nxs_string_init(&description);
	nxs_string_init(&subject);
	nxs_string_init(&project_name_regex);
	nxs_string_init(&file_name);
	nxs_string_init(&file_path);

	nxs_buf_init2(&response_buf);

	nxs_array_init2(&projects, nxs_chat_srv_m_db_sess_project_t);

	nxs_chat_srv_c_tlgrm_message_init(&response_message);

	api_key        = nxs_chat_srv_u_db_sess_get_rdmn_api_key(sess_ctx);
	chat_id        = nxs_chat_srv_u_db_sess_get_chat_id(sess_ctx);
	bot_message_id = nxs_chat_srv_u_db_sess_get_bot_message_id(sess_ctx);
	usr_message_id = nxs_chat_srv_u_db_sess_get_usr_message_id(sess_ctx);
	files          = nxs_chat_srv_u_db_sess_get_files(sess_ctx);

	db_issue_ctx          = nxs_chat_srv_u_db_issues_init();
	last_issue_ctx        = nxs_chat_srv_u_last_issues_init();
	tlgrm_attachments_ctx = nxs_chat_srv_u_tlgrm_attachments_init();
	rdmn_attachments_ctx  = nxs_chat_srv_u_rdmn_attachments_init();
	rdmn_issues_ctx       = nxs_chat_srv_u_rdmn_issues_init();

	switch(nxs_chat_srv_u_db_sess_get_wait_for(sess_ctx)) {

		case NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_ISSUE_SUBJECT:

			/* setup subject */

			update = nxs_chat_srv_u_db_queue_list_pull_head(queue_ctx);

			if(nxs_chat_srv_u_db_sess_t_get_new_issue(
			           sess_ctx, &project_id, NULL, &priority_id, NULL, &subject, &description, &is_private, NULL, NULL) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_u_db_sess_set_wait_for(sess_ctx, NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_NONE) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_string_cmp(&subject, 0, &update->message.text, 0) != NXS_YES) {

				/* if subject is changed */

				nxs_string_clone(&subject, &update->message.text);

				if(nxs_chat_srv_u_db_sess_t_set_new_issue(sess_ctx, 0, 0, NULL, &update->message.text, NULL, NULL) !=
				   NXS_CHAT_SRV_E_OK) {

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
				}

				if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_new_issue(
				           sess_ctx, chat_id, bot_message_id, NXS_NO, NULL) != NXS_CHAT_SRV_E_OK) {

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
				}
			}

			for(i = 0; i < nxs_array_count(files); i++) {

				file = nxs_array_get(files, i);

				if(nxs_chat_srv_u_tlgrm_attachments_download(
				           tlgrm_attachments_ctx, chat_id, file, &file_name, &file_path) != NXS_CHAT_SRV_E_OK) {

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
				}

				if(nxs_chat_srv_u_rdmn_attachments_upload(
				           rdmn_attachments_ctx, api_key, &file_name, &file_path, &file->mime_type) != NXS_CHAT_SRV_E_OK) {

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
				}
			}

			rdmn_uploads = nxs_chat_srv_u_rdmn_attachments_uploads_get(rdmn_attachments_ctx);

			if(nxs_chat_srv_u_rdmn_issues_create(rdmn_issues_ctx,
			                                     project_id,
			                                     priority_id,
			                                     &subject,
			                                     &description,
			                                     is_private,
			                                     &new_issue_id,
			                                     rdmn_uploads,
			                                     api_key) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			/* set issue 'new_issue_id' as last for telegram user 'chat_id' */
			nxs_chat_srv_u_last_issues_set(last_issue_ctx, chat_id, new_issue_id);

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_issue_created(sess_ctx, chat_id, 0, new_issue_id, &response_buf) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_c_tlgrm_message_result_pull_json(&response_message, &response_status, &response_buf) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			nxs_chat_srv_u_db_issues_set(db_issue_ctx, chat_id, usr_message_id, new_issue_id);
			nxs_chat_srv_u_db_issues_set(db_issue_ctx, chat_id, bot_message_id, new_issue_id);
			nxs_chat_srv_u_db_issues_set(db_issue_ctx, chat_id, response_message.message_id, new_issue_id);

			nxs_chat_srv_u_db_sess_destroy(sess_ctx);

			break;

		case NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_ISSUE_DESCRIPTION:

			/* setup description */

			/*
			 * TODO: at the moment change description functionality for new issue is disabled
			 */
			/*
			if(nxs_chat_srv_u_db_sess_t_get_new_issue(
			           sess_ctx, NULL, NULL, NULL, NULL, NULL, &description, &is_private, &is_ext, NULL) != NXS_CHAT_SRV_E_OK) {

			        nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_u_db_sess_set_wait_for(sess_ctx, NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_NONE) != NXS_CHAT_SRV_E_OK) {

			        nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_string_cmp(&description, 0, &update->message.text, 0) != NXS_YES ||
			   is_private != nxs_chat_srv_c_notes_check_private(&update->message.text, NULL) ||
			   is_ext != nxs_chat_srv_c_notes_check_ext(&update->message.text, NULL)) {

			        // if description or its privacy is changed

			        if(nxs_chat_srv_u_db_sess_t_set_new_issue(sess_ctx, 0, 0, NULL, NULL, &update->message.text, NULL) !=
			           NXS_CHAT_SRV_E_OK) {

			                nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			        }

			        if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_new_issue(
			                   sess_ctx, chat_id, bot_message_id, NXS_YES, NULL) != NXS_CHAT_SRV_E_OK) {

			                nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			        }
			}

			if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_description_changed(sess_ctx, chat_id, NULL) != NXS_CHAT_SRV_E_OK) {

			        nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}
			*/

			break;

		case NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_PROJECTNAME_REGEX:

			/* setup projectname regex */

			update = nxs_chat_srv_u_db_queue_list_pull_head(queue_ctx);

			if(nxs_chat_srv_u_db_sess_t_get_new_issue(
			           sess_ctx, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &project_name_regex) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_u_db_sess_set_wait_for(sess_ctx, NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_NONE) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_string_cmp(&project_name_regex, 0, &update->message.text, 0) != NXS_YES) {

				/* if project name regex is changed */

				if(nxs_chat_srv_u_db_sess_t_set_new_issue(sess_ctx, 0, 0, NULL, NULL, NULL, &update->message.text) !=
				   NXS_CHAT_SRV_E_OK) {

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
				}

				if(nxs_chat_srv_u_db_sess_t_get_new_issue_projects(
				           sess_ctx, &projects, 0, NXS_CHAT_SRV_TLGRM_PROJECTS_LIMIT, &projects_count) !=
				   NXS_CHAT_SRV_E_OK) {

					nxs_log_write_warn(&process,
					                   "[%s]: can't find user projects in cache ("
					                   "user id: %zu)",
					                   nxs_proc_get_name(&process),
					                   nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx),
					                   rc);

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
				}

				if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_select_project(
				           sess_ctx, chat_id, bot_message_id, &projects, 0, projects_count, NULL) != NXS_CHAT_SRV_E_OK) {

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
				}
			}

			break;

		default:

			messages_queue_clean_text_messages(queue_ctx);

			nxs_error(rc, nxs_chat_srv_p_queue_worker_tlgrm_update_win_wrong_action(sess_ctx, chat_id, NULL), error);

			break;
	}

error:

	if(rc != NXS_CHAT_SRV_E_OK) {

		nxs_chat_srv_p_queue_worker_tlgrm_update_win_error(sess_ctx, chat_id, NULL);

		nxs_log_write_warn(&process,
		                   "[%s]: error in 'proc.queue-worker.tlgrm-update.handler_message_sess_type_new_issue' ("
		                   "user id: %zu, "
		                   "error code: %d)",
		                   nxs_proc_get_name(&process),
		                   nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx),
		                   rc);
	}

	db_issue_ctx          = nxs_chat_srv_u_db_issues_free(db_issue_ctx);
	last_issue_ctx        = nxs_chat_srv_u_last_issues_free(last_issue_ctx);
	tlgrm_attachments_ctx = nxs_chat_srv_u_tlgrm_attachments_free(tlgrm_attachments_ctx);
	rdmn_attachments_ctx  = nxs_chat_srv_u_rdmn_attachments_free(rdmn_attachments_ctx);
	rdmn_issues_ctx       = nxs_chat_srv_u_rdmn_issues_free(rdmn_issues_ctx);

	nxs_chat_srv_c_tlgrm_message_free(&response_message);

	nxs_array_free(&projects);

	nxs_buf_free(&response_buf);

	nxs_string_free(&description);
	nxs_string_free(&subject);
	nxs_string_free(&project_name_regex);
	nxs_string_free(&file_name);
	nxs_string_free(&file_path);

	return rc;
}

static nxs_chat_srv_err_t handler_message_reply(nxs_chat_srv_u_db_queue_t *queue_ctx,
                                                nxs_chat_srv_u_db_sess_t * sess_ctx,
                                                nxs_chat_srv_m_user_ctx_t *user_ctx,
                                                size_t                     tlgrm_userid)
{
	nxs_chat_srv_u_db_issues_t *        db_issue_ctx;
	nxs_chat_srv_u_rdmn_user_t *        rdmn_user_ctx;
	nxs_chat_srv_u_last_issues_t *      last_issue_ctx;
	nxs_chat_srv_u_tlgrm_attachments_t *tlgrm_attachments_ctx;
	nxs_chat_srv_u_rdmn_attachments_t * rdmn_attachments_ctx;
	nxs_chat_srv_u_rdmn_issues_t *      rdmn_issues_ctx;
	nxs_chat_srv_m_tlgrm_update_t *     update;
	nxs_chat_srv_m_db_sess_file_t *     file;
	nxs_string_t *                      rdmn_api_key, *message, file_name, file_path;
	nxs_chat_srv_err_t                  rc;
	nxs_array_t *                       files, *rdmn_uploads;
	nxs_bool_t                          private_notes;
	size_t                              issue_id, reply_to_message, message_id, i;

	if((update = nxs_chat_srv_u_db_queue_list_get_head(queue_ctx)) == NULL) {

		return NXS_CHAT_SRV_E_OK;
	}

	rc = NXS_CHAT_SRV_E_OK;

	db_issue_ctx          = nxs_chat_srv_u_db_issues_init();
	rdmn_user_ctx         = nxs_chat_srv_u_rdmn_user_init();
	last_issue_ctx        = nxs_chat_srv_u_last_issues_init();
	tlgrm_attachments_ctx = nxs_chat_srv_u_tlgrm_attachments_init();
	rdmn_attachments_ctx  = nxs_chat_srv_u_rdmn_attachments_init();
	rdmn_issues_ctx       = nxs_chat_srv_u_rdmn_issues_init();

	issue_id         = 0;
	reply_to_message = update->message.reply_to_message->message_id;
	message_id       = update->message.message_id;

	nxs_string_init(&file_name);
	nxs_string_init(&file_path);

	if(nxs_chat_srv_u_db_sess_check_exist(sess_ctx) == NXS_YES) {

		messages_queue_clean_reply(queue_ctx);

		nxs_error(rc, nxs_chat_srv_p_queue_worker_tlgrm_update_win_wrong_action(sess_ctx, tlgrm_userid, NULL), error);
	}

	if(nxs_chat_srv_u_db_issues_get(db_issue_ctx, tlgrm_userid, reply_to_message, &issue_id) != NXS_CHAT_SRV_E_OK) {

		nxs_log_write_warn(&process,
		                   "[%s]: can't send user reply into Redmine: can't get issue_id ("
		                   "user id: %zu)",
		                   nxs_proc_get_name(&process),
		                   tlgrm_userid);

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if(issue_id == 0) {

		nxs_log_write_warn(&process,
		                   "[%s]: can't send user reply into Redmine: issue_id is 0("
		                   "user id: %zu)",
		                   nxs_proc_get_name(&process),
		                   tlgrm_userid);

		nxs_error(rc, NXS_CHAT_SRV_E_EXIST, error);
	}

	if(nxs_chat_srv_u_rdmn_user_pull(rdmn_user_ctx, user_ctx->r_userid) != NXS_CHAT_SRV_E_OK) {

		nxs_log_write_warn(&process,
		                   "[%s]: can't send user reply into Redmine: can't get user data from Redmine ("
		                   "user id: %zu)",
		                   nxs_proc_get_name(&process),
		                   tlgrm_userid);

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	rdmn_api_key = nxs_chat_srv_u_rdmn_user_get_api_key(rdmn_user_ctx);

	if(nxs_string_len(rdmn_api_key) == 0) {

		nxs_log_write_warn(&process,
		                   "[%s]: can't send user reply into Redmine: user api key has zero size (user id: %zu)",
		                   nxs_proc_get_name(&process),
		                   tlgrm_userid);

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if(nxs_chat_srv_u_db_sess_start(sess_ctx,
	                                rdmn_api_key,
	                                tlgrm_userid,
	                                message_id,
	                                0,
	                                NXS_CHAT_SRV_M_DB_SESS_TYPE_MESSAGE,
	                                NXS_CHAT_SRV_M_DB_SESS_WAIT_FOR_TYPE_NONE) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	for(; update != NULL; update = nxs_chat_srv_u_db_queue_list_del_head(queue_ctx)) {

		if(nxs_chat_srv_c_tlgrm_update_get_type(update) != NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_MESSAGE ||
		   update->message.reply_to_message == NULL || update->message.reply_to_message->message_id != reply_to_message) {

			break;
		}
		else {

			if(messages_queue_elt_parse(sess_ctx, update) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}
		}
	}

	message = nxs_chat_srv_u_db_sess_t_get_message(sess_ctx);

	private_notes = nxs_chat_srv_u_db_sess_t_get_message_is_private(sess_ctx);

	if(nxs_chat_srv_u_db_sess_t_get_message_is_ext(sess_ctx) == NXS_YES) {

		/* if '/ext ' command specified */

		if(handler_message_reply_ext(sess_ctx, tlgrm_userid, issue_id) != NXS_CHAT_SRV_E_OK) {

			nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
		}
	}
	else {

		files = nxs_chat_srv_u_db_sess_get_files(sess_ctx);

		for(i = 0; i < nxs_array_count(files); i++) {

			file = nxs_array_get(files, i);

			if(nxs_chat_srv_u_tlgrm_attachments_download(tlgrm_attachments_ctx, tlgrm_userid, file, &file_name, &file_path) !=
			   NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}

			if(nxs_chat_srv_u_rdmn_attachments_upload(
			           rdmn_attachments_ctx, rdmn_api_key, &file_name, &file_path, &file->mime_type) != NXS_CHAT_SRV_E_OK) {

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
			}
		}

		rdmn_uploads = nxs_chat_srv_u_rdmn_attachments_uploads_get(rdmn_attachments_ctx);

		switch(nxs_chat_srv_u_rdmn_issues_add_note(
		               rdmn_issues_ctx, issue_id, 0, message, private_notes, 0, rdmn_uploads, rdmn_api_key) != NXS_CHAT_SRV_E_OK) {

			case NXS_CHAT_SRV_E_OK:

				/* set issue 'issue_id' as last for telegram user 'tlgrm_userid' */
				nxs_chat_srv_u_last_issues_set(last_issue_ctx, tlgrm_userid, issue_id);

				if(nxs_chat_srv_u_db_issues_set(db_issue_ctx, tlgrm_userid, message_id, issue_id) != NXS_CHAT_SRV_E_OK) {

					nxs_log_write_warn(&process,
					                   "[%s]: can't send user reply into Redmine: can't set value into db-issues ("
					                   "user id: %zu)",
					                   nxs_proc_get_name(&process),
					                   tlgrm_userid);
				}

				nxs_chat_srv_u_db_sess_destroy(sess_ctx);

				break;

			case NXS_CHAT_SRV_E_WARN:

				/* On HTTP/422 */

				if(handler_message_reply_ext(sess_ctx, tlgrm_userid, issue_id) != NXS_CHAT_SRV_E_OK) {

					nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
				}

				break;

			default:

				nxs_log_write_warn(&process,
				                   "[%s]: can't send user message to Redmine issue: can't add note into Redmine issue ("
				                   "rdmn user id: %zu)",
				                   nxs_proc_get_name(&process),
				                   user_ctx->r_userid);

				nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);

				break;
		}
	}

error:

	db_issue_ctx          = nxs_chat_srv_u_db_issues_free(db_issue_ctx);
	rdmn_user_ctx         = nxs_chat_srv_u_rdmn_user_free(rdmn_user_ctx);
	last_issue_ctx        = nxs_chat_srv_u_last_issues_free(last_issue_ctx);
	tlgrm_attachments_ctx = nxs_chat_srv_u_tlgrm_attachments_free(tlgrm_attachments_ctx);
	rdmn_attachments_ctx  = nxs_chat_srv_u_rdmn_attachments_free(rdmn_attachments_ctx);
	rdmn_issues_ctx       = nxs_chat_srv_u_rdmn_issues_free(rdmn_issues_ctx);

	nxs_string_free(&file_name);
	nxs_string_free(&file_path);

	return rc;
}

static nxs_chat_srv_err_t handler_message_reply_ext(nxs_chat_srv_u_db_sess_t *sess_ctx, size_t tlgrm_userid, size_t issue_id)
{
	nxs_chat_srv_err_t             rc;
	nxs_chat_srv_m_tlgrm_message_t response_message;
	nxs_buf_t                      response_buf;
	nxs_bool_t                     response_status;

	if(sess_ctx == NULL) {

		return NXS_CHAT_SRV_E_PTR;
	}

	rc = NXS_CHAT_SRV_E_OK;

	nxs_chat_srv_c_tlgrm_message_init(&response_message);

	nxs_buf_init2(&response_buf);

	if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_add_note_ext(tlgrm_userid, 0, issue_id, &response_buf) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if(nxs_chat_srv_c_tlgrm_message_result_pull_json(&response_message, &response_status, &response_buf) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if(nxs_chat_srv_u_db_sess_set_ids(sess_ctx, 0, 0, response_message.message_id) != NXS_CHAT_SRV_E_OK) {

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

error:

	nxs_chat_srv_c_tlgrm_message_free(&response_message);

	nxs_buf_free(&response_buf);

	return rc;
}

static nxs_chat_srv_err_t handler_command_start(nxs_chat_srv_m_tlgrm_update_t *update,
                                                nxs_chat_srv_u_db_sess_t *     sess_ctx,
                                                nxs_chat_srv_u_db_cache_t *    cache_ctx,
                                                nxs_chat_srv_m_user_ctx_t *    user_ctx)
{
	size_t tlgrm_userid;

	if(update == NULL || sess_ctx == NULL || cache_ctx == NULL || user_ctx == NULL) {

		return NXS_CHAT_SRV_E_PTR;
	}

	tlgrm_userid = nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx);

	return nxs_chat_srv_p_queue_worker_tlgrm_update_win_hello_runtime(tlgrm_userid, &user_ctx->r_userfname, NULL);
}

static nxs_chat_srv_err_t handler_command_help(nxs_chat_srv_m_tlgrm_update_t *update,
                                               nxs_chat_srv_u_db_sess_t *     sess_ctx,
                                               nxs_chat_srv_u_db_cache_t *    cache_ctx,
                                               nxs_chat_srv_m_user_ctx_t *    user_ctx)
{
	size_t             tlgrm_userid;
	nxs_string_t       message, cmd_description;
	nxs_chat_srv_err_t rc;
	size_t             i;

	if(update == NULL || sess_ctx == NULL || cache_ctx == NULL || user_ctx == NULL) {

		return NXS_CHAT_SRV_E_PTR;
	}

	nxs_string_init(&message);
	nxs_string_init_empty(&cmd_description);

	for(i = 0; nxs_string_len(&handlers_command[i].command) > 0; i++) {

		if(nxs_string_len(&handlers_command[i].description) == 0) {

			continue;
		}

		if(handlers_command[i].has_args == NXS_NO) {

			nxs_string_printf2_cat(
			        &cmd_description, "%r - %r\n", &handlers_command[i].command, &handlers_command[i].description);
		}
		else {

			nxs_string_printf2_cat(&cmd_description,
			                       "%r <i>arguments</i> - %r\n",
			                       &handlers_command[i].command,
			                       &handlers_command[i].description);
		}
	}

	nxs_string_printf(&message, NXS_CHAT_SRV_TLGRM_MESSAGE_HELP, &cmd_description);

	tlgrm_userid = nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx);

	rc = nxs_chat_srv_p_queue_worker_tlgrm_update_win_gen_notice(tlgrm_userid, 0, &message, NULL);

	nxs_string_free(&message);
	nxs_string_free(&cmd_description);

	return rc;
}

static nxs_chat_srv_err_t handler_command_dialogdestroy(nxs_chat_srv_m_tlgrm_update_t *update,
                                                        nxs_chat_srv_u_db_sess_t *     sess_ctx,
                                                        nxs_chat_srv_u_db_cache_t *    cache_ctx,
                                                        nxs_chat_srv_m_user_ctx_t *    user_ctx)
{
	nxs_chat_srv_err_t rc;

	rc = NXS_CHAT_SRV_E_OK;

	if(update == NULL || sess_ctx == NULL || cache_ctx == NULL || user_ctx == NULL) {

		return NXS_CHAT_SRV_E_PTR;
	}

	nxs_chat_srv_u_db_sess_destroy(sess_ctx);

	if(nxs_chat_srv_p_queue_worker_tlgrm_update_win_session_destroyed(sess_ctx, update->message.chat.id, NULL) != NXS_CHAT_SRV_E_OK) {

		rc = NXS_CHAT_SRV_E_ERR;
	}

	return rc;
}

static nxs_chat_srv_err_t handler_command_dummy(nxs_chat_srv_m_tlgrm_update_t *update,
                                                nxs_chat_srv_u_db_sess_t *     sess_ctx,
                                                nxs_chat_srv_u_db_cache_t *    cache_ctx,
                                                nxs_chat_srv_m_user_ctx_t *    user_ctx)
{
	size_t             tlgrm_userid;
	nxs_string_t       message;
	nxs_chat_srv_err_t rc;

	if(update == NULL || sess_ctx == NULL || cache_ctx == NULL || user_ctx == NULL) {

		return NXS_CHAT_SRV_E_PTR;
	}

	nxs_string_init(&message);

	nxs_string_printf(&message, NXS_CHAT_SRV_TLGRM_MESSAGE_DUMMY);

	tlgrm_userid = nxs_chat_srv_u_db_sess_get_tlgrm_userid(sess_ctx);

	rc = nxs_chat_srv_p_queue_worker_tlgrm_update_win_gen_notice(tlgrm_userid, 0, &message, NULL);

	nxs_string_free(&message);

	return rc;
}

static void messages_queue_clean_text_messages(nxs_chat_srv_u_db_queue_t *queue_ctx)
{
	nxs_chat_srv_m_tlgrm_update_t *update;

	for(update = nxs_chat_srv_u_db_queue_list_get_head(queue_ctx); update != NULL;
	    update = nxs_chat_srv_u_db_queue_list_del_head(queue_ctx)) {

		if(nxs_chat_srv_c_tlgrm_update_get_type(update) == NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_MESSAGE &&
		   nxs_array_count(&update->message.photo) == 0) {

			continue;
		}
		else {

			break;
		}
	}
}

static void messages_queue_clean_reply(nxs_chat_srv_u_db_queue_t *queue_ctx)
{
	nxs_chat_srv_m_tlgrm_update_t *update;

	for(update = nxs_chat_srv_u_db_queue_list_get_head(queue_ctx); update != NULL;
	    update = nxs_chat_srv_u_db_queue_list_del_head(queue_ctx)) {

		if(nxs_chat_srv_c_tlgrm_update_get_type(update) != NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_MESSAGE ||
		   update->message.reply_to_message == NULL) {

			break;
		}
	}
}

static nxs_chat_srv_err_t messages_queue_elt_parse(nxs_chat_srv_u_db_sess_t *sess_ctx, nxs_chat_srv_m_tlgrm_update_t *update)
{
	nxs_string_t *s;

	if(nxs_string_len(&update->message.text) > 0) {

		s = &update->message.text;
	}
	else {

		if(nxs_string_len(&update->message.caption) > 0) {

			s = &update->message.caption;
		}
		else {

			s = &update->message.text;
		}
	}

	if(messages_queue_extract_files(sess_ctx, update) == NXS_CHAT_SRV_E_ERR) {

		return NXS_CHAT_SRV_E_ERR;
	}

	if(nxs_chat_srv_u_db_sess_t_cat_message(sess_ctx, s) != NXS_CHAT_SRV_E_OK) {

		return NXS_CHAT_SRV_E_ERR;
	}

	return NXS_CHAT_SRV_E_OK;
}

/*
 * Returns:
 * * NXS_CHAT_SRV_E_TYPE	- if update type is not NXS_CHAT_SRV_E_TYPE
 * * NXS_CHAT_SRV_E_EXIST	- if no files exists in update
 * * NXS_CHAT_SRV_E_OK		- if one or more files exists in update
 * * NXS_CHAT_SRV_E_ERR		- on error
 */
static nxs_chat_srv_err_t messages_queue_extract_files(nxs_chat_srv_u_db_sess_t *sess_ctx, nxs_chat_srv_m_tlgrm_update_t *update)
{
	nxs_chat_srv_m_tlgrm_photo_size_t *p;
	nxs_chat_srv_err_t                 rc;

	if(nxs_chat_srv_c_tlgrm_update_get_type(update) != NXS_CHAT_SRV_M_TLGRM_UPDATE_TYPE_MESSAGE) {

		return NXS_CHAT_SRV_E_TYPE;
	}

	rc = NXS_CHAT_SRV_E_EXIST;

	if(nxs_array_count(&update->message.photo) > 0) {

		if((p = nxs_array_get(&update->message.photo, nxs_array_count(&update->message.photo) - 1)) != NULL) {

			rc = NXS_CHAT_SRV_E_OK;

			if(nxs_chat_srv_u_db_sess_add_file(sess_ctx, &p->file_id, NULL, NULL) != NXS_CHAT_SRV_E_OK) {

				return NXS_CHAT_SRV_E_ERR;
			}
		}
	}

	if(update->message.document._is_used == NXS_YES) {

		rc = NXS_CHAT_SRV_E_OK;

		if(nxs_chat_srv_u_db_sess_add_file(sess_ctx,
		                                   &update->message.document.file_id,
		                                   &update->message.document.file_name,
		                                   &update->message.document.mime_type) != NXS_CHAT_SRV_E_OK) {

			return NXS_CHAT_SRV_E_ERR;
		}
	}

	if(update->message.sticker._is_used == NXS_YES) {

		rc = NXS_CHAT_SRV_E_OK;

		if(nxs_chat_srv_u_db_sess_add_file(sess_ctx, &update->message.sticker.file_id, NULL, &_s_stickers_mime) !=
		   NXS_CHAT_SRV_E_OK) {

			return NXS_CHAT_SRV_E_ERR;
		}
	}

	if(update->message.voice._is_used == NXS_YES) {

		rc = NXS_CHAT_SRV_E_OK;

		if(nxs_chat_srv_u_db_sess_add_file(sess_ctx, &update->message.voice.file_id, NULL, &update->message.voice.mime_type) !=
		   NXS_CHAT_SRV_E_OK) {

			return NXS_CHAT_SRV_E_ERR;
		}
	}

	if(update->message.video._is_used == NXS_YES) {

		rc = NXS_CHAT_SRV_E_OK;

		if(nxs_chat_srv_u_db_sess_add_file(sess_ctx, &update->message.video.file_id, NULL, &update->message.video.mime_type) !=
		   NXS_CHAT_SRV_E_OK) {

			return NXS_CHAT_SRV_E_ERR;
		}
	}

	return rc;
}
