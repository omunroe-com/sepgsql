/*
 * proc.c
 *
 * It provides access controls on procedure objects
 *
 * Author: KaiGai Kohei <kaigai@ak.jp.nec.com>
 *
 * Copyright (c) 2007 - 2010, NEC Corporation
 * Copyright (c) 2006 - 2007, KaiGai Kohei <kaigai@kaigai.gr.jp>
 */
#include "postgres.h"

#include "access/genam.h"
#include "access/heapam.h"
#include "access/sysattr.h"
#include "catalog/indexing.h"
#include "catalog/pg_namespace.h"
#include "catalog/pg_proc.h"
#include "commands/seclabel.h"
#include "utils/fmgroids.h"
#include "utils/tqual.h"

#include "sepgsql.h"

/*
 * sepgsql_proc_post_create
 *
 * The post creation hook of procedures
 */
void
sepgsql_proc_post_create(Oid functionId)
{
	Relation		rel;
	ScanKeyData		skey;
	SysScanDesc		sscan;
	HeapTuple		tuple;
	Oid				namespaceId;
	ObjectAddress	object;
	char		   *ncontext;

	rel = heap_open(ProcedureRelationId, AccessShareLock);

	ScanKeyInit(&skey,
				ObjectIdAttributeNumber,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(functionId));

	sscan = systable_beginscan(rel, ProcedureOidIndexId, true,
							   SnapshotSelf, 1, &skey);

	tuple = systable_getnext(sscan);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "catalog lookup failed for proc %u", functionId);

	namespaceId = ((Form_pg_proc) GETSTRUCT(tuple))->pronamespace;

	systable_endscan(sscan);
	heap_close(rel, AccessShareLock);

	object.classId = NamespaceRelationId;
	object.objectId = namespaceId;
	object.objectSubId = 0;
	ncontext = sepgsql_client_compute_create(&object,
											 SEPG_CLASS_DB_PROCEDURE);

	object.classId = ProcedureRelationId;
	object.objectId = functionId;
	object.objectSubId = 0;
	SetSecurityLabel(&object, SEPGSQL_LABEL_TAG, ncontext);
}
