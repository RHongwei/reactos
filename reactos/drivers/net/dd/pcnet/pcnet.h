/*
 * ReactOS AMD PCNet Driver
 *
 * Copyright (C) 2003 Vizzini <vizzini@plasmic.com>
 * Copyright (C) 2004 Filip Navara <navaraf@reactos.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * REVISIONS:
 *     01-Sep-2003 vizzini - Created
 * NOTES:
 *     - this assumes a 32-bit machine, where sizeof(PVOID) = 32 and sizeof(USHORT) = 16
 *     - this assumes 32-bit physical addresses
 */

#ifndef _PCNET_H_
#define _PCNET_H_

/* adapter struct */
typedef struct _ADAPTER 
{
  NDIS_HANDLE MiniportAdapterHandle;
  ULONG Flags;
  ULONG InterruptVector;
  ULONG IoBaseAddress;
  PVOID PortOffset;
  NDIS_MINIPORT_INTERRUPT InterruptObject;
  ULONG CurrentReceiveDescriptorIndex;
  ULONG CurrentPacketFilter;
  ULONG CurrentLookaheadSize;

  /* circular indexes to transmit descriptors */
  ULONG CurrentTransmitStartIndex;
  ULONG CurrentTransmitEndIndex;

  /* initialization block */
  ULONG InitializationBlockLength;
  PINITIALIZATION_BLOCK InitializationBlockVirt;
  PINITIALIZATION_BLOCK InitializationBlockPhys;

  /* transmit descriptor ring */
  ULONG TransmitDescriptorRingLength;
  PTRANSMIT_DESCRIPTOR TransmitDescriptorRingVirt;
  PTRANSMIT_DESCRIPTOR TransmitDescriptorRingPhys;

  /* transmit buffers */
  ULONG TransmitBufferLength;
  PCHAR TransmitBufferPtrVirt;
  PCHAR TransmitBufferPtrPhys;

  /* receive descriptor ring */
  ULONG ReceiveDescriptorRingLength;
  PRECEIVE_DESCRIPTOR ReceiveDescriptorRingVirt;
  PRECEIVE_DESCRIPTOR ReceiveDescriptorRingPhys;

  /* receive buffers */
  ULONG ReceiveBufferLength;
  PCHAR ReceiveBufferPtrVirt;
  PCHAR ReceiveBufferPtrPhys;
} ADAPTER, *PADAPTER;

/* forward declarations */
NDIS_STATUS
STDCALL
MiniportQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded);

NDIS_STATUS
STDCALL
MiniportSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded);

/* operational constants */
#define NUMBER_OF_BUFFERS     0x20
#define LOG_NUMBER_OF_BUFFERS 5         /* log2(NUMBER_OF_BUFFERS) */
#define BUFFER_SIZE           0x600

/* flags */
#define RESET_IN_PROGRESS 0x1

/* debugging */
#if DBG
#define PCNET_DbgPrint(_x) \
{\
  DbgPrint("%s:%d %s: ", __FILE__, __LINE__, __FUNCTION__); \
  DbgPrint _x; \
}
#else
#define PCNET_DbgPrint(_x)
#endif

#if DBG
#define BREAKPOINT __asm__ ("int $3\n");
#else
#define BREAKPOINT
#endif

/* memory pool tag */
#define PCNET_TAG 0xbaadf00d

#endif // _PCNET_H_

