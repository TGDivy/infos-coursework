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

	
	enum {
		cmos_address = 0x70,
		cmos_data    = 0x71
	};

	struct time {
		unsigned char second;
		unsigned char minute;
		unsigned char hour;
		unsigned char day;
		unsigned char month;
		unsigned int year;

		bool operator==(const time& t1) {
			return 	second == t1.second && 
					minute == t1.minute &&
					hour == t1.hour &&
					day == t1.day &&
					month == t1.month &&
					year == t1.year;
		}

	} current_time; 
	
	int get_update_in_progress_flag() {
		__outb(cmos_address, 0x0A);
		return (__inb(cmos_data) & 0x80);
	}
	
	unsigned char get_RTC_register(int reg) {
		__outb(cmos_address, reg);
		return __inb(cmos_data);
	}

	void read_registers(time t){
		t.second = get_RTC_register(0x00);
		t.minute = get_RTC_register(0x02);
		t.hour = get_RTC_register(0x04);
		t.day = get_RTC_register(0x07);
		t.month = get_RTC_register(0x08);
		t.year = get_RTC_register(0x09);
	}

	void BCD_to_binary(time t){
		t.second = (t.second & 0x0F) + ((t.second / 16) * 10);
		t.minute = (t.minute & 0x0F) + ((t.minute / 16) * 10);
		t.hour = ( (t.hour & 0x0F) + (((t.hour & 0x70) / 16) * 10) ) | (t.hour & 0x80);
		t.day = (t.day & 0x0F) + ((t.day / 16) * 10);
		t.month = (t.month & 0x0F) + ((t.month / 16) * 10);
		t.year = (t.year & 0x0F) + ((t.year / 16) * 10);
	}

	void read_rtc() {
		time last_time;
		unsigned char registerB;
	
		// Note: This uses the "read registers until you get the same values twice in a row" technique
		//       to avoid getting dodgy/inconsistent values due to RTC updates
	
		while (get_update_in_progress_flag());                // Make sure an update isn't in progress
		read_registers(current_time);

		do {
			last_time = current_time;

			while (get_update_in_progress_flag());           // Make sure an update isn't in progress
			read_registers(current_time);

		} while(!(last_time==current_time));
	
		registerB = get_RTC_register(0x0B);
	
		// Convert BCD to binary values if necessary
		if (!(registerB & 0x04)) {
			BCD_to_binary(current_time);
		}
	
		// Convert 12 hour clock to 24 hour clock if necessary
	
		if (!(registerB & 0x02) && (current_time.hour & 0x80)) {
			current_time.hour = ((current_time.hour & 0x7F) + 12) % 24;
		}	 	
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
		read_rtc();
		tp.seconds = current_time.second;
		tp.minutes = current_time.minute; 
		tp.hours=current_time.hour; 
		tp.day_of_month=current_time.day; 
		tp.month=current_time.month;
		tp.year=current_time.year;
	}
};

const DeviceClass CMOSRTC::CMOSRTCDeviceClass(RTC::RTCDeviceClass, "cmos-rtc");

RegisterDevice(CMOSRTC);
