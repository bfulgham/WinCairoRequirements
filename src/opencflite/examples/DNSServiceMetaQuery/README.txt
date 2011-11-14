DNSServiceMetaQuery is based on the Apple, inc. example:
http://developer.apple.com/samplecode/DNSServiceMetaQuery/index.html

It has been modified to build using OpenCFLite.

The program shows how to discover every Bonjour service type
being advertised on the local network.  You must have the
Bonjour SDK installed (or an equivalent Zeroconf service, such
as Avahi).  You may need to modify project settings to point
to the proper location on your system for the necessary
service discovery API (dns_sd.h).

This sample uses the socket-based DNSServiceDiscovery API to
discover all service types being advertised on the network.
Starting in Mac OS X 10.3.4, computers on the network which
advertise Bonjour services will automatically register an
additional PTR record with the name "_services._dns-sd._udp.local."
which points to the service type and domain of the advertised
service.  For example:

_services._dns-sd._udp.local.  IN  PTR  _ftp._tcp.local.

This sample uses DNSServiceQueryRecord() to send a Multicast
DNS query for a PTR record named "_services._dns-sd._udp.local.".
The result of the query will be a PTR record which points to the
DNS-SD service type and domain.  This sample also shows how to
parse the resulting PTR record data so that you could potentially
pass the results to DNSServiceBrowse().

