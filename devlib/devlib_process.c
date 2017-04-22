/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#include "devlib_process.h"

/* internal functions */
int i_devlib_process_reader_thread(struct device *dev);
int i_devlib_process_filter_thread(struct device *dev);
int i_devlib_process_trigger_thread(struct device *dev);
int i_devlib_process_writer_thread(struct device *dev);
int i_devlib_process_finish(struct devlib_process *process, struct device *dev);

struct devlib_process *devlib_process_first = NULL;
struct devlib_process *devlib_process_last = NULL;
lock_rw_t devlib_process_lock;


int devlib_process_init()
{
	lock_rw_init(&devlib_process_lock);
	devlib_process_first = NULL;
	devlib_process_last = NULL;
	return 0;
}

void devlib_process_quit()
{
	/** @todo devlib_process_quit() */
	lock_rw_destroy(&devlib_process_lock);
}

struct devlib_process *devlib_process_create(const char *lib_name, const char *serial)
{
	int err = 0;
	struct devlib_process *process = NULL;
	struct device *source = NULL;

	/* check that primary device is the right type */
	IF_ERR(dev->type != DEV_TYPE_IO_READ, -1, "primary process device must be of type DEV_TYPE_IO_READ");
	/* open device */
	source = devlib_open(lib_name, serial);
	IF_ERR(!source, -1, "unable to open source device: %s", devlib_serial(dev));
	/* allocate and setup process side */
	DD_SALLOC(process);
	process->source = source;
	process->execute = 1;
	process->pause = 1;
	process->samples_per_buffer = 1000;
	mutex_init(&process->buffer_done_mutex);
	/* setup device side */
	source->process.paused = 0;
	source->process.process = process;
	source->process.buffers_pre_trigger = 2;
	source->process.buffers_total = 0;
	mutex_init(&source->process.buffer_mutex);
	/* add process to process list */
	lock_rw_write(&devlib_process_lock);
	LL_APP(devlib_process_first, devlib_process_last, process);
	lock_rw_unlock(&devlib_process_lock);
	/* start main process thread */
	source->process.thread = thread_create((int (*)(void *))i_devlib_process_reader_thread, source);
	/* wait thread to start */
	while (!source->process.paused)
	{
		os_sleep(0.001);
	}

out_err:
	if (err)
	{
		if (process)
		{
			free(process);
		}
		return NULL;
	}
	return process;
}

struct device *devlib_process_source(struct devlib_process *process)
{
	return process->source;
}

int devlib_process_insert(struct devlib_process *process, int index, const char *serial)
{
	int err = 0;
	struct device *dev = NULL;

	/* find device by serial */
	dev = devlib_find(serial);
	IF_ERR(!dev, -1, "unable to find device %s", serial);
	/* cannot insert source devices */
	IF_ERR(dev->type == DEV_TYPE_IO_READ, -1, "secondary process device cannot be of type DEV_TYPE_IO_READ");
	/* insert:
	 * only read lock so that the process cannot be removed etc,
	 * the process list itself is not written here
	 */
	lock_rw_read(&devlib_process_lock);
	LL_APP(process->dev_first, process->dev_last, dev);
	lock_rw_unlock(&devlib_process_lock);

out_err:
	return err;
}

int devlib_process_remove(struct devlib_process *process, int index)
{
	return 0;
}

int devlib_process_samples_per_buffer(struct devlib_process *process, int count)
{
	/* only set if count is over zero */
	if (count > 0)
	{
		if (!process)
		{
			/* if NULL was given as process, set for all processes */
			for (process = devlib_process_first; process; process = process->next)
			{
				devlib_process_samples_per_buffer(process, count);
			}
			return count;
		}
		else
		{
			process->samples_per_buffer = count;
		}
	}
	/* return current count */
	return process->samples_per_buffer;
}

int devlib_process_start(struct devlib_process *process)
{
	/* if NULL, start all processes */
	if (!process)
	{
		for (process = devlib_process_first; process; process = process->next)
		{
			devlib_process_start(process);
		}
		return 0;
	}
	/* start single process */
	if (process->pause != 1)
	{
		ERROR_MSG("process is not in paused state, will not (re)start process");
		return -1;
	}
	devlib_reset(process->source);
	process->done = 0;
	process->triggered = 0;
	process->pause = 0;
	/** @todo wait until all threads are running */
	return 0;
}

int devlib_process_sampling(struct devlib_process *process)
{
	/* if NULL, check all processes */
	if (!process)
	{
		int n = 0;
		for (process = devlib_process_first; process; process = process->next)
		{
			n += devlib_process_sampling(process);
		}
		return n;
	}
	/* check single process */
	return process->pause ? 0 : 1;
}

int devlib_process_pause(struct devlib_process *process)
{
	/* if NULL, pause all processes */
	if (!process)
	{
		for (process = devlib_process_first; process; process = process->next)
		{
			devlib_process_pause(process);
		}
		return 0;
	}
	/* pause single process */
	if (process->pause != 0)
	{
		ERROR_MSG("process is not in running state, will not (re)pause process");
		return -1;
	}
	process->pause = 1;
	/** @todo wait until all threads are paused */
	return 0;
}

int devlib_process_trigger(struct devlib_process *process)
{
	/* trigger all if NULL given */
	if (!process)
	{
		for (process = devlib_process_first; process; process = process->next)
		{
			devlib_process_trigger(process);
		}
		return 0;
	}
	/* single trigger */
	if (process->pause != 0)
	{
		ERROR_MSG("process is not in running state, will not trigger process");
		return -1;
	}
	process->triggered = 1;
	return 0;
}

int devlib_process_done(struct devlib_process *process)
{
	/* check all if NULL given */
	if (!process)
	{
		int n = 0, i = 0;
		for (process = devlib_process_first; process; process = process->next)
		{
			n += devlib_process_done(process);
			i++;
		}
		return i == n ? 1 : 0;
	}
	return process->done;
}

struct devlib_process_buffer *devlib_process_buffer_get(struct devlib_process *process)
{
	struct devlib_process_buffer *buffer = NULL;
	if (process->source->process.buffers_total == 0)
	{
		/** @todo get last device in processing chain */
		struct device *dev = process->source;
		/* get current buffers when using continuous reading */
		mutex_lock(&dev->process.buffer_mutex);
		buffer = dev->process.buffer_first;
		dev->process.buffer_first = NULL;
		dev->process.buffer_last = NULL;
		dev->process.buffers_count = 0;
		mutex_unlock(&dev->process.buffer_mutex);
	}
	else
	{
		/* get buffers when all data has been read after triggering */
		mutex_lock(&process->buffer_done_mutex);
		buffer = process->buffer_done;
		process->buffer_done = NULL;
		mutex_unlock(&process->buffer_done_mutex);
	}
	return buffer;
}

void devlib_process_buffer_free(struct devlib_process_buffer *buffer)
{
	while (buffer)
	{
		struct devlib_process_buffer *buffer_tmp = buffer->next;
		free(buffer->data);
		free(buffer);
		buffer = buffer_tmp;
	}
}

void devlib_process_set_userdata(struct devlib_process *process, void *data)
{
	process->userdata = data;
}

void devlib_process_set_callback_data(struct devlib_process *process, int (*callback)(struct devlib_process *, void *))
{
	process->callback_data = callback;
}

void devlib_process_set_callback_done(struct devlib_process *process, int (*callback)(struct devlib_process *, void *))
{
	process->callback_done = callback;
}

int i_devlib_process_reader_thread(struct device *dev)
{
	struct devlib_process *process = dev->process.process;
	int err;
	int bytes_per_sample;

	/* calculate single sample data size */
	switch (process->source->io.format)
	{
	case DEV_FORMAT_DIGITAL:
		bytes_per_sample = (int)ceil((double)process->source->io.channels / 8.0);
		break;
	case DEV_FORMAT_INT8:
	case DEV_FORMAT_UINT8:
		bytes_per_sample = 1 * process->source->io.channels;
		break;
	case DEV_FORMAT_INT16:
	case DEV_FORMAT_UINT16:
		bytes_per_sample = 2 * process->source->io.channels;
		break;
	case DEV_FORMAT_INT24:
	case DEV_FORMAT_UINT24:
		bytes_per_sample = 3 * process->source->io.channels;
		break;
	case DEV_FORMAT_INT32:
	case DEV_FORMAT_UINT32:
		bytes_per_sample = 4 * process->source->io.channels;
		break;
	}

	while (process->execute)
	{
		/* if waiting is what should be done */
		if (process->pause)
		{
			/* also one way variable */
			dev->process.paused = 1;
			os_sleep(0.001);
			continue;
		}
		if (dev->process.paused)
		{
			/* clear old buffers */
			struct devlib_process_buffer *buffer;
			mutex_lock(&dev->process.buffer_mutex);
			do
			{
				LL_GET(dev->process.buffer_first, dev->process.buffer_last, buffer);
				if (!buffer)
				{
					break;
				}
				free(buffer->data);
				free(buffer);
			}
			while (1);
			mutex_unlock(&dev->process.buffer_mutex);
			/* thread is running now */
			dev->process.paused = 0;
		}
		/* allocate new buffer */
		struct devlib_process_buffer *buffer = malloc(sizeof(*buffer));
		memset(buffer, 0, sizeof(*buffer));
		buffer->count = process->samples_per_buffer;
		buffer->bytes_per_sample = bytes_per_sample;
		buffer->data = malloc(buffer->count * buffer->bytes_per_sample);
		/* read data into buffer */
		err = devlib_manipulate(dev, NULL, buffer->data, buffer->count);
		if (err)
		{
			/** @todo handle errors */
			ERROR_MSG("device read failed");
			process->pause = 1;
			process->error = 1;
			continue;
		}
		/* lock buffers for this thread */
		mutex_lock(&dev->process.buffer_mutex);
		/* append data into process stream */
		LL_APP(dev->process.buffer_first, dev->process.buffer_last, buffer);
		dev->process.buffers_count++;
		/* if we have trigger */
		if (process->triggered)
		{
			/* if no maximum set, send buffers continuously until told otherwise */
			if (dev->process.buffers_total == 0)
			{
				/** @todo way to inform about new data when callback not set */
				if (process->callback_data)
				{
					/* unlock buffer mutex now */
					mutex_unlock(&dev->process.buffer_mutex);
					/* so this call can fetch data from buffer with locking
					 * (or do it later asynchronously)
					 */
					if (process->callback_data(process, process->userdata))
					{
						/* value returned was not zero, we are done here */
						process->pause = 1;
						process->done = 1;
						if (process->callback_done)
						{
							process->callback_done(process, process->userdata);
						}
					}
				}
			}
			/* check after trigger buffers count when maximum set*/
			else if (dev->process.buffers_count >= dev->process.buffers_total)
			{
				/* enough data read, stop */
				process->pause = 1;
				/* if there are no filters then ready now */
				if (1)
				{
					/* keep buffer mutex locked, needed and released here */
					i_devlib_process_finish(process, dev);
				}
			}
		}
		/* pre-trigger: check that there aint too much data */
		else if (dev->process.buffers_count > dev->process.buffers_pre_trigger)
		{
			/* pull oldest entry and free it */
			LL_GET(dev->process.buffer_first, dev->process.buffer_last, buffer);
			/* unlock buffer mutex now */
			mutex_unlock(&dev->process.buffer_mutex);
			free(buffer->data);
			free(buffer);
			dev->process.buffers_count--;
		}
		else
		{
			/* simply unlock */
			mutex_unlock(&dev->process.buffer_mutex);
		}
	}

	return 0;
}

int i_devlib_process_filter_thread(struct device *dev)
{
	dev++; /* placeholder */
	return 0;
}

int i_devlib_process_trigger_thread(struct device *dev)
{
	dev++; /* placeholder */
	return 0;
}

int i_devlib_process_writer_thread(struct device *dev)
{
	dev++; /* placeholder */
	return 0;
}

/* dev->process.buffer_mutex mutex must be locked before this call
 * and it will be unlocked in this call
 */
int i_devlib_process_finish(struct devlib_process *process, struct device *dev)
{
	mutex_lock(&process->buffer_done_mutex);
	/* if old data has not been read, free it now */
	if (process->buffer_done)
	{
		INFO_MSG("previous process buffer not read, have to release it before setting new one");
		devlib_process_buffer_free(process->buffer_done);
	}
	process->buffer_done = dev->process.buffer_first;
	dev->process.buffer_first = NULL;
	dev->process.buffer_last = NULL;
	dev->process.buffers_count = 0;
	mutex_unlock(&process->buffer_done_mutex);
	/* this must have been locked earlier! (not in this function) */
	mutex_unlock(&dev->process.buffer_mutex);
	process->done = 1;
	if (process->callback_done)
	{
		process->callback_done(process, process->userdata);
	}
	return 0;
}

