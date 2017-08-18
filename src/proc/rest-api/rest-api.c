#include <nxs-core/nxs-core.h>

// clang-format off

/* Project core include */
#include <nxs-chat-srv-core.h>
#include <nxs-chat-srv-meta.h>
#include <nxs-chat-srv-collections.h>
#include <nxs-chat-srv-units.h>

#include <proc/rest-api/rest-api.h>
#include <proc/rest-api/ctx/rest-api-ctx.h>
#include <proc/rest-api/rest-api-subproc.h>

/* Definitions */



/* Project globals */
extern		nxs_process_t			process;
extern		nxs_chat_srv_cfg_t		nxs_chat_srv_cfg;

/* Module typedefs */



/* Module declarations */



/* Module internal (static) functions prototypes */

// clang-format on

static nxs_chat_srv_err_t nxs_chat_srv_p_rest_api_handlers_init(nxs_chat_srv_p_rest_api_ctx_t *p_ctx);
static nxs_chat_srv_err_t nxs_chat_srv_p_rest_api_exec(nxs_chat_srv_p_rest_api_ctx_t *p_ctx);
static nxs_rest_api_err_t nxs_chat_srv_p_rest_api_log_handler(nxs_rest_api_ctx_t *     rest_api_ctx,
                                                              nxs_rest_api_request_t * req,
                                                              nxs_rest_api_log_stage_t stage,
                                                              void *                   user_ctx);

// clang-format off

/* Module initializations */

static nxs_string_t _s_ra_handler_tlgrm			= nxs_string("/tlgrm");
static nxs_string_t _s_ra_handler_redmine		= nxs_string("/redmine");

static nxs_string_t _s_header_user_agent		= nxs_string("User-Agent");

/* Module global functions */

// clang-format on

nxs_chat_srv_err_t nxs_chat_srv_p_rest_api_runtime(void)
{
	nxs_chat_srv_p_rest_api_ctx_t p_ctx;
	nxs_chat_srv_err_t            rc;

	if(nxs_chat_srv_p_rest_api_ctx_init(&p_ctx) != NXS_CHAT_SRV_E_OK) {

		nxs_log_write_error(&process, "[%s]: can't start nxs-chat-srv", nxs_proc_get_name(&process));

		return NXS_CHAT_SRV_E_ERR;
	}

	rc = NXS_CHAT_SRV_E_OK;

	/*
	 * Start Rest API server
	 */

	if(nxs_chat_srv_p_rest_api_handlers_init(&p_ctx) != NXS_CHAT_SRV_E_OK) {

		nxs_log_write_error(&process, "[%s]: can't start nxs-chat-srv: handlers init error", nxs_proc_get_name(&process));

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

	if(nxs_chat_srv_p_rest_api_exec(&p_ctx) != NXS_CHAT_SRV_E_OK) {

		nxs_log_write_error(&process, "[%s]: nxs-chat-srv execution error", nxs_proc_get_name(&process));

		nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
	}

error:

	nxs_chat_srv_p_rest_api_ctx_free(&p_ctx);

	return rc;
}

/* Module internal (static) functions */

static nxs_chat_srv_err_t nxs_chat_srv_p_rest_api_handlers_init(nxs_chat_srv_p_rest_api_ctx_t *p_ctx)
{
	nxs_rest_api_ctx_t *api_ctx;

	if((api_ctx = nxs_chat_srv_p_rest_api_ctx_get_ra_conn(p_ctx)) == NULL) {

		return NXS_CHAT_SRV_E_ERR;
	}

	nxs_rest_api_handler_add_log(api_ctx, NULL, NULL, &nxs_chat_srv_p_rest_api_log_handler);

	nxs_rest_api_handler_add(
	        api_ctx, &_s_ra_handler_tlgrm, NXS_REST_API_COMMON_CMD_POST, NXS_YES, NULL, &nxs_chat_srv_p_rest_api_tlgrm_handler_post);
	nxs_rest_api_handler_add(api_ctx, &_s_ra_handler_redmine, NXS_REST_API_COMMON_CMD_POST, NXS_YES, NULL, NULL);

	return NXS_CHAT_SRV_E_OK;
}

static nxs_chat_srv_err_t nxs_chat_srv_p_rest_api_exec(nxs_chat_srv_p_rest_api_ctx_t *p_ctx)
{
	nxs_rest_api_ctx_t *api_ctx;
	nxs_rest_api_err_t  re;

	if((api_ctx = nxs_chat_srv_p_rest_api_ctx_get_ra_conn(p_ctx)) == NULL) {

		return NXS_CHAT_SRV_E_ERR;
	}

	while(1) {

		re = nxs_rest_api_process(api_ctx, 1, 0);

		if(re != NXS_REST_API_E_OK && re != NXS_REST_API_E_TIMEOUT) {

			return NXS_CHAT_SRV_E_ERR;
		}
	}

	return NXS_CHAT_SRV_E_OK;
}

static nxs_rest_api_err_t nxs_chat_srv_p_rest_api_log_handler(nxs_rest_api_ctx_t *     rest_api_ctx,
                                                              nxs_rest_api_request_t * req,
                                                              nxs_rest_api_log_stage_t stage,
                                                              void *                   user_ctx)
{
	nxs_string_t l, *ip, args, parts, *s, *v, *u;
	u_char *     method;
	u_int16_t    port;
	size_t       i;
	nxs_buf_t *  b;

	nxs_string_init(&l);
	nxs_string_init(&args);
	nxs_string_init(&parts);

	method = nxs_rest_api_common_type_to_text(nxs_rest_api_get_req_type(req));
	ip     = nxs_rest_api_get_req_peer_ip(req);
	port   = nxs_rest_api_get_req_peer_port(req);

	if(nxs_rest_api_get_req_part(req, 0) == NULL) {

		nxs_string_char_add_char_dyn(&parts, (u_char)'/');
	}
	else {

		for(i = 0; (s = nxs_rest_api_get_req_part(req, i)) != NULL; i++) {

			nxs_string_printf2_cat_dyn(&parts, "%r", s);
		}
	}

	if(nxs_rest_api_get_req_args_key(req, 0) != NULL) {

		nxs_string_char_add_char_dyn(&args, (u_char)'?');

		for(i = 0; (s = nxs_rest_api_get_req_args_key(req, i)) != NULL; i++) {

			v = nxs_rest_api_get_req_args_val(req, i);

			if(i > 0) {

				nxs_string_char_add_char_dyn(&args, (u_char)'&');
			}

			nxs_string_printf2_cat_dyn(&args, "%r=%r", s, v);
		}
	}

	u = nxs_rest_api_get_req_headers_find(req, &_s_header_user_agent);

	b = nxs_rest_api_get_out_buf(req);

	nxs_string_printf_dyn(&l,
	                      "connection accepted\t%r:%d\t\"%s %r%r\"\t%d\t%zu\t\"%r\"",
	                      ip,
	                      port,
	                      method,
	                      &parts,
	                      &args,
	                      nxs_rest_api_get_req_http_status(req),
	                      nxs_buf_get_len(b),
	                      u);

	nxs_log_write_info(&process, "%s", nxs_string_str(&l));

	nxs_string_free(&l);
	nxs_string_free(&args);
	nxs_string_free(&parts);

	return NXS_REST_API_E_OK;
}
