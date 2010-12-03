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

sepgsql_relation_post_create(objectId, subId);






/*****/
static List *
sepgsql_relation_create(const char *relName,
						Oid namespaceId,
						Oid tablespaceId,
						char relkind,
						TupleDesc tupdesc,
						bool createAs)
{
	SecLabelItem   *sl;
	char		   *tcontext = "system_u:object_r:sepgsql_table_t:s0";
	uint16			tclass;
	uint32			required;
	char			auname[NAMEDATALEN * 2 + 10];
	int				index;
	List		   *seclabels = NIL;

	if (relkind == RELKIND_RELATION)
		tclass = SEPG_CLASS_DB_TABLE;
	else if (relkind == RELKIND_SEQUENCE)
		tclass = SEPG_CLASS_DB_SEQUENCE;
	else if (relkind == RELKIND_VIEW)
		tclass = SEPG_CLASS_DB_TUPLE;
	else
		return NIL;

	required = SEPG_DB_TABLE__CREATE;
	if (createAs)
		required |= SEPG_DB_TABLE__INSERT;

	/* permission on the relation */
	sepgsql_compute_perms(sepgsql_get_client_label(),
						  tcontext,
						  tclass,
						  required,
						  relName,
						  true);

	sl = palloc0(sizeof(SecLabelItem));
	sl->object.classId = RelationRelationId;
	sl->object.objectId = InvalidOid;	/* to be assigned later */
	sl->object.objectSubId = 0;
	sl->tag = SEPGSQL_LABEL_TAG;
	sl->seclabel = tcontext;

	seclabels = list_make1(sl);

	if (relkind != RELKIND_RELATION)
		return seclabels;

	/* permission on the columns */
	for (index = FirstLowInvalidHeapAttributeNumber + 1;
		 index < tupdesc->natts;
		 index++)
	{
		Form_pg_attribute	attr;

		if (index == ObjectIdAttributeNumber && !tupdesc->tdhasoid)
			continue;

		if (index < 0)
			attr = SystemAttributeDefinition(index, tupdesc->tdhasoid);
		else
			attr = tupdesc->attrs[index];

		required = SEPG_DB_COLUMN__CREATE;
		if (createAs && index >= 0)
			required |= SEPG_DB_COLUMN__INSERT;

		snprintf(auname, sizeof(auname), "%s.%s", relName, NameStr(attr->attname));

		sepgsql_compute_perms(sepgsql_get_client_label(),
							  tcontext,
							  tclass,
							  required,
							  auname,
							  true);

		sl = palloc0(sizeof(SecLabelItem));
		sl->object.classId = RelationRelationId;
		sl->object.objectId = InvalidOid;		/* to be assigned later */
		sl->object.objectSubId = attr->attnum;
		sl->tag = SEPGSQL_LABEL_TAG;
		sl->seclabel = tcontext;
		seclabels = lappend(seclabels, sl);
	}
	return seclabels;
}

