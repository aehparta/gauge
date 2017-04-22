/*
 * Generic Scope: Simple Capture Application
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#ifdef _WIN32
#include <conio.h>
#else
#endif
#include <stdio.h>
#include "dev.h"
#include "linkedlist.h"


uint8_t channel = 0x1;
/* trigger on rising(1) or falling(0) edge */
int trigger = 1;
int capture_continue = 1;
int sample_count = 1000;
uint64_t sample_interval_nanoseconds = 1000;
uint64_t sample_count_after_trigger = 1000000;

struct sample *sample_first = NULL;
struct sample *sample_last = NULL;

// int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
//                           void *(*start_routine) (void *), void *arg);

int capture(void *p_dev)
{
	int err = 0;
	struct device *dev = p_dev;
	struct sample *sample = NULL;

	dev_reset(dev);

	/* capture data continuosly until told to stop */
	while (capture_continue)
	{
		sample = dev_read(dev, sample_count);
		IF_ERR(!sample, -1, "failed to read sample from demo device");
		LL_APP(sample_first, sample_last, sample);
	}

out_err:
	capture_continue = -1;
	return err;
}

int devices_print(struct device *dev)
{
	if (dev->mode != 0)
	{
		return 0;
	}
	fprintf(stderr, "%s : %s\n", dev->serial, dev->name);
	return 0;
}

int main(int argc, char* argv[])
{
	uint8_t *p, *pend;
	struct sample *sample;
	int err = 0;
	uint64_t sample_count_done = 0;

	/* initialize logging and debugging */
	dd_log_init(NULL);

	/* initialize devices */
	dev_init();

	if (argc != 2)
	{
		fprintf(stderr, "Single argument needed, device serial from this list:\n");
		dev_list(devices_print);
		exit(1);
	}

	/* start capture */
	struct device *dev = dev_open(argv[1], sample_interval_nanoseconds);
	IF_ERR(!dev, -1, "failed to open device, maybe it does not support 1MS/s");
	thread_create(capture, dev);

	/* read samples */
	while (capture_continue > 0)
	{
		LL_GET(sample_first, sample_last, sample);
		if (!sample)
		{
			usleep(1);
			continue;
		}

		p = sample->data;
		pend = sample->data + sample->size;
		for ( ; p < pend && !(*p & 0x1); p++);
		if (p < pend)
		{
			break;
		}

		free(sample->data);
		free(sample);
	}

	if (capture_continue > 0)
	{
		struct sample *sample_temp;
		while (!sample_first && capture_continue > 0);
		sample_temp = sample_first;
		for (sample_count_done = 0; sample_count_done < sample_count_after_trigger; )
		{
			while (capture_continue > 0 && !sample_temp->next);
			if (capture_continue <= 0)
			{
				break;
			}
			sample_count_done += sample_temp->size;
			sample_temp = sample_temp->next;
		}
		capture_continue = 0;
		while (capture_continue >= 0);
		capture_continue = 0;
	}

	if (capture_continue == 0)
	{
		int current = 0;
		int last_state = 0x1;
		uint64_t t = 0;
		double div = (double)1000000000.0;

		for ( ; current < sample->size && !(sample->data[current] & 0x1); current++);
		printf("%lf,%d\n", (double)t / div, last_state);

		for (sample_count_done = 0; sample_count_done < sample_count_after_trigger; sample_count_done += sample->size)
		{
			for ( ; current < sample->size; current++)
			{
				t += sample_interval_nanoseconds;
				if ((sample->data[current] & 0x1) != last_state)
				{
					last_state ^= 0x1;
					printf("%lf,%d\n", (double)t / div, last_state);
				}
			}

			free(sample->data);
			free(sample);

			LL_GET(sample_first, sample_last, sample);
			if (!sample)
			{
				break;
			}

			current = 0;
		}
	}

	while (1)
	{
		LL_GET(sample_first, sample_last, sample);
		if (!sample)
		{
			break;
		}
		free(sample->data);
		free(sample);
	}

	dd_log_quit();

out_err:
	return err;
}


#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return main(0, NULL);
}
#endif


