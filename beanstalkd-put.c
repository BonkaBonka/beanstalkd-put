#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "beanstalk.h"

#define MAX_JOB_SIZE_KEY "max-job-size: "

char *tube_name = "default";
char *server_host = "localhost";
int server_port = 11300;
// Priority 0 is max, UINT32_MAX is lowest
// See: https://github.com/kr/beanstalkd/blob/v1.3/doc/protocol.txt#l135
uint32_t job_priority = UINT32_MAX;
uint32_t job_delay = 0;
uint32_t job_ttr = 120;
int quiet = 0;

void display_help(char *prog)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  %s [options] <job body>\n\n", basename(prog));
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -t, --tube      <tube>      beanstalk tube to put the job into (%s)\n", tube_name);
	fprintf(stderr, "  -s, --server    <server>    beanstalk server hostname (%s)\n", server_host);
	fprintf(stderr, "  -p, --port      <port>      beanstalk server port number (%u)\n", server_port);
	fprintf(stderr, "  -P, --priority  <priority>  job priority, 0=max (%u)\n", job_priority);
	fprintf(stderr, "  -D, --delay     <delay>     seconds before the job becomes available (%u)\n", job_delay);
	fprintf(stderr, "  -T, --ttr       <ttr>       seconds to give the job processor (%u)\n", job_ttr);
	fprintf(stderr, "  -q, --quiet                 do not output job ID\n");
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
		{ "quiet",    no_argument,       0, 'q' },
		{ "help",     no_argument,       0, 'h' },
		{ NULL,       0,                 0, 0   }
	};

	int c;

	while ((c = getopt_long(argc, argv, "t:s:p:P:D:T:qh", long_options, NULL)) != -1) {
		switch (c) {
			case 't':
				tube_name = optarg;
				break;
			case 's':
				server_host = optarg;
				break;
			case 'p':
				if (sscanf(optarg, "%d", &server_port) != 1) {
					if (errno != 0) {
						perror("sscanf: server port");
					} else {
						fprintf(stderr, "Non-numeric characters in server port\n");
					}
					return -1;
				}
				break;
			case 'P':
				if (sscanf(optarg, "%"SCNu32, &job_priority) != 1) {
					if (errno != 0) {
						perror("sscanf: job priority");
					} else {
						fprintf(stderr, "Non-numeric characters in job priority\n");
					}
					return -1;
				}
				break;
			case 'D':
				if (sscanf(optarg, "%"SCNu32, &job_delay) != 1) {
					if (errno != 0) {
						perror("sscanf: job delay");
					} else {
						fprintf(stderr, "Non-numeric characters in job delay\n");
					}
					return -1;
				}
				break;
			case 'T':
				if (sscanf(optarg, "%"SCNu32, &job_ttr) != 1) {
					if (errno != 0) {
						perror("sscanf: job ttr");
					} else {
						fprintf(stderr, "Non-numeric characters in job ttr\n");
					}
					return -1;
				}
				break;
			case 'q':
				quiet = 1;
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
	unsigned char *job;
	unsigned char *max_job_size_key;
	uint64_t max_job_size = 0;
	uint64_t job_size = 0;

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
		fprintf(stderr, "Unable to use beanstalkd tube %s\n", tube_name);
		bs_disconnect(socket);
		return 2;
	}

	if (bs_stats(socket, (char **)&yaml) == BS_STATUS_FAIL) {
		fprintf(stderr, "Unable to get beanstalkd stats\n");
		bs_disconnect(socket);
		return 2;
	}

	if ((max_job_size_key = (unsigned char *)strstr((char *)yaml, MAX_JOB_SIZE_KEY)) == NULL) {
		fprintf(stderr, "Unable to determine beanstalkd's max job size\n");
		free(yaml);
		bs_disconnect(socket);
		return 2;
	}

	if (sscanf((char *)max_job_size_key + strlen(MAX_JOB_SIZE_KEY), "%"SCNu64, &max_job_size) != 1) {
		if (errno != 0) {
			perror("sscanf: max job size");
		} else {
			fprintf(stderr, "Non-numeric characters in max job size\n");
		}
		free(yaml);
		bs_disconnect(socket);
		return 2;
	}

	free(yaml);

	job_size = strlen(argv[c]);
	if (job_size > max_job_size) {
		fprintf(stderr, "Job too large to send through beanstalkd %ld > %ld\n", job_size, max_job_size);
		bs_disconnect(socket);
		return 3;
	}

	// Add an extra byte to ensure null termination and to detect stdin overflow
	job = (unsigned char *)calloc(max_job_size + 1, 1);
	memcpy(job, argv[c], job_size);

	if (job_size == 1 && job[0] == '-') {
		job_size = 0;

		do {
			c = read(STDIN_FILENO, job + job_size, (1 + max_job_size) - job_size);
			if (c < 0) {
				perror("reading from stdin");
				free(job);
				bs_disconnect(socket);
				return 3;
			}
			job_size += c;
		} while (c > 0);

		if (job_size > max_job_size) {
			fprintf(stderr, "Job too large to send through beanstalkd %ld > %ld\n", job_size, max_job_size);
			free(job);
			bs_disconnect(socket);
			return 3;
		} else if (job_size == 0) {
			fprintf(stderr, "No job read from stdin\n");
			free(job);
			bs_disconnect(socket);
			return 3;
		}
	}

	int64_t id = bs_put(socket, job_priority, job_delay, job_ttr, (char *)job, job_size);
	if (id == 0) {
		fprintf(stderr, "Error putting job into tube\n");
		free(job);
		bs_disconnect(socket);
		return 3;
	}

	free(job);

	if (quiet == 0) {
		printf("%ld\n", id);
	}

	bs_disconnect(socket);

	return 0;
}
