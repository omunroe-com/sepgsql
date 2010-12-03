/*
 * language.c
 *
 * It provides access controls on procedural language objects.
 * (pg_language)
 *
 * Author: KaiGai Kohei <kaigai@ak.jp.nec.com>
 *
 * Copyright (c) 2007 - 2010, NEC Corporation
 * Copyright (c) 2006 - 2007, KaiGai Kohei <kaigai@kaigai.gr.jp>
 */
#include "postgres.h"

#include "catalog/indexing.h"
#include "catalog/pg_language.h"

#include "sepgsql.h"


static HeapTuple
lookup_language_tuple(Oid langOid)
{
}

/*
 * sepgsql_language_post_create
 * 
 * The post-creation hook of procedural language object.
 */
void
sepgsql_language_post_create(Oid langOid)
{
	Form_pg_language	formLang;
	Relation	rel;
	ScanKeyData	skey[1];
	SysScanDesc	scan;
	HeapTuple	tuple;

	rel = heap_open(LanguageRelationId, AccessShareLock);

	ScanKeyInit(&skey[0],
				ObjectIdAttributeNumber,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(langOid));

	scan = systable_beginscan(rel, LanguageOidIndexId,
							  SnapshotSelf, 1, skey);
	tuple = systable_getnext(scan);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "Heap lookup failed for language %u", langOid);

	formLang = (Form_pg_language) GETSTRUCT(tuple);









	systable_endscan(scan);

	heap_close(rel, AccessShareLock);
}
