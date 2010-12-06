#include <stdio.h>

#include "blobmsg.h"

static const char *indent_str = "\t\t\t\t\t\t\t\t\t\t\t\t\t";

#define indent_printf(indent, ...) do { \
	if (indent > 0) \
		fwrite(indent_str, indent, 1, stderr); \
	fprintf(stderr, __VA_ARGS__); \
} while(0)

static void dump_attr_data(void *data, int len, int type, int indent, int next_indent);

static void
dump_array(struct blob_attr *head, int len, int indent)
{
	struct blob_attr *attr;

	indent_printf(indent, "{\n");
	__blob_for_each_attr(attr, head, len) {
		dump_attr_data(blob_data(attr), blob_len(attr), blob_id(attr), indent + 1, indent + 1);
	}
	indent_printf(indent, "}\n");
}

static void
dump_table(struct blob_attr *head, int len, int indent)
{
	struct blob_attr *attr, *last_attr;
	struct blobmsg_hdr *hdr;

	indent_printf(indent, "{\n");
	__blob_for_each_attr(attr, head, len) {
		hdr = blob_data(attr);
		indent_printf(indent + 1, "%s : ", hdr->name);
		dump_attr_data(blobmsg_data(attr), blobmsg_data_len(attr), blob_id(attr), 0, indent + 1);
		last_attr = attr;
	}
	indent_printf(indent, "}\n");
}

static void dump_attr_data(void *data, int len, int type, int indent, int next_indent)
{
	switch(type) {
	case BLOBMSG_TYPE_STRING:
		indent_printf(indent, "%s\n", (char *) data);
		break;
	case BLOBMSG_TYPE_INT8:
		indent_printf(indent, "%d\n", *(uint8_t *)data);
		break;
	case BLOBMSG_TYPE_INT16:
		indent_printf(indent, "%d\n", *(uint16_t *)data);
		break;
	case BLOBMSG_TYPE_INT32:
		indent_printf(indent, "%d\n", *(uint32_t *)data);
		break;
	case BLOBMSG_TYPE_INT64:
		indent_printf(indent, "%lld\n", *(uint64_t *)data);
		break;
	case BLOBMSG_TYPE_TABLE:
		if (!indent)
			indent_printf(indent, "\n");
		dump_table(data, len, next_indent);
		break;
	case BLOBMSG_TYPE_ARRAY:
		if (!indent)
			indent_printf(indent, "\n");
		dump_array(data, len, next_indent);
		break;
	}
}

enum {
	FOO_MESSAGE,
	FOO_LIST,
	FOO_TESTDATA
};

static const struct blobmsg_policy pol[] = {
	[FOO_MESSAGE] = {
		.name = "message",
		.type = BLOBMSG_TYPE_STRING,
	},
	[FOO_LIST] = {
		.name = "list",
		.type = BLOBMSG_TYPE_ARRAY,
	},
	[FOO_TESTDATA] = {
		.name = "testdata",
		.type = BLOBMSG_TYPE_TABLE,
	},
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static void dump_message(struct blob_buf *buf)
{
	struct blob_attr *tb[ARRAY_SIZE(pol)];

	if (blobmsg_parse(pol, ARRAY_SIZE(pol), tb, blob_data(buf->head), blob_len(buf->head)) != 0) {
		fprintf(stderr, "Parse failed\n");
		return;
	}
	if (tb[FOO_MESSAGE])
		fprintf(stderr, "Message: %s\n", (char *) blobmsg_data(tb[FOO_MESSAGE]));

	if (tb[FOO_LIST]) {
		fprintf(stderr, "List: ");
		dump_array(blobmsg_data(tb[FOO_LIST]), blob_len(tb[FOO_LIST]), 0);
	}
	if (tb[FOO_TESTDATA]) {
		fprintf(stderr, "Testdata: ");
		dump_table(blobmsg_data(tb[FOO_TESTDATA]), blob_len(tb[FOO_TESTDATA]), 0);
	}
}

static void
fill_message(struct blob_buf *buf)
{
	void *tbl;
#if 0
	int i;
#endif

	blobmsg_add_string(buf, "message", "Hello, world!");

	tbl = blobmsg_open_array(buf, "list");
	blobmsg_add_u32(buf, NULL, 0);
	blobmsg_add_u32(buf, NULL, 1);
	blobmsg_add_u32(buf, NULL, 2);
	blobmsg_close_table(buf, tbl);

	tbl = blobmsg_open_table(buf, "testdata");
	blobmsg_add_u32(buf, "hello", 1);
	blobmsg_add_string(buf, "world", "2");
	blobmsg_close_table(buf, tbl);

#if 0
	for (i = 0; i < buf->buflen; i++) {
		if (i % 0x10 == 0)
			fprintf(stderr, "\n");
		fprintf(stderr, "%02x ", ((uint8_t *) buf->buf)[i]);
	}
	fprintf(stderr, "\n");
#endif
}

int main(int argc, char **argv)
{
	static struct blob_buf buf;

	blobmsg_buf_init(&buf);
	fill_message(&buf);
	dump_message(&buf);

	if (buf.buf)
		free(buf.buf);

	return 0;
}