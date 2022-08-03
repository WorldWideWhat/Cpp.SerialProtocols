#define CCTALK

#include <iostream>
//#include <unistd.h>

#include <string>
#include "lib/stm/stmboot.h"

#ifdef CCTALK
#include "lib/cctalk/cctalk.h"

CCTalk cct(1);
CCTalk::EventStack eventStack;

int n_response_cnt = 0;

void runComm(){
	bool n_connected = false;
	std::printf("Connect CCT\n");
	while (true)
	{
		int n_diff = cct.getEventStack(2, eventStack);

		if(n_diff < 0) {
			n_response_cnt = 0;
			std::printf("ERROR!!!!!\r");
			std::fflush(stdout);
			cct.disconnect();
			break;
		} else if (!n_connected)
		{
			std::printf("\nConnected\nLast event id %d\n", eventStack.lastEventId);
			n_connected = true;
		}
		n_response_cnt++;

		if(n_diff > 0) {
#ifdef _WIN32
			std::printf("Stack size %d\n", eventStack.events.size());
#else			
			std::printf("Stack size %ld\n", eventStack.events.size());
#endif			
			std::printf("Changed %d, new eventID %d\n", n_diff, eventStack.lastEventId);
			for(int n_index = 0; n_index < n_diff; n_index++) {
				int m_index = eventStack.events.size() - (n_index+1);
				std::printf("Event - type = %d, value = %d\n", 
					eventStack.events[m_index].event_Type, 
					eventStack.events[m_index].event_Value);
			}
		}
#ifdef _WIN32
		Sleep(100);
#else		
		usleep(100000);
#endif		
	}
}


#endif


void progress(uint32_t total, uint32_t written){
	std::printf("Total %d, Written %d\r", total, written);
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
	std::string hostPort("COM39");
	std::string cctPort("COM38");
#else
	std::string hostPort("/dev/ttyUSB0");
	std::string cctPort("/dev/ttyUSB1");
#endif

	std::printf("Welcome\n");
	std::printf("argc: %d\n", argc);

	Host n_host;
	if(n_host.connect(hostPort.c_str(), B115200) != 0){
		std::printf("Unable to connect to port\n");
		return 0;
	}
	
	if(n_host.reboot() != 0){
		std::printf("Turn on device\n");
	}
	n_host.disconnect();

	STMBoot n_boot;
	STMBoot::Header n_header;
	//std::string str();
	n_boot.setProgressCallback(progress);

	if(n_boot.connect(hostPort.c_str(), B115200) == 0) {
		n_boot.setBinaryFile("controller_app.bin");

		int n_res = n_boot.getHeader(n_header);
		if(n_res != 0){
			std::printf("Error while loading header %d", n_res);
			return 0;
		}

		std::printf("Signature: %08X\nVersion: %d.%d.%d\n", (uint32_t)n_header.signature,
														n_header.major,
														n_header.minor,
														n_header.build);

		if(n_boot.init(STMBoot::Target::ControllerFW) != 0) {
			std::printf("Unable to init device\n");
		} else {
			std::printf("Device connected\n");
			n_res = n_boot.programTarget(true);
			if(n_res != 0) {
				std::printf("Error while programming target\n");
			} else {
				std::printf("Target programmed OK\n");
				n_boot.disconnect();
			}
		}
	} else std::printf("Unable to open USB0\n");


#ifdef CCTALK	
	
	while(true) {
		
		if(cct.connect(cctPort.c_str(), B9600) != 0){
			std::printf("Unable to connect to port\n");
			std::fflush(stdout);
#ifdef _WIN32
			Sleep(50);
#else			
			usleep(50000);
#endif			
		} else {
			runComm();
		}
	}
#endif	
}

