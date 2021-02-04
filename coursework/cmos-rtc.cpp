/*
 * CMOS Real-time Clock
 * SKELETON IMPLEMENTATION -- TO BE FILLED IN FOR TASK (1)
 */

/*
 * STUDENT NUMBER: s1885517
 */
#include <infos/drivers/timer/rtc.h>
#include <infos/kernel/log.h>

using namespace infos::kernel;
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
		syslog.messagef(LogLevel::DEBUG, "WOOOO HOOOOOOOOOOO");
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
