/* tools/test_host_mcc_hash.py's driver: port/android/host/host_mcc_hash.c's
fingerprints of a package folder and of one map, printed for the test to
check against Python's hashlib and zlib. */
#define HOST_MCC_HASH_TEST 1
#include "../../port/android/host/host_mcc_hash.c"

static void print(char const *label, unsigned char const digest[32], uint32_t crc)
{
	int i;

	printf("%s ", label);
	for (i = 0; i < 32; i++)
		printf("%02x", digest[i]);
	printf(" %08x\n", (unsigned)crc);
}

/* argv: data root, package folder, map, bitmaps ("-" none), sounds ("-" none), the map's checksum */
int main(int argc, char **argv)
{
	struct job job;
	struct known entry;
	uint32_t init;
	int type;

	if (argc != 7)
		return 2;
	pthread_once(&hash_once, hash_setup);
	printf("kind %s %s\n", sha_kind, crc_kind);
	init = (uint32_t)strtoul(argv[6], NULL, 16);
	/* one map, worked out (nothing known yet) */
	memset(&job, 0, sizeof(job));
	snprintf(job.data_root, sizeof(job.data_root), "%s", argv[1]);
	for (type = 0; type < 3; type++)
		if (strcmp(argv[3 + type], "-"))
			snprintf(job.paths[type], sizeof(job.paths[type]), "%s", argv[3 + type]);
	job.crc = init;
	job.state = JOB_RUNNING;
	job_main(&job);
	if (job.state != JOB_DONE)
		return 3;
	print("job", job.result.digest, job.result.crc);
	/* the package's, then each way of the map from what is kept */
	printf("package %d\n", host_mcc_hash_package(argv[1], argv[2], NULL, NULL, NULL));
	for (type = 0; type < 4; type++) {
		memset(&entry, 0, sizeof(entry));
		key_append(entry.key, sizeof(entry.key), argv[3]);
		if (type & 1)
			key_append(entry.key, sizeof(entry.key), argv[4]);
		if (type & 2)
			key_append(entry.key, sizeof(entry.key), argv[5]);
		if (!known_find(argv[1], entry.key, &entry))
			printf("way%d missing\n", type);
		else {
			char label[16];

			snprintf(label, sizeof(label), "way%d", type);
			print(label, entry.digest, crc_combine(init, entry.crc, entry.length));
		}
	}
	return 0;
}
