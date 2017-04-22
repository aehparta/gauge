/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#ifndef __DEVLIB_PROCESS_H__
#define __DEVLIB_PROCESS_H__

#ifdef _WIN32
#define DLLEXP __declspec(dllexport)
#else
#define DLLEXP
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "devlib.h"
#include "os.h"


#ifdef __cplusplus
extern "C" {
#endif

struct devlib_process_buffer
{
	/** buffer length in samples */
	int count;
	/** bytes per sample */
	int bytes_per_sample;
	/** data itself */
	uint8_t *data;

	struct devlib_process_buffer *prev;
	struct devlib_process_buffer *next;
};

struct devlib_process
{
	/** keep all process threads running until execute is 0 */
	int execute;
	/** pause all process threads until pause is 0 */
	int pause;
	/** trigger occured */
	int triggered;
	/** set when enough data is read after trigger */
	int done;
	/** set to other than zero on errors */
	int error;
	/** source device for this process */
	struct device *source;
	/** filters/triggers/writers etc */
	struct device *dev_first;
	struct device *dev_last;

	/** samples to be read into single buffer with one manipulation call */
	int samples_per_buffer;
	/** data after trigger is stored here */
	struct devlib_process_buffer *buffer_done;
	/** mutex for data */
	mutex_t buffer_done_mutex;

	/** custom userdata to pass on to callbacks */
	void *userdata;
	/** callback to call when data has been received */
	int (*callback_data)(struct devlib_process *, void *);
	/** callback to call when receiving data is finished */
	int (*callback_done)(struct devlib_process *, void *);

	struct devlib_process *prev;
	struct devlib_process *next;
};


/**
 * Initialize processing.
 */
DLLEXP int devlib_process_init();
/**
 * De-initialize processing.
 */
DLLEXP void devlib_process_quit();
/**
 * Create new process from device of type DEV_TYPE_IO_READ.
 */
DLLEXP struct devlib_process *devlib_process_create(struct device *source);
/**
 * Return source device for this process.
 * @param  process devlib process pointer.
 * @return         Source device that was set when creating this process.
 */
DLLEXP struct device *devlib_process_source(struct devlib_process *process);
/**
 * Insert manipulation stage (filter/trigger) to process.
 */
DLLEXP int devlib_process_insert(struct devlib_process *process, int index, const char *serial);
/**
 * Remove manipulation stage (filter/trigger) from process.
 */
DLLEXP int devlib_process_remove(struct devlib_process *process, int index);
/**
 * Set samples per received buffer. This is the amount that will be read
 * at once from device. Notice that no data will be returned until this
 * amount is read from the device.
 *
 * @param  process devlib process pointer or NULL for all processes.
 * @param  count   Amount of samples to buffer or zero to query current value.
 * @return         Current value.
 */
DLLEXP int devlib_process_samples_per_buffer(struct devlib_process *process, int count);

/**
 * Start sampling data. To start all processes, give NULL as argument.
 */
DLLEXP int devlib_process_start(struct devlib_process *process);
/**
 * Check if process(es) is sampling. Give NULL as argument to check all,
 * in which case count of sampling processes is returned.
 *
 * @return 0 when not sampling
 */
DLLEXP int devlib_process_sampling(struct devlib_process *process);
/**
 * Pause process.
 */
DLLEXP int devlib_process_pause(struct devlib_process *process);
/**
 * Manually trigger process. Trigger all by giving NULL as argument.
 */
DLLEXP int devlib_process_trigger(struct devlib_process *process);
/**
 * Poll to check if process has read and processed all its data after trigger.
 */
DLLEXP int devlib_process_done(struct devlib_process *process);
/**
 * Get current processed data.
 */
DLLEXP struct devlib_process_buffer *devlib_process_buffer_get(struct devlib_process *process);
/**
 * Free buffer.
 */
DLLEXP void devlib_process_buffer_free(struct devlib_process_buffer *buffer);
/**
 * Set process userdata to pass on to callbacks.
 */
DLLEXP void devlib_process_set_userdata(struct devlib_process *process, void *data);
/**
 * Set callback to execute when new data can be read.
 */
DLLEXP void devlib_process_set_callback_data(struct devlib_process *process, int (*callback)(struct devlib_process *, void *));
/**
 * Set callback to execute when reading data is finished.
 */
DLLEXP void devlib_process_set_callback_done(struct devlib_process *process, int (*callback)(struct devlib_process *, void *));


#ifdef __cplusplus
}
#endif

#endif /* __DEV_H__ */
