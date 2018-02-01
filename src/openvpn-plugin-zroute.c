#include "openvpn-plugin-zroute.h"

static const char* logprefix = "zroute";

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

OPENVPN_EXPORT int openvpn_plugin_open_v3 (
	__attribute__((unused)) const int version,
	struct openvpn_plugin_args_open_in const* args,
	struct openvpn_plugin_args_open_return* rv )
{
	int rc = OPENVPN_PLUGIN_FUNC_ERROR;

	struct plugin_context* plugin = calloc ( 1, sizeof ( *plugin ) );

	plugin->log = args->callbacks->plugin_log;

	rv->type_mask = OPENVPN_PLUGIN_MASK ( OPENVPN_PLUGIN_CLIENT_CONNECT ) | OPENVPN_PLUGIN_MASK ( OPENVPN_PLUGIN_CLIENT_DISCONNECT );

	rv->handle = (void*) plugin;

	rc = OPENVPN_PLUGIN_FUNC_SUCCESS;

	return rc;
}

OPENVPN_EXPORT void openvpn_plugin_close_v1 ( openvpn_plugin_handle_t handle )
{
	struct plugin_context* context = (struct plugin_context*) handle;

	free ( context );
}

OPENVPN_EXPORT int openvpn_plugin_func_v3 (
	__attribute__((unused)) const int version,
	struct openvpn_plugin_args_func_in const* args,
	__attribute__((unused)) struct openvpn_plugin_args_func_return* rv )
{
	int rc = OPENVPN_PLUGIN_FUNC_ERROR;

	struct plugin_context* plugin = (struct plugin_context*) args->handle;

	const char* rip_str = get_env ( "ifconfig_pool_remote_ip", args->envp );
	const char* ldv_str = get_env ( "dev", args->envp );
	const char* lip_str = get_env ( "ifconfig_local", args->envp );
	const char* lnm_str = get_env ( "ifconfig_netmask", args->envp );

	struct in_addr rip, lip, lnm;

	if ( rip_str == NULL )
		plugin->log ( PLOG_ERR, logprefix, "envp[ifconfig_pool_remote_ip] is not set" );
	else if ( ldv_str == NULL )
		plugin->log ( PLOG_ERR, logprefix, "envp[dev] is not set" );
	else if ( lip_str == NULL )
		plugin->log ( PLOG_ERR, logprefix, "envp[ifconfig_local] is not set" );
	else if ( lnm_str == NULL )
		plugin->log ( PLOG_ERR, logprefix, "envp[ifconfig_netmask] is not set" );
	else if ( inet_aton ( rip_str, &rip ) == 0 || inet_aton ( lip_str, &lip ) == 0 || inet_aton ( lnm_str, &lnm ) == 0 )
		plugin->log ( PLOG_ERR, logprefix, "inet_aton return 0" );
	else if ( ( rip.s_addr & lnm.s_addr ) != ( lip.s_addr & lnm.s_addr ) )
		plugin->log (
			PLOG_ERR,
			logprefix,
			"remote ip not in local net: rip_str: [%s] lip_str: [%s] lnm_str: [%s]",
			rip_str,
			lip_str,
			lnm_str );
	else if ( zroute_start () != 0 )
		plugin->log ( PLOG_ERR, logprefix, "zroute_start failed" );
	else
	{
		switch ( args->type )
		{
			default:
				plugin->log ( PLOG_ERR, logprefix, "openvpn_plugin_func_v3: unknown type" );
				break;
			case OPENVPN_PLUGIN_CLIENT_CONNECT:
				plugin->log ( PLOG_NOTE, logprefix, "zroute_route_to_host: add: [%s] [%s]", rip_str, ldv_str );
				if ( zroute_route_to_host ( ZEBRA_IPV4_ROUTE_ADD, &rip, ldv_str ) != 0 )
					plugin->log ( PLOG_ERR, logprefix, "zroute_route_to_host: add: [%s] [%s]", rip_str, ldv_str );
				else
					rc = OPENVPN_PLUGIN_FUNC_SUCCESS;
				break;
			case OPENVPN_PLUGIN_CLIENT_DISCONNECT:
				plugin->log ( PLOG_NOTE, logprefix, "zroute_route_to_host: del: [%s] [%s]", rip_str, ldv_str );
				if ( zroute_route_to_host ( ZEBRA_IPV4_ROUTE_DELETE, &rip, ldv_str ) != 0 )
					plugin->log ( PLOG_ERR, logprefix, "zroute_route_to_host: del: [%s] [%s]", rip_str, ldv_str );
				else
					rc = OPENVPN_PLUGIN_FUNC_SUCCESS;
				break;
		}

		zroute_stop ();
	}

	return rc;
}
