#include "zroute.h"

static struct zclient* client = 0;
static struct thread_master* master = 0;

/*!
*	return returns 0 on success
*/
int zroute_start ()
{
	int rc = 1;

	if ( ( master = thread_master_create () ) != 0 )
	{
		if ( ( client = zclient_new ( master ) ) != 0 )
		{
			zclient_init ( client, ZEBRA_ROUTE_STATIC );

			client->t_connect = NULL;

			if ( zclient_start ( client ) == 0 && client->sock != -1 )
				rc = 0;
		}
	}

	return rc;
}

void zroute_stop ()
{
	zclient_stop ( client );
}

/*!
*	return returns 0 on success
*/
int zroute_route_to_host ( u_char cmd, struct in_addr* ip_in, const char* dv_str )
{
	int rc = 1;
	struct zapi_ipv4 za;
	struct prefix_ipv4 ip;

	memset ( &za, 0, sizeof ( za ) );
	memset  ( &ip, 0, sizeof ( ip ) );

	ip.family = AF_INET;
	ip.prefixlen = 32;
	ip.prefix = *ip_in;

	za.vrf_id = VRF_DEFAULT;
	za.type = ZEBRA_ROUTE_STATIC;
	za.flags = ZEBRA_FLAG_STATIC;
	za.message = 0;
	za.safi = SAFI_UNICAST;
	SET_FLAG (za.message, ZAPI_MESSAGE_NEXTHOP);
	za.nexthop_num = 0;
	za.nexthop = 0;
	SET_FLAG (za.message, ZAPI_MESSAGE_IFINDEX);
	za.ifindex_num = 1;

	ifindex_t ifh = if_nametoindex ( dv_str );

	if ( ifh != 0 )
	{
		za.ifindex = &ifh;
		za.tag = 0;
		SET_FLAG (za.message, ZAPI_MESSAGE_METRIC);
		za.metric = 20;

		rc = zapi_ipv4_route ( cmd, client, &ip,  &za );
	}

	return rc;
}
