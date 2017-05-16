```
ADBUS0  -|1   20|-  GND
ADBUS1  -|2   19|-  VCC (3.3v)
ADBUS2  -|3   18|-  ACBUS9
ADBUS3  -|4   17|-  ACBUS8
ADBUS4  -|5   16|-  ACBUS7
ADBUS5  -|6   15|-  ACBUS6
ADBUS6  -|7   14|-  ACBUS5
ADBUS7  -|8   13|-  ACBUS4
ACBUS0  -|9   12|-  ACBUS3
ACBUS1  -|10  11|-  ACBUS2

#ISP-signals UM232H
  sck    = 0;
  mosi   = 1;
  miso   = 2;
  reset  = 3;



/* simple.c
   Simple libftdi usage example
   This program is distributed under the GPL, version 2
*/
#include <stdio.h>
#include <stdlib.h>
#include <ftdi.h>
int main(void)
{
	int ret;
	struct ftdi_context *ftdi;
	//struct ftdi_version_info version;
	if ((ftdi = ftdi_new()) == 0)
	{
		fprintf(stderr, "ftdi_new failed\n");
		return EXIT_FAILURE;
	}

	if ((ret = ftdi_usb_open(ftdi, 0x0403, 0x6014)) < 0)
	{
		fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
		ftdi_free(ftdi);
		return EXIT_FAILURE;
	}
	// Read out FTDIChip-ID of R type chips
	if (ftdi->type == TYPE_R)
	{
		unsigned int chipid;
		printf("ftdi_read_chipid: %d\n", ftdi_read_chipid(ftdi, &chipid));
		printf("FTDI chipid: %X\n", chipid);
	}

	printf("ftdi type: %d\n", ftdi->type);
	printf("bitbang: %d\n", ftdi->bitbang_enabled);

	while (1)
	{
		int b = ftdi_set_bitmode(ftdi, 0xFF, BITMODE_CBUS);
		printf("off %d\n", b);
		printf("bitbang: %d\n", ftdi->bitbang_enabled);
		sleep(1);
		int a = ftdi_set_bitmode(ftdi, 0xF0, BITMODE_CBUS);
		printf("on %d\n", a);
		sleep(1);
	}

	ftdi_free(ftdi);
	return EXIT_SUCCESS;
}
```
