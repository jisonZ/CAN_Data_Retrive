// CAN_data_extract.cpp : main project file.

#include "stdafx.h"
#include <cstdlib>
#include ".\PCANBasicCLR.h"

namespace PCANBasicExample 
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Data;
	using namespace System::Threading;

	using namespace Peak::Can::Basic;

	int main(array<System::String ^> ^args)
	{
		CAN_API^ live_lab = gcnew CAN_API();
		live_lab->init_connect();
		live_lab->read();
		return 0;
	}


	ref class CAN_API {
	private:
		// USB Channel connected to harware
		// could have more channels due to more device connected
		TPCANHandle ^CAN_USB_Channel;
		TPCANBaudrate ^Baudrate;
		TPCANType ^Device;
		// CAN or CAN_FD, select CAN
		bool CAN_IS_FD = false;

		void ConfigureTraceFile(TPCANHandle ^CAN_Channel, UInt32 Buffer_size);
		TPCANStatus ^ ReadMessageFD();
		TPCANStatus ^ ReadMessage();
		void ReadMessage(TPCANMsg ^theMsg, TPCANTimestamp ^itsTimeStamp);

	public:
		CAN_API();
		void init_connect();
		void read();
	};

	void CAN_API::ConfigureTraceFile(TPCANHandle ^CAN_Channel, UInt32 Buffer_size)
	{
		TPCANStatus ^stsResult;
		stsResult = PCANBasic::SetValue(*CAN_Channel, TPCANParameter::PCAN_TRACE_SIZE, Buffer_size, sizeof(UInt32));
		if (*stsResult != TPCANStatus::PCAN_ERROR_OK) {
			Console::WriteLine(L"ConfigureTraceFile Error!");
			exit(1);
		}
		// Configure the way how trace files are created: 
		// * Standard name is used
		// * Existing file is ovewritten, 
		// * Only one file is created.
		// * Recording stopts when the file size reaches 5 megabytes.
		//
		UInt32 iBuffer = PCANBasic::TRACE_FILE_SINGLE | PCANBasic::TRACE_FILE_OVERWRITE;
		stsResult = PCANBasic::SetValue(*CAN_Channel, TPCANParameter::PCAN_TRACE_CONFIGURE, iBuffer, sizeof(UInt32));
		if (*stsResult != TPCANStatus::PCAN_ERROR_OK) {
			Console::WriteLine(L"ConfigureTraceFile Error!");
			exit(1);
		}
	}

	void CAN_API::init_connect()
	{
		//Initilize USB Channel
		CAN_USB_Channel = PCANBasic::PCAN_USBBUS1;
		/// 20 kBit/s
		Baudrate = TPCANBaudrate::PCAN_BAUD_20K;
		// Unkown Device type
		Device = TPCANType::PCAN_TYPE_ISA;
		// Unkonw I/O port
		UInt32 IOP = 1011;
		// Unkown Interrupt
		UInt16 Interrupt = 3;

		TPCANStatus ^stsResult;

		// Connects a selected PCAN-Basic channel
		//
		if (CAN_IS_FD) {
			// stsResult = PCANBasic::InitializeFD(CAN_USB_Channel, txtBitrate->Text);
		}
		else {
			// set up unkown I/O port and Interrupt
			stsResult = PCANBasic::Initialize(*CAN_USB_Channel, *Baudrate, *Device, IOP, Interrupt);
		}

		if (*stsResult != TPCANStatus::PCAN_ERROR_OK) {
			Console::WriteLine(L"Initialize Error!");
			exit(1);
		}
		else {
			// Prepares the PCAN-Basic's PCAN-Trace file
			// set tracefile size to 5MB
			ConfigureTraceFile(CAN_USB_Channel, 5);
		}
	}

	void CAN_API::read()
	{
		TPCANStatus ^stsResult;

		// We execute the "Read" function of the PCANBasic                
		//
		stsResult = CAN_IS_FD ? ReadMessageFD() : ReadMessage();
		if (*stsResult != TPCANStatus::PCAN_ERROR_OK) {
			Console::WriteLine(L"ReadMessage Error!");
			exit(1);
		}
	}

	//ghost-function
	TPCANStatus ^ CAN_API::ReadMessageFD()
	{
		return TPCANStatus::PCAN_ERROR_QRCVEMPTY;
	}

	/// <summary>
	/// Function for reading CAN messages on normal CAN devices
	/// </summary>
	/// <returns>A TPCANStatus error code</returns>
	TPCANStatus ^ CAN_API::ReadMessage()
	{
		TPCANMsg ^CANMsg;
		TPCANTimestamp ^CANTimeStamp;
		TPCANStatus ^stsResult;

		// We execute the "Read" function of the PCANBasic                
		//
		CANMsg = gcnew TPCANMsg();
		CANTimeStamp = gcnew TPCANTimestamp();
		stsResult = PCANBasic::Read(*CAN_USB_Channel, *CANMsg, *CANTimeStamp);
		if (*stsResult != TPCANStatus::PCAN_ERROR_QRCVEMPTY)
			// We process the received message
			// 
			ReadMessage(CANMsg, CANTimeStamp);

		return stsResult;
	}

	/// <summary>
	/// Processes a received message, in order to show it in the Message-ListView
	/// </summary>
	/// <param name="theMsg">The received PCAN-Basic message</param>
	/// <param name="itsTimeStamp">The Timestamp of the received message</param>
	void CAN_API::ReadMessage(TPCANMsg ^theMsg, TPCANTimestamp ^itsTimeStamp)
	{
		Console::WriteLine(L"Message ID is " + theMsg->ID);
		Console::WriteLine(L"Message DLC is " + theMsg->LEN);
		for (int i = 0; i < ((theMsg->LEN > 8) ? 8 : theMsg->LEN); i++) {
			Console::WriteLine(L"Message DATA[" + i + "] = " + theMsg->DATA[i]);
		}
		Console::WriteLine(L"Message type is " + theMsg->MSGTYPE);
		Console::WriteLine(L"Message time is " + Convert::ToUInt64(itsTimeStamp->micros + 1000 * itsTimeStamp->millis + 0x100000000 * 1000 * itsTimeStamp->millis_overflow));
	}

}


