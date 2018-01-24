#include "openvpn-plugin-zroute.h"

static const char* logprefix = "zroute: ";

static const char* get_env ( const char* name, const char* envp[] )
{
	if ( envp )
	{
		int i;
		const int namelen = strlen ( name );

		for ( i = 0; envp[i]; ++ i )
		{
			if ( ! strncmp ( envp[i], name, namelen ) )
			{
				const char* cp = envp[i] + namelen;

				if ( *cp == '=' )
					return cp + 1;
			}
		}
	}

	return NULL;
}

OPENVPN_EXPORT openvpn_plugin_handle_t openvpn_plugin_open_v1 (
	unsigned int* type_mask,
	__attribute__((unused)) const char* argv[],
	__attribute__((unused)) const char* envp[] )
{
	struct plugin_context* context;

	context = (struct plugin_context*) calloc ( 1, sizeof ( struct plugin_context ) );

	*type_mask = OPENVPN_PLUGIN_MASK ( OPENVPN_PLUGIN_CLIENT_CONNECT ) | OPENVPN_PLUGIN_MASK ( OPENVPN_PLUGIN_CLIENT_DISCONNECT );

	return (openvpn_plugin_handle_t) context;
}

OPENVPN_EXPORT void openvpn_plugin_close_v1 ( openvpn_plugin_handle_t handle )
{
	struct plugin_context* context = (struct plugin_context*) handle;

	free ( context );
}

OPENVPN_EXPORT int openvpn_plugin_func_v1 (
	__attribute__((unused)) openvpn_plugin_handle_t handle,
	const int type,
	__attribute__((unused)) const char* argv[],
	const char* envp[] )
{
	int rc = OPENVPN_PLUGIN_FUNC_ERROR;

	const char* rip_str = get_env ( "ifconfig_pool_remote_ip", envp );
	const char* ldv_str = get_env ( "dev", envp );
	const char* lip_str = get_env ( "ifconfig_local", envp );
	const char* lnm_str = get_env ( "ifconfig_netmask", envp );

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
	else if ( zroute_start () != 0 )
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

	return rc;
}
