#include <stdio.h>
#include <stdlib.h>
#include <openvpn/openvpn-plugin.h>

#include "zroute.h"

#include <quagga/table.h>

/** Only for testing the plugin! */

static const char* logprefix = "zroute: ";

int main ( int argc, char *argv[] )
{
	int rc = OPENVPN_PLUGIN_FUNC_ERROR;

	if ( argc < 5 )
	{
		printf ( "%serror: argc\n", logprefix );
		return 1;
	}

	int type;

	const char* typ_str = argv[1];
	const char* rip_str = argv[2];
	const char* ldv_str = argv[3];
	const char* lip_str = argv[4];
	const char* lnm_str = argv[5];

	if ( strcmp ( typ_str, "C" ) == 0 )
		type = OPENVPN_PLUGIN_CLIENT_CONNECT;
	else if ( strcmp ( typ_str, "D" ) == 0 )
		type = OPENVPN_PLUGIN_CLIENT_DISCONNECT;
	else
	{
		printf ( "%serror: typ_str\n", logprefix );
		return 1;
	}

//	struct route_table table;


	struct in_addr rip, lip, lnm;



	if ( rip_str == NULL )
		printf ( "%serror: envp[ifconfig_pool_remote_ip] is not set\n", logprefix );
	else if ( ldv_str == NULL )
		printf ( "%serror: envp[dev] is not set\n", logprefix );
	else if ( lip_str == NULL )
		printf ( "%serror: envp[ifconfig_local] is not set\n", logprefix );
	else if ( lnm_str == NULL )
		printf ( "%serror: envp[ifconfig_netmask] is not set\n", logprefix );
	else if ( inet_aton ( rip_str, &rip ) == 0 || inet_aton ( lip_str, &lip ) == 0 || inet_aton ( lnm_str, &lnm ) == 0 )
		printf ( "%serror: inet_aton return 0\n", logprefix );
	else if ( ( rip.s_addr & lnm.s_addr ) != ( lip.s_addr & lnm.s_addr ) )
		printf (
			"%serror: remote ip not in local net: rip_str: [%s] lip_str: [%s] lnm_str: [%s]\n",
			logprefix,
			rip_str,
			lip_str,
			lnm_str );
	else
	{
/*		struct route_table* table = route_table_init ();

		if ( table == 0 )
			printf ( "%serror: table == 0\n", logprefix );
		else
			printf ( "%serror: table != 0\n", logprefix );

		unsigned long count = route_table_count ( table );

		printf ( "%serror: count: %lu\n", logprefix, count );

		struct route_node* node = route_node_match_ipv4 ( table, &rip );

		route_table_finish ( table );

		if ( node != 0 )
			printf ( "%serror: exists??\n", logprefix );
		else*/ if ( zroute_start () != 0 )
			printf ( "%serror: zroute_start\n", logprefix );
		else
		{
			switch ( type )
			{
				default:
					printf ( "%serror: openvpn_plugin_func_v1: unknown type\n", logprefix );
					break;
				case OPENVPN_PLUGIN_CLIENT_CONNECT:
					printf ( "%szroute_add_route_to_host: [%s] [%s]\n", logprefix, rip_str, ldv_str );
					if ( zroute_route_to_host ( ZEBRA_IPV4_ROUTE_ADD, &rip, ldv_str ) != 0 )
						printf ( "%serror: zroute_route_to_host: add: [%s] [%s]\n", logprefix, rip_str, ldv_str );
					else
						rc = OPENVPN_PLUGIN_FUNC_SUCCESS;
					break;
				case OPENVPN_PLUGIN_CLIENT_DISCONNECT:
					printf ( "%szroute_del_route_to_host: [%s] [%s]\n", logprefix, rip_str, ldv_str );
					if ( zroute_route_to_host ( ZEBRA_IPV4_ROUTE_DELETE, &rip, ldv_str ) != 0 )
						printf ( "%serror: zroute_route_to_host: del: [%s] [%s]\n", logprefix, rip_str, ldv_str );
					else
						rc = OPENVPN_PLUGIN_FUNC_SUCCESS;
					break;
			}

			zroute_stop ();
		}
	}

	return rc;
}
