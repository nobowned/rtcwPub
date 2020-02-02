#include "server.h"

netadr_t adr;
streamed_socket *ss;

char http_request[MAX_MSGLEN];

qboolean SV_TcpServerConnect(char *hostname, int port)
{
	if (!NET_StringToAdr(hostname, &adr))
	{
		return qfalse;
	}

	adr.port = BigShort(port);

	return NET_OpenStreamedSocket(&ss, &adr);
}

qboolean SV_GlobalStatsConnect(void)
{
	return SV_TcpServerConnect(sv_globalStatsServerName->string, sv_globalStatsPort->integer);
}

void SV_GlobalStatsReconnect(void)
{
	SV_GlobalStatsDisconnect();
	SV_GlobalStatsConnect();
}

void SV_GlobalStatsDisconnect(void)
{
	NET_CloseStreamedSocket(ss);
}

// Send every client's statistics to the stats server (http request over tcp)
void SV_SendGlobalStats(char *stats_data)
{
	if (!sv_globalStats->integer)
	{
		return;
	}
	if (adr.type == NA_BAD)
	{
		return;
	}
	// Set the header
	Q_strncpyz(http_request,
		va("POST %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"Accept: %s\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: %i\r\n\r\n",
			sv_globalStatsServerPath->string, sv_globalStatsServerName->string,
			sv_globalStatsToken->string, strlen(stats_data)),
		sizeof(http_request));
	// Append the body
	Q_strcat(http_request, sizeof(http_request), stats_data);
	// Send the tcp packet containing http_request
	Sys_SendStreamedPacket(ss, http_request, strlen(http_request));
}

void SV_SendExternalIpRequest(void)
{
	if (!SV_TcpServerConnect(sv_getExternalIpDomain->string, 80))
	{
		return;
	}

	Cvar_Set("sv_getExternalIpServerIp", NET_AdrToString(adr));

	snprintf(http_request, sizeof(http_request), "GET / HTTP/1.1\r\nHOST: %s\r\n\r\n", sv_getExternalIpDomain->string);
	Sys_SendStreamedPacket(ss, http_request, strlen(http_request));
}
