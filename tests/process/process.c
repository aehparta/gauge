#include <stdio.h>
#include <stdlib.h>
#include <devlib_process.h>

int init()
{
	int err = 0;
	int c;
	int i;
	struct device **devices;

	devlib_init();
	devlib_add("test");
	devlib_add("ftd2xx");
	devlib_add("serial-port");
	devlib_add("trigger-edge");
	devlib_process_init();

	devlib_scan();
	devices = devlib_list(&c);
	IF_ERR(c < 1, -1, "no devices found");

	printf("input device list:\n");
	for (i = 0; i < c; i++)
	{
		if (!devlib_is_type(devices[i], DEV_TYPE_IO_READ))
		{
			continue;
		}
		printf(" - %s: %s\n", devlib_serial(devices[i]), devlib_name(devices[i]));
	}

	printf("trigger list:\n");
	for (i = 0; i < c; i++)
	{
		if (!devlib_is_type(devices[i], DEV_TYPE_TRIGGER))
		{
			continue;
		}
		printf(" - %s: %s\n", devlib_serial(devices[i]), devlib_name(devices[i]));
	}

out_err:
	return err;
}

int data_received_cb(struct devlib_process *process, void *userdata)
{
	static long double start = -1;
	static int count = 0;
	double t;
	struct device *dev = devlib_process_source(process);
	if (start < 0.0L)
	{
		double interval = (double) * (long double *)devlib_ioctl(dev, DEV_IOCTL_GET_INTERVAL);
		start = os_time() - (interval * devlib_process_samples_per_buffer(process, 0));
	}
	t = (double)(os_time() - start);

	struct devlib_process_buffer *buffer = devlib_process_buffer_get(process);
	struct devlib_process_buffer *tmpbuf;
	int sum = 0, n = 0;
	for (tmpbuf = buffer; tmpbuf; tmpbuf = tmpbuf->next)
	{
		int i;
		for (i = 0; i < tmpbuf->count; i++)
		{
			sum += tmpbuf->data[i] & 0x1;
			n++;
		}
	}
	devlib_process_buffer_free(buffer);

	printf("%lf %lf data received, inform the men! ", t / count, t);
	printf("samples: %d, average: %d, sum: %d\n", n, sum / n, sum);

	count++;
	if (count > 10)
	{
		start = -1;
		count = 0;
		return 1;
	}
	return 0;
}

int main(void)
{
	int err = 0;
	int i;
	struct device *dev, *trigger;
	struct devlib_process *process;

	IF_ERR(init(), -1, "init failed");

	char *serial = "ES1111-1";
	serial = "TLPIrCPW";
	dev = devlib_find(serial);
	IF_ERR(!dev, -1, "unable to find device %s", serial);
	IF_ERR(devlib_ioctl(dev, DEV_IOCTL_SET_INTERVAL, (long double)0.0001) != DEV_IOCTL_OK, -1, "unable to set interval");

	/* create and start test process */
	process = devlib_process_create(dev);
	IF_ERR(!process, -1, "failed creating process, source device %s", serial);
	devlib_process_set_callback_data(process, data_received_cb);
	/* add trigger */
	serial = "trigger-edge";
	devlib_process_insert(process, -1, serial);

	for (i = 0; i < 3; i++)
	{
		devlib_process_start(process);
		/* wait some time for samples */
		printf("waiting for samples\n");
		//os_sleep(5.345);
		/* manual trigger */
		printf("trigger manually\n");
		devlib_process_trigger(process);
		/* wait for process to be done */
		printf("wait process to finish sampling\n");
		while (!devlib_process_done(process))
		{
			os_sleep(0.1);
		}
		printf("done\n");
		os_sleep(1.0);
	}

	devlib_process_quit();
	devlib_quit();

out_err:
	return err;
}
