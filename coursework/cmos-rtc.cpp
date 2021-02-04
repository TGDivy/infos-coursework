/*
 * CMOS Real-time Clock
 * SKELETON IMPLEMENTATION -- TO BE FILLED IN FOR TASK (1)
 */

/*
 * STUDENT NUMBER: s1885517
 */
#include <infos/drivers/timer/rtc.h>
#include <infos/kernel/log.h>
// #include <infos/kernel/sched.h>
// #include <infos/kernel/thread.h>
// #include <infos/util/list.h>
#include <infos/util/lock.h>
#include <arch/x86/pio.h>

// #include <infos/mm/page-allocator.h>
// #include <infos/mm/mm.h>
// #include <infos/kernel/kernel.h>
// #include <infos/util/math.h>
#include <infos/util/printf.h>

using namespace infos::arch::x86;

using namespace infos::kernel;
// using namespace infos::mm;
using namespace infos::util;
using namespace infos::drivers;
using namespace infos::drivers::timer;

class CMOSRTC : public RTC {
public:
	static const DeviceClass CMOSRTCDeviceClass;

	unsigned char second;
	unsigned char minute;
	unsigned char hour;
	unsigned char day;
	unsigned char month;
	unsigned int year;

	enum {
		cmos_address = 0x70,
		cmos_data    = 0x71
	};

	
	int get_update_in_progress_flag() {
		__outb(cmos_address, 0x0A);
		return (__inb(cmos_data) & 0x80);
	}
	
	unsigned char get_RTC_register(int reg) {
		__outb(cmos_address, reg);
		return __inb(cmos_data);
	}

	void read_rtc(RTCTimePoint& tp) {
		unsigned char registerB;

		registerB = get_RTC_register(0x0B);

		second = get_RTC_register(0x00);
		minute = get_RTC_register(0x02);
		hour = get_RTC_register(0x04);
		day = get_RTC_register(0x07);
		month = get_RTC_register(0x08);
		year = get_RTC_register(0x09);
			
		if (!(registerB & 0x04)) {
            second = (second & 0x0F) + ((second / 16) * 10);
            minute = (minute & 0x0F) + ((minute / 16) * 10);
            hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
            day = (day & 0x0F) + ((day / 16) * 10);
            month = (month & 0x0F) + ((month / 16) * 10);
            year = (year & 0x0F) + ((year / 16) * 10);
    	}

	    if (!(registerB & 0x02) && (hour & 0x80)) {
            hour = ((hour & 0x7F) + 12) % 24;
      	}
 
		tp.seconds = second;
		tp.minutes = minute; 
		tp.hours=hour; 
		tp.day_of_month=day; 
		tp.month=month;
		tp.year=year;

		while (get_update_in_progress_flag()); 	
	}

	const DeviceClass& device_class() const override
	{
		return CMOSRTCDeviceClass;
	}

	/**
	 * Interrogates the RTC to read the current date & time.
	 * @param tp Populates the tp structure with the current data & time, as
	 * given by the CMOS RTC device.
	 */
	void read_timepoint(RTCTimePoint& tp) override
	{
		UniqueIRQLock();

		__outb(0x70, 0x00); // Activate offset 3
		uint8_t v = __inb(0x71); // Read data	
		
		syslog.messagef(LogLevel::DEBUG, "WOOOO HOOOOOOOOOOO %d",v);
		tp.seconds = 1;
		tp.minutes = 1; 
		tp.hours=1; 
		tp.day_of_month=1; 
		tp.month=1;
		tp.year=1;
		// FILL IN THIS METHOD - WRITE HELPER METHODS IF NECESSARY
	}
};

const DeviceClass CMOSRTC::CMOSRTCDeviceClass(RTC::RTCDeviceClass, "cmos-rtc");

RegisterDevice(CMOSRTC);
