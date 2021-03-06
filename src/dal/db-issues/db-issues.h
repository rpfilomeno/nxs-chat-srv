#ifndef _INCLUDE_NXS_CHAT_SRV_D_DB_ISSUES_H
#define _INCLUDE_NXS_CHAT_SRV_D_DB_ISSUES_H

// clang-format off

/* Structs declarations */

struct nxs_chat_srv_d_db_issues_s;

/* Prototypes */

nxs_chat_srv_d_db_issues_t	*nxs_chat_srv_d_db_issues_init		(void);
nxs_chat_srv_d_db_issues_t	*nxs_chat_srv_d_db_issues_free		(nxs_chat_srv_d_db_issues_t *d_ctx);

nxs_chat_srv_err_t		nxs_chat_srv_d_db_issues_get		(nxs_chat_srv_d_db_issues_t *d_ctx, size_t tlgrm_chat_id, size_t tlgrm_message_id, size_t *rdmn_issue_id);
nxs_chat_srv_err_t		nxs_chat_srv_d_db_issues_put		(nxs_chat_srv_d_db_issues_t *d_ctx, size_t tlgrm_chat_id, size_t tlgrm_message_id, size_t rdmn_issue_id);

#endif /* _INCLUDE_NXS_CHAT_SRV_D_DB_ISSUES_H */
