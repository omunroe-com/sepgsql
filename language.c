/*
 * language.c
 *
 * It provides access controls on procedural language objects
 *
 * Author: KaiGai Kohei <kaigai@ak.jp.nec.com>
 *
 * Copyright (c) 2007 - 2010, NEC Corporation
 * Copyright (c) 2006 - 2007, KaiGai Kohei <kaigai@kaigai.gr.jp>
 */
#include "postgres.h"

#include "catalog/pg_language.h"
#include "commands/seclabel.h"

#include "sepgsql.h"

/*
 * sepgsql_language_post_create
 *
 * The post-creation hook of schema object
 */
void
sepgsql_language_post_create(Oid languageId)
{
	/* FIXME: Right now we assume database has fixed label */
	char		   *tcontext = "system_u:object_r:sepgsql_db_t:s0";
	char		   *ncontext;
	ObjectAddress	object = {
		.classId = LanguageRelationId,
		.objectId = languageId,
		.objectSubId = 0,
	};

	ncontext = sepgsql_compute_create(sepgsql_get_client_label(),
									  tcontext,
									  SEPG_CLASS_DB_LANGUAGE);

	SetSecurityLabel(&object, SEPGSQL_LABEL_TAG, ncontext);
}
