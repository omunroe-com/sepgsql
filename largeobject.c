/*
 * largeobject.c
 *
 * It provides access controls on binary large objects
 *
 * Author: KaiGai Kohei <kaigai@ak.jp.nec.com>
 *
 * Copyright (c) 2007 - 2010, NEC Corporation
 * Copyright (c) 2006 - 2007, KaiGai Kohei <kaigai@kaigai.gr.jp>
 */
#include "postgres.h"

#include "catalog/pg_largeobject.h"
#include "commands/seclabel.h"

#include "sepgsql.h"

/*
 * sepgsql_largeobject_post_create
 *
 * The post-creation hook of binary large object
 */
void
sepgsql_largeobject_post_create(Oid blobId)
{
	/* FIXME: Right now we assume database has fixed label */
	char		   *tcontext = "system_u:object_r:sepgsql_db_t:s0";
	char		   *ncontext;
	ObjectAddress	object = {
		.classId = LargeObjectRelationId,
		.objectId = blobId,
		.objectSubId = 0,
	};

	ncontext = sepgsql_compute_create(sepgsql_get_client_label(),
									  tcontext,
									  SEPG_CLASS_DB_BLOB);

	SetSecurityLabel(&object, SEPGSQL_LABEL_TAG, ncontext);
}
