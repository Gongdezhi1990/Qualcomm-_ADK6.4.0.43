#ifndef __HOST_H__
#define __HOST_H__

/*!
file 
\brief Host communication over the TestTunnel 'host' transport.
**
A user application can communicate with the outside world by
using the 'host' TestTunnel. This library provides a simple
interface for the user application
to:
- send and receive \ref host_messages
- send and receive \ref host_streams (not supported in ADK6)
**
An application which does not use a host transport can use the
USB or UART interface directly, using the stream.h library. 
The host library is not used in that case.
**
User-written software on the host based on TestEngine is expected
to form the other end of the connection.
**
See \ref host for more info.
**
\page host Host communication
\section host_messages 16-bit data in messages
\subsection host_message_fmt Format of a packet
**
The packet format that is used by the VM for communications is
very simple, but allows the user to implement a large number of
features on top of the simple interface that we provide. The
packet begins with one 16-bit word indicating the length of the
packet (including the header), followed by a sub-type word. For
messages the sub-type can have any value in the range of 0-127
(0x00-0x7f), and is meant to indicate the type of the packet to
the code at either end. There are no restrictions on the use of
this word, as long as it is within the correct range. The
remaining words in the packet can contain any 16-bit data.
**
Typically the sub-type field is used to multiplex an number of
distinct channels between the application and the host.
**
All packets are held in dynamic memory blocks and the
constraints on number and size of these described in the
standard library apply to packets.
**
<table>
<tr>
<td>Length (16bit)</td>
<td>Sub-type (16bit)</td>
<td>Data...</td>
</tr>
</table>
**
To receive packets from the host, the application must register a 
task for \#MESSAGE_FROM_HOST using MessageHostCommsTask().
**
\subsection host_message_code Example code
Send a small packet to the host
\code
uint16 *data = PanicUnlessMalloc(3 * sizeof(uint16));
data[0] = 3; // length
data[1] = 0x7e; // sub-type
data[2] = 0x1234; // data
HostSendMessage(data);
data = NULL; // we don't own this block anymore
\endcode
**
\section host_streams 8-bit data over Streams
There are 256 sources and 256 sinks associated with
stream-based host communications; an application might use
one for control and one for data.
**
An application can obtain the source and sink for a particular
one of the 256 channels using the StreamHostSource() and
StreamHostSink() functions. The source and sink are then
manipulated using the other functions from the stream.h 
library.
**
\subsection host_stream_fmt Format of a packet
The format of a packet is almost identical to that used for
messages; this preserves compatibility with existing firmware,
at the cost of making the stream-based protocol more complex
than strictly necessary.
**
The packet again begins with one 16-bit word indicating the
length of the packet in words (including the header), followed
by a sub-type word. The payload follows, with the bytes packed
into 16-bit words in low-byte high-byte order and padded out to
a 16-bit boundary. That is a payload of 0x01, 0x02, 0x03 will
be packed as 0x0201, 0x0003. For streams the sub-type can have
any value in the range of 0x100-0x2ff, and encodes both the 
stream being used and whether the payload includes a padding 
byte.
**
The bottom 8-bits of the sub-type word encode the channel for
the source or sink. If the top 8-bits are 2 then all of the 
bytes of the payload are used, if 1 then the top byte of the 
last word is not part of the payload.
**
In other words, a consumer might look like:
\code
void incoming(const uint16 *packet)
{
uint16 words = packet[0];
uint16 type  = packet[1];
uint16 chan  = type & 0xff;
uint16 bytes = (words - 2) * 2 - ((type >> 8) == 1);
...
}
\endcode
**
\subsection host_stream_code Example code
Send a 2 byte packet to the host on channel 1
\code
Sink sink = PanicNull(StreamHostSink(1));
uint8 *data = SinkMap(sink) + SinkClaim(sink, 2);
data[0] = 0x12;
data[1] = 0x34;
PanicZero(SinkFlush(sink, 2));
\endcode
**
Process packets from the host on channel 2
\code
uint16 size;
Source source = StreamHostSource(2);
while ((size = SourceBoundary(source)) != 0)
{
const uint8 *data = SourceMap(source);
// do something with the data here
SourceDrop(source, size);
}
\endcode
**
\section host_issues Related Issues
\subsection host_limitations Limitations
All packets must fit in an available on-chip memory block
and as such are restricted to around 80 words in total.
This restriction applies even to stream-based messages.
**
If a packet arrives for a source and insufficient space is
available, the message will be silently discarded. Users
can expect to have at most 200 bytes of data held in any
source associated with host communications.
**
\subsection host_coex Coexistence
Both message and stream based host communications can be
used in the same application as required; the second word
of a data message is used to distinguish between the two
formats.
**
Which is more appropriate depends on your application:
streams are designed for 8-bit data, while messages are
more suited to packed binary data such as structures.
*/

#if TRAPSET_HOST

/**
 *  \brief Send a message to the host over the TestTunnel 'host' channel.
 *  \param msg The message (block) to send. If the block has been malloc'd then it will be
 *  free'd automatically as part of this call. Non-malloc'd blocks (for example,
 *  constant arrays) will be copied into a block malloc'd by the call, and that
 *  block will be used instead. 
 *  \return The length of the block sent on success, zero otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_host
 */
uint16 HostSendMessage(uint16 * msg);
#endif /* TRAPSET_HOST */
#endif
