/*
 * schema.c
 *
 * It provides access controls on schema objects
 *
 * Author: KaiGai Kohei <kaigai@ak.jp.nec.com>
 *
 * Copyright (c) 2007 - 2010, NEC Corporation
 * Copyright (c) 2006 - 2007, KaiGai Kohei <kaigai@kaigai.gr.jp>
 */
#include "postgres.h"

#include "catalog/pg_namespace.h"
#include "commands/seclabel.h"

#include "sepgsql.h"

/*
 * sepgsql_schema_post_create
 *
 * The post-creation hook of schema object
 */
void
sepgsql_schema_post_create(Oid namespaceId)
{
	/* FIXME: Right now we assume database has fixed label */
	char		   *tcontext = "system_u:object_r:sepgsql_db_t:s0";
	char		   *ncontext;
	ObjectAddress	object = {
		.classId = NamespaceRelationId,
		.objectId = namespaceId,
		.objectSubId = 0,
	};

	ncontext = sepgsql_compute_create(sepgsql_get_client_label(),
									  tcontext,
									  SEPG_CLASS_DB_SCHEMA);

	SetSecurityLabel(&object, SEPGSQL_LABEL_TAG, ncontext);
}
