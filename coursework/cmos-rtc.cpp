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

	// Address locations
	enum {
		cmos_address = 0x70,
		cmos_data    = 0x71
	};

	// If cmos is updating, as shown in register A
	int get_update_in_progress_flag() {
		__outb(cmos_address, 0x0A);
		return (__inb(cmos_data) & 0x80);
	}
	
	/* Note: there should be a delay between outb, and inb calls to ensure 
	the value has updated correctly, however, being a simulation, instructor said
	we can skip that part.  
	*/
	unsigned char get_RTC_register(int reg) {
		__outb(cmos_address, reg);
		// add some small delay.
		return __inb(cmos_data);
	}

	// Structure saved loaded time from cmos.
	struct time {
		unsigned char second;
		unsigned char minute;
		unsigned char hour;
		unsigned char day;
		unsigned char month;
		unsigned int year;

		// Comparing if two instances of time structure are same.
		bool operator==(const time &t1) {
			return 	second == t1.second && 
					minute == t1.minute &&
					hour == t1.hour &&
					day == t1.day &&
					month == t1.month &&
					year == t1.year;
		}
		// If our time is in BCD, this function makes it binary.
		void BCD_to_binary(){
			second = (second & 0x0F) + ((second / 16) * 10);
			minute = (minute & 0x0F) + ((minute / 16) * 10);
			hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
			day = (day & 0x0F) + ((day / 16) * 10);
			month = (month & 0x0F) + ((month / 16) * 10);
			year = (year & 0x0F) + ((year / 16) * 10);
		}
		// If our time is 12 hour clock, calling this function makes it 24 hour.
		void from_12_to_24_hourclock(){
			hour = ((hour & 0x7F) + 12) % 24;
		}

	} current_time; 

	// A function to update the time by reaeding the cmos registers.
	void read_registers(time &t){
		t.second = get_RTC_register(0x00);
		t.minute = get_RTC_register(0x02);
		t.hour = get_RTC_register(0x04);
		t.day = get_RTC_register(0x07);
		t.month = get_RTC_register(0x08);
		t.year = get_RTC_register(0x09);
	}

	/* Note: Reading the time by waiting until an entire update cycle is very slow and will always 
	take upto 1 second. We can instead read the values instantly, and check them again to confirm 
	if what we read is correct, repeat this until we get a correct reading. This also helps
	avoid reading incorrect values.
	*/
	void read_clock() {
		time last_time; // to have a previous timestamp comparison.
		unsigned char registerB;
	
		// Wait until there is no update and then read the time.
		while (get_update_in_progress_flag()); 
		read_registers(current_time);

		do {
			last_time = current_time; // Save our previously read time, as a copy.

			// Again, wait until no update is happening, and read the time.
			while (get_update_in_progress_flag());
			read_registers(current_time);
		} // If the two readings are the same, the readings are correct, and we can exit the do-while
		while(!(last_time==current_time));
	
		// Interpreting our readings correctly, such that there are no formatting errors.

		// Register B decides BCD or binary format.
		registerB = get_RTC_register(0x0B);
		// Convert BCD to binary values if necessary
		if (!(registerB & 0x04)) {
			current_time.BCD_to_binary();
		}

		// Converting from 12 hour clock to 24 hour clock when necessary.
		if (!(registerB & 0x02) && (current_time.hour & 0x80)) {
			current_time.from_12_to_24_hourclock();
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
		UniqueIRQLock(); // Disabling interupts.
		read_clock(); // Populating current time.
		tp.seconds = current_time.second;
		tp.minutes = current_time.minute; 
		tp.hours = current_time.hour; 
		tp.day_of_month = current_time.day; 
		tp.month = current_time.month;
		tp.year = current_time.year;
	}
};

const DeviceClass CMOSRTC::CMOSRTCDeviceClass(RTC::RTCDeviceClass, "cmos-rtc");

RegisterDevice(CMOSRTC);
