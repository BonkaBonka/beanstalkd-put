#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "beanstalk.h"

#define MAX_JOB_SIZE "max-job-size: "
#define MAX_JOB_SIZE_LEN strlen(MAX_JOB_SIZE)

char *tube_name = "default";
char *server_host = "localhost";
int server_port = 11300;
// Priority 0 is max, UINT32_MAX is lowest
// See: https://github.com/kr/beanstalkd/blob/v1.3/doc/protocol.txt#l135
uint32_t job_priority = UINT32_MAX;
uint32_t job_delay = 0;
uint32_t job_ttr = 120;

void display_help(char *prog)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  %s [options] <job body>\n\n", basename(prog));
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -t, --tube      <tube>      beantsalk tube to put the job into (%s)\n", tube_name);
	fprintf(stderr, "  -s, --server    <server>    beanstalk server hostname (%s)\n", server_host);
	fprintf(stderr, "  -p, --port      <port>      beanstalk server port number (%u)\n", server_port);
	fprintf(stderr, "  -P, --priority  <priority>  job priority, 0=max (%u)\n", job_priority);
	fprintf(stderr, "  -D, --delay     <delay>     seconds before the job becomes available (%u)\n", job_delay);
	fprintf(stderr, "  -T, --ttr       <ttr>       seconds to give the job processor (%u)\n", job_ttr);
	fprintf(stderr, "  -h, --help                  display this help\n\n");
	fprintf(stderr, "If \"job body\" is \"-\", then the body content will be read from stdin.\n\n");
}

int process_args(int argc, char **argv)
{
	static struct option long_options[] = {
		{ "tube",     required_argument, 0, 't' },
		{ "server",   required_argument, 0, 's' },
		{ "port",     required_argument, 0, 'p' },
		{ "priority", required_argument, 0, 'P' },
		{ "delay",    required_argument, 0, 'D' },
		{ "ttr",      required_argument, 0, 'T' },
		{ "help",     no_argument,       0, 'h' },
		{ NULL,       0,                 0, 0   }
	};

	int c;

	while ((c = getopt_long(argc, argv, "t:s:p:P:D:T:h", long_options, NULL)) != -1) {
		switch (c) {
			case 't':
				tube_name = optarg;
				break;
			case 's':
				server_host = optarg;
				break;
			case 'p':
				server_port = atoi(optarg);
				break;
			case 'P':
				job_priority = atoi(optarg);
				break;
			case 'D':
				job_delay = atoi(optarg);
				break;
			case 'T':
				job_ttr = atoi(optarg);
				break;
			default:
				display_help(argv[0]);
				return -1;
		}
	}

	if (optind >= argc || optind + 1 < argc) {
		display_help(argv[0]);
		return -1;
	}

	return optind;
}

int main(int argc, char **argv) {
	unsigned char *yaml;
	unsigned char *payload;
	unsigned char *server_job_size;
	uint64_t max_payload_size = 0;
	uint64_t payload_size = 0;

	int c = process_args(argc, argv);
	if (c < 1) {
		return 1;
	}

	int socket = bs_connect(server_host, server_port);
	if (socket == BS_STATUS_FAIL) {
		fprintf(stderr, "Unable to connect to beanstalkd on %s:%d\n", server_host, server_port);
		return 2;
	}

	if (bs_use(socket, tube_name) == BS_STATUS_FAIL) {
		fprintf(stderr, "Unable to use beanstalk tube %s\n", tube_name);
		bs_disconnect(socket);
		return 2;
	}

	if (bs_stats(socket, (char **)&yaml) == BS_STATUS_FAIL) {
		fprintf(stderr, "Unable to get beanstalk stats\n");
		bs_disconnect(socket);
		return 2;
	}

	if ((server_job_size = (unsigned char *)strstr((char *)yaml, MAX_JOB_SIZE)) == NULL) {
		fprintf(stderr, "Unable to determine max job size\n");
		free(yaml);
		bs_disconnect(socket);
		return 2;
	}

	max_payload_size = atoi((char *)server_job_size + MAX_JOB_SIZE_LEN);

	free(yaml);

	// Add an extra byte to ensure null termination and to detect stdin overflow
	payload = (unsigned char *)calloc(max_payload_size + 1, 1);
	payload_size = strlen(argv[c]);

	if (payload_size > max_payload_size) {
		fprintf(stderr, "Payload too large to send through beanstalkd %ld > %ld\n", payload_size, max_payload_size);
		free(payload);
		bs_disconnect(socket);
		return 3;
	}

	memcpy(payload, argv[c], payload_size);

	if (payload_size == 1 && payload[0] == '-') {
		payload_size = 0;

		do {
			c = read(STDIN_FILENO, payload + payload_size, (1 + max_payload_size) - payload_size);
			if (c < 0) {
				perror("reading from stdin");
				free(payload);
				bs_disconnect(socket);
				return 3;
			}
			payload_size += c;
		} while (c > 0);

		if (payload_size > max_payload_size) {
			fprintf(stderr, "Payload too large to send through beanstalkd %ld > %ld\n", payload_size, max_payload_size);
			free(payload);
			bs_disconnect(socket);
			return 3;
		} else if (payload_size == 0) {
			fprintf(stderr, "No payload read from stdin\n");
			free(payload);
			bs_disconnect(socket);
			return 3;
		}
	}

	int64_t id = bs_put(socket, job_priority, job_delay, job_ttr, (char *)payload, payload_size);
	if (id == 0) {
		fprintf(stderr, "Unable to put message into tube %s\n", payload);
		free(payload);
		bs_disconnect(socket);
		return 3;
	}

	free(payload);

	printf("%ld\n", id);

	bs_disconnect(socket);

	return 0;
}
