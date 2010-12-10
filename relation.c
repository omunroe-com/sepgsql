/*
 * relation.c
 *
 * It provides access controls on relation objects
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
#include "catalog/pg_attribute.h"
#include "catalog/pg_class.h"
#include "catalog/pg_namespace.h"
#include "commands/seclabel.h"
#include "utils/fmgroids.h"
#include "utils/lsyscache.h"
#include "utils/tqual.h"

#include "sepgsql.h"

/*
 * sepgsql_attribute_post_create
 *
 * The post creation hook of attribute
 */
void
sepgsql_attribute_post_create(Oid relOid, int subId)
{
	ObjectAddress	object;
	char		   *ncontext;

	if (get_rel_relkind(relOid) != RELKIND_RELATION)
		return;

	object.classId = RelationRelationId;
	object.objectId = relOid;
	object.objectSubId = 0;
	ncontext = sepgsql_client_compute_create(&object,
											 SEPG_CLASS_DB_COLUMN);

	object.objectSubId = subId;
	SetSecurityLabel(&object, SEPGSQL_LABEL_TAG, ncontext);
}

/*
 * sepgsql_relation_post_create
 *
 * The post creation hook of relation/attribute
 */
void
sepgsql_relation_post_create(Oid relOid)
{
	Relation		rel;
	ScanKeyData		skey;
	SysScanDesc		sscan;
	HeapTuple		tuple;
	Form_pg_class	classForm;
	Oid				namespaceId;
	AttrNumber		natts;
	AttrNumber		attnum;
	char			relkind;
	bool			relhasoids;
	ObjectAddress	object;
	uint16			tclass;
	char		   *tcontext;
	char		   *acontext;

	rel = heap_open(RelationRelationId, AccessShareLock);

	ScanKeyInit(&skey,
				ObjectIdAttributeNumber,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(relOid));

	sscan = systable_beginscan(rel, ClassOidIndexId, true,
							   SnapshotSelf, 1, &skey);

	tuple = systable_getnext(sscan);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "catalog lookup failed for relation %u", relOid);

	classForm = (Form_pg_class) GETSTRUCT(tuple);

	namespaceId = classForm->relnamespace;
	natts = classForm->relnatts;
	relkind = classForm->relkind;
	relhasoids = classForm->relhasoids;

	systable_endscan(sscan);
	heap_close(rel, AccessShareLock);

	if (relkind == RELKIND_RELATION)
		tclass = SEPG_CLASS_DB_TABLE;
	else if (relkind == RELKIND_SEQUENCE)
		tclass = SEPG_CLASS_DB_SEQUENCE;
	else if (relkind == RELKIND_VIEW)
		tclass = SEPG_CLASS_DB_VIEW;
	else
		return;		/* no label assignment */

	object.classId = NamespaceRelationId;
	object.objectId = namespaceId;
	object.objectSubId = 0;
	tcontext = sepgsql_client_compute_create(&object, tclass);

	object.classId = RelationRelationId;
	object.objectId = relOid;
	object.objectSubId = 0;
	SetSecurityLabel(&object, SEPGSQL_LABEL_TAG, tcontext);

	if (relkind != RELKIND_RELATION)
		return;		/* no label assignment on attributes */

	/*
	 * NOTE: the parent relation is not visible yet
	 */
	acontext = sepgsql_compute_create(sepgsql_get_client_label(),
									  tcontext,
									  SEPG_CLASS_DB_COLUMN);
	for (attnum = FirstLowInvalidHeapAttributeNumber + 1;
		 attnum <= natts;
		 attnum++)
	{
		if (attnum == InvalidAttrNumber)
			continue;

		if (attnum == ObjectIdAttributeNumber && !relhasoids)
			continue;

		object.classId = RelationRelationId;
		object.objectId = relOid;
		object.objectSubId = attnum;
		SetSecurityLabel(&object, SEPGSQL_LABEL_TAG, acontext);
	}
}
