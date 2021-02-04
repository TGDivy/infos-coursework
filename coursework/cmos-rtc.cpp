/*
 * CMOS Real-time Clock
 * SKELETON IMPLEMENTATION -- TO BE FILLED IN FOR TASK (1)
 */

/*
 * STUDENT NUMBER: s1885517
 */
#include <infos/drivers/timer/rtc.h>
#include <infos/kernel/log.h>
#include <infos/kernel/sched.h>
#include <infos/kernel/thread.h>
#include <infos/util/list.h>
#include <infos/util/lock.h>
#include <arch/x86/pio.h>

// #include <infos/mm/page-allocator.h>
// #include <infos/mm/mm.h>
// #include <infos/kernel/kernel.h>
// #include <infos/util/math.h>
#include <infos/util/printf.h>

using namespace infos::arch::x86;

// using namespace infos::kernel;
// using namespace infos::mm;
using namespace infos::util;
using namespace infos::drivers;
using namespace infos::drivers::timer;

class CMOSRTC : public RTC {
public:
	static const DeviceClass CMOSRTCDeviceClass;

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

		__outb(0x70, 3); // Activate offset 3
		uint8_t v = __inb(0x71); // Read data	
		
		// syslog.messagef(LogLevel::DEBUG, "WOOOO HOOOOOOOOOOO %d",v);
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
